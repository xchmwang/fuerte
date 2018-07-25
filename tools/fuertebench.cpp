////////////////////////////////////////////////////////////////////////////////
/// DISCLAIMER
///
/// Copyright 2018 ArangoDB GmbH, Cologne, Germany
///
/// Licensed under the Apache License, Version 2.0 (the "License");
/// you may not use this file except in compliance with the License.
/// You may obtain a copy of the License at
///
///     http://www.apache.org/licenses/LICENSE-2.0
///
/// Unless required by applicable law or agreed to in writing, software
/// distributed under the License is distributed on an "AS IS" BASIS,
/// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
/// See the License for the specific language governing permissions and
/// limitations under the License.
///
/// Copyright holder is ArangoDB GmbH, Cologne, Germany
///
/// @author Simon Grätzer
////////////////////////////////////////////////////////////////////////////////

#define ENABLE_FUERTE_LOG_ERROR 1
#define ENABLE_FUERTE_LOG_DEBUG 1
#define ENABLE_FUERTE_LOG_TRACE 1

#include <chrono>
#include <unordered_set>

#include <fuerte/connection.h>
#include <iostream>
#include <fuerte/message.h>
#include <fuerte/loop.h>
#include <fuerte/requests.h>
#include <fuerte/helper.h>

#include <velocypack/velocypack-aliases.h>

#include "bench/BenchmarkCounter.h"
#include "bench/BenchmarkOperation.h"
#include "bench/BenchmarkThread.h"
#include "bench/test-cases.h"
#include "bench/TestConfig.h"

using ConnectionBuilder = arangodb::fuerte::ConnectionBuilder;
using EventLoopService = arangodb::fuerte::EventLoopService;
using Request = arangodb::fuerte::Request;
using MessageHeader = arangodb::fuerte::MessageHeader;
using Response = arangodb::fuerte::Response;
using RestVerb = arangodb::fuerte::RestVerb;

namespace fu = ::arangodb::fuerte;

using namespace arangodb;
using namespace arangodb::arangobench;

struct BenchRunResult {
  double time;
  size_t failures;
  size_t incomplete;
  double requestTime;
  
  void update(double _time, size_t _failures, size_t _incomplete, double _requestTime) {
    time = _time;
    failures = _failures;
    incomplete = _incomplete;
    requestTime = _requestTime;
  }
};

void printResult(TestConfig const& config, BenchRunResult const& result) {
  std::cout << "Total request/response duration (sum of all threads): "
  << std::fixed << result.requestTime << " s" << std::endl;
  
  std::cout << "Request/response duration (per thread): " << std::fixed
  << (result.requestTime / (double)config._concurrency) << " s" << std::endl;
  
  std::cout << "Time needed per operation: " << std::fixed
  << (result.time / config._operations) << " s" << std::endl;
  
  std::cout << "Time needed per operation per thread: " << std::fixed
  << (result.time / (double)config._operations * (double)config._concurrency) << " s"
  << std::endl;
  
  std::cout << "Operations per second rate: " << std::fixed
  << ((double)config._operations / result.time) << std::endl;
  
  std::cout << "Elapsed time since start: " << std::fixed << result.time << " s"
  << std::endl
  << std::endl;
  
  if (result.failures > 0) {
    std::cout << result.failures << " arangobench request(s) failed!";
  }
  if (result.incomplete > 0) {
    std::cout << result.incomplete
    << " arangobench requests with incomplete results!";
  }
}

void report(TestConfig const& config, std::vector<BenchRunResult> results) {
  std::cout << std::endl;
  
  std::cout << "Total number of operations: " << config._operations
  << ", runs: " << config._runs
  /*<< ", replication factor: " << _replicationFactor
  << ", number of shards: " << _numberOfShards
  << ", wait for sync: " << (_waitForSync ? "true" : "false")*/
  << ", concurrency level (threads): " << config._concurrency << std::endl;
  
  std::cout << "Test case: " << config._testCase << ", complexity: " << config._complexity
  << ", database: '" << "_system" << "', collection: '"
  << config._collection << "'" << std::endl;
  
  std::sort(results.begin(), results.end(),
            [](BenchRunResult a, BenchRunResult b) { return a.time < b.time; });
  
  BenchRunResult output{0, 0, 0, 0};
  if (config._runs > 1) {
    size_t size = results.size();
    std::cout << std::endl;
    std::cout << "Printing fastest result" << std::endl;
    std::cout << "=======================" << std::endl;
    printResult(config, results[0]);
    
    std::cout << "Printing slowest result" << std::endl;
    std::cout << "=======================" << std::endl;
    printResult(config, results[size - 1]);
    
    std::cout << "Printing median result" << std::endl;
    std::cout << "=======================" << std::endl;
    size_t mid = (size_t)size / 2;
    if (size % 2 == 0) {
      output.update(
                    (results[mid - 1].time + results[mid].time) / 2,
                    (results[mid - 1].failures + results[mid].failures) / 2,
                    (results[mid - 1].incomplete + results[mid].incomplete) / 2,
                    (results[mid - 1].requestTime + results[mid].requestTime) / 2);
    } else {
      output = results[mid];
    }
  } else if (config._runs > 0) {
    output = results[0];
  }
  printResult(config, output);
}


static void usage(char const* name) {
  // clang-format off
  std::cout << "Usage: " << name << " [OPTIONS]" << "\n\n"
            << "OPTIONS:\n"
            << "  --host tcp://127.0.0.1:8529\n"
            << "  --test insert|crud\n"
            << "  --collection name\n"
            << "  --concurrency numThreads\n"
            << "  --complexity 1\n"
            << "  --requests numRequests\n"
            << " example: fuertebench --host vst://127.0.0.1:8530 --test insert --collection benchmark"
            << std::endl;
  // clang-format on
}

static bool isOption(char const* arg, char const* expected) {
  return (strcmp(arg, expected) == 0);
}

static std::string parseString(int argc, char* argv[], int& i) {
  ++i;

  if (i >= argc) {
    usage(argv[0]);
    exit(EXIT_FAILURE);
  }

  return argv[i];
}

int main(int argc, char* argv[]) {
  std::string host = "http://127.0.0.1:8529";
  const std::string documentPath = "/_api/document";
  const std::string collectionPath = "/_api/collection";
  arangodb::TestConfig config;
  config._username = "root";

  arangodb::fuerte::StringMap meta;
  arangodb::fuerte::StringMap parameter;

//#warning TODO: add database flag

  bool allowFlags = true;
  int i = 1;

  try {
    while (i < argc) {
      char const* p = argv[i];

      if (allowFlags && isOption(p, "--help")) {
        usage(argv[0]);
        return EXIT_SUCCESS;
      } else if (allowFlags && (isOption(p, "-H") || isOption(p, "--host"))) {
        host = parseString(argc, argv, i);
      } else if (allowFlags && (isOption(p, "-c") || isOption(p, "--collection"))) {
        config._collection = parseString(argc, argv, i);
      } else if (allowFlags &&
                 (isOption(p, "-t") || isOption(p, "--test"))) {
        config._testCase = parseString(argc, argv, i);
      } else if (allowFlags && isOption(p, "--concurrency")) {
        config._concurrency = std::atol(parseString(argc, argv, i).c_str());
      } else if (allowFlags &&
                 (isOption(p, "-r") || isOption(p, "--requests"))) {
        config._operations = std::atol(parseString(argc, argv, i).c_str());
      } else if (allowFlags &&
                 (isOption(p, "-n") || isOption(p, "--complexity"))) {
        config._complexity = std::atol(parseString(argc, argv, i).c_str());
      } else if (allowFlags && isOption(p, "--")) {
        allowFlags = false;
      } else {
        usage(argv[0]);
        return EXIT_FAILURE;
      }
      ++i;
    }
  } catch (std::exception const& ex) {
    std::cerr << "cannot parse arguments: " << ex.what() << std::endl;
    exit(EXIT_FAILURE);
  }
  
  EventLoopService eventLoopService(config._concurrency);
  arangodb::fuerte::ConnectionBuilder builder;
  try {
    builder.host(host);
  } catch (std::exception const& ex) {
    std::cerr << "cannot understand server-url: " << ex.what() << std::endl;
    exit(EXIT_FAILURE);
  }
  builder.user(config._username).password(config._password);
  
  std::unordered_set<std::string> cases = {"version",
    "stream-cursor",
    "document",
    "collection",
    "import-document",
    "hash",
    "skiplist",
    "edge",
    "shapes",
    "shapes-append",
    "random-shapes",
    "crud",
    "crud-append",
    "crud-write-read",
    "aqltrx",
    "counttrx",
    /*"multitrx",
    "multi-collection",
    "aqlinsert",
    "aqlv8"*/};
  
  if (cases.find(config._testCase) == cases.end()) {
    std::cerr << "invalid test case name '" << config._testCase << "'";
    exit(EXIT_FAILURE);
  }
  
  double const stepSize = (double)config._operations / (double)config._concurrency;
  int64_t realStep = (int64_t)stepSize;
  
  if (stepSize - (double)((int64_t)stepSize) > 0.0) {
    realStep++;
  }
  
  if (realStep % 1000 != 0) {
    realStep += 1000 - (realStep % 1000);
  }
  
  // add some more offset so we don't get into trouble with threads of different
  // speed
  realStep += 10000;
  
  std::vector<std::thread> threads;
  std::mutex startMutex;
  std::condition_variable startCondition;
  std::atomic<size_t> started(0);
  auto startCb = [&] {
    std::unique_lock<std::mutex> guard(startMutex);
    started++;
    startCondition.wait(guard);
  };
  
  bool ok = true;
  std::vector<BenchRunResult> results;
  for (uint64_t j = 0; j < config._runs; j++) {
    std::cout << "starting threads...\n";
    BenchmarkCounter<unsigned long> operationsCounter(0, (unsigned long)config._operations);

    // start client threads
    started = 0;
    for (uint64_t i = 0; i < config._concurrency; ++i) {
      auto conn = builder.connect(eventLoopService);
      
      threads.emplace_back([&]() {
        std::unique_ptr<BenchmarkOperation> benchmark(GetTestCase(config._testCase, config));
        BenchmarkThread b(std::move(benchmark), startCb, static_cast<int>(i), &operationsCounter,
                        std::move(conn));
        b.setOffset((size_t)(i * realStep));
        b.run();
      });
    }
    
    // give all threads a chance to start so they will not miss the broadcast
    while (started.load() < config._concurrency) {
      std::this_thread::sleep_for(std::chrono::microseconds(5000));
    }
    
    /*if (_delay) {
      status("sleeping (startup delay)...");
      std::this_thread::sleep_for(std::chrono::seconds(10));
    }*/
    
    std::cout << "executing tests...\n";
    auto start = std::chrono::steady_clock::now();
    
    // broadcast the start signal to all threads
    startCondition.notify_all();
    
    size_t const stepValue = static_cast<size_t>(config.operations() / 20);
    size_t nextReportValue = stepValue;
    
    if (nextReportValue < 100) {
      nextReportValue = 100;
    }
    
    while (true) {
      size_t const numOperations = operationsCounter.getDone();
      
      if (numOperations >= (size_t)config.operations()) {
        break;
      }
      
      if (numOperations >= nextReportValue) {
        std::cout << "number of operations: " << nextReportValue << "\n";
        nextReportValue += stepValue;
      }
      
      std::this_thread::sleep_for(std::chrono::microseconds(10000));
    }
    
    std::chrono::duration<float> time = std::chrono::steady_clock::now() - start;
    double requestTime = 0.0;
    
    /*for (size_t i = 0; i < static_cast<size_t>(config.__concurreny); ++i) {
      requestTime += threads[i]->getTime();
    }*/
    
    if (operationsCounter.failures() > 0) {
      ok = false;
    }
    
    results.emplace_back(BenchRunResult{
      time.count(), operationsCounter.failures(),
      operationsCounter.incompleteFailures(), 0.0
    });
    for (std::thread& t : threads) {
      t.join();
    }
    threads.clear();
  }
  
  report(config, results);
  
  return EXIT_SUCCESS;
}
