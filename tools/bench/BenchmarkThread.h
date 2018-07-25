////////////////////////////////////////////////////////////////////////////////
/// DISCLAIMER
///
/// Copyright 2014-2016 ArangoDB GmbH, Cologne, Germany
/// Copyright 2004-2014 triAGENS GmbH, Cologne, Germany
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
/// @author Jan Steemann
////////////////////////////////////////////////////////////////////////////////

#ifndef ARANGODB_BENCHMARK_BENCHMARK_THREAD_H
#define ARANGODB_BENCHMARK_BENCHMARK_THREAD_H 1

#include <fuerte/fuerte.h>
#include <fuerte/requests.h>

#include "BenchmarkCounter.h"
#include "BenchmarkOperation.h"

namespace arangodb {
namespace arangobench {

class BenchmarkThread {
 public:
  BenchmarkThread(std::unique_ptr<BenchmarkOperation> ops, std::function<void(void)> callback,
                  int threadNumber, BenchmarkCounter<unsigned long>* operationsCounter,
                  std::shared_ptr<fuerte::Connection> const& conn)
  : _operation(std::move(ops)),
        _callback(callback),
        _threadNumber(threadNumber),
        _warningCount(0),
        _operationsCounter(operationsCounter),
        _connection(conn),
        _offset(0),
        _counter(0),
        _time(0.0) {
  }

  //////////////////////////////////////////////////////////////////////////////
  /// @brief the thread program
  //////////////////////////////////////////////////////////////////////////////

  void run() {
    
    auto result =
    _connection->sendRequest(fuerte::createRequest(fuerte::RestVerb::Get, "/_api/version"));

    // test the connection
    if (!result || result->statusCode() != fuerte::StatusOK) {
      std::cerr << "could not connect to server";
      exit(1);
    }

    // if we're the first thread, set up the test
    if (_threadNumber == 0) {
      if (!_operation->setUp(_connection.get())) {
        std::cerr << "could not set up the test";
        exit(1);
      }
    }
    
    // wait for start condition to be broadcasted
    _callback();
    
    fuerte::WaitGroup wg;
    auto cb = [&] (fuerte::Error e,
                   std::shared_ptr<fuerte::Request> const& req,
                   std::shared_ptr<fuerte::Response> const& res) {
      fuerte::WaitGroupDone done(wg);
      if (e) {
        _operationsCounter->incFailures(1);
        if (fuerte::intToError(e) == fuerte::ErrorCondition::Timeout) {
          _operationsCounter->incIncompleteFailures(1);
        }
        _warningCount++;
        if (_warningCount < MaxWarnings) {
          std::cerr << "single operation failed because server did not reply";
        }
      } else if (res->statusCode() >= 500) {
        _operationsCounter->incFailures(1);
        _warningCount++;
        if (_warningCount < MaxWarnings) {
          std::string c(reinterpret_cast<const char*>(res->payload().data()),
                        res->payload().size());
          std::cerr << "request for URL '" << req->header.path << "' failed with HTTP code "
          << res->statusCode() << ": " << c;
        } else if (_warningCount == MaxWarnings) {
          std::cerr << "...more warnings...";
        }
      }
    };

    while (!_isStopping) {
      unsigned long numOps = _operationsCounter->next(1);
      if (numOps == 0) {
        break;
      }

      size_t const threadCounter = _counter++;
      size_t const globalCounter = _offset + threadCounter;
      fuerte::RestVerb const type = _operation->type(_threadNumber, threadCounter, globalCounter);
      std::string const url = _operation->url(_threadNumber, threadCounter, globalCounter);
      
      // std::cout << "thread number #" << _threadNumber << ", threadCounter " <<
      // threadCounter << ", globalCounter " << globalCounter << "\n";
      VPackSlice payload = _operation->payload(_threadNumber, threadCounter, globalCounter);
      
      std::unique_ptr<fuerte::Request> request(new fuerte::Request());
      request->header.restVerb = type;
      request->header.contentType(fuerte::ContentType::VPack);
      request->header.acceptType(fuerte::ContentType::VPack);
      
      if (!payload.isNone()) {
        request->addVPack(payload);
      }
      
      //auto now = std::chrono::steady_clock::now();
      wg.add(1);
      _connection->sendRequest(std::move(request), cb);
      
      /*httpclient::SimpleHttpResult* result =
      _httpClient->request(type, url, payload, payloadLength, _headers);
      _time += TRI_microtime() - start;*/
      
      _operationsCounter->done(1);
    }
    
    wg.wait();
  }

 private:
  //////////////////////////////////////////////////////////////////////////////
  /// @brief request location rewriter (injects database name)
  //////////////////////////////////////////////////////////////////////////////

  static std::string rewriteLocation(void* data, std::string const& location) {
    auto t = static_cast<arangobench::BenchmarkThread*>(data);

    assert(t != nullptr);

    if (location.compare(0, 5, "/_db/") == 0) {
      // location already contains /_db/
      return location;
    }

    if (location[0] == '/') {
      return std::string("/_db/" + t->_databaseName + location);
    } 
    return std::string("/_db/" + t->_databaseName + "/" + location);
  }

 public:
  //////////////////////////////////////////////////////////////////////////////
  /// @brief set the threads offset value
  //////////////////////////////////////////////////////////////////////////////

  void setOffset(size_t offset) { _offset = offset; }

  //////////////////////////////////////////////////////////////////////////////
  /// @brief return the total time accumulated by the thread
  //////////////////////////////////////////////////////////////////////////////

  double getTime() const { return _time; }

 private:
  //////////////////////////////////////////////////////////////////////////////
  /// @brief the operation to benchmark
  //////////////////////////////////////////////////////////////////////////////

  std::unique_ptr<BenchmarkOperation> _operation;

  //////////////////////////////////////////////////////////////////////////////
  /// @brief start callback function
  //////////////////////////////////////////////////////////////////////////////

  std::function<void(void)> _callback;

  //////////////////////////////////////////////////////////////////////////////
  /// @brief our thread number
  //////////////////////////////////////////////////////////////////////////////

  int _threadNumber;

  //////////////////////////////////////////////////////////////////////////////
  /// @brief warning counter
  //////////////////////////////////////////////////////////////////////////////

  int _warningCount;

  //////////////////////////////////////////////////////////////////////////////
  /// @brief benchmark counter
  //////////////////////////////////////////////////////////////////////////////

  arangobench::BenchmarkCounter<unsigned long>* _operationsCounter;

  //////////////////////////////////////////////////////////////////////////////
  /// @brief database name
  //////////////////////////////////////////////////////////////////////////////

  std::string const _databaseName;

  //////////////////////////////////////////////////////////////////////////////
  /// @brief HTTP username
  //////////////////////////////////////////////////////////////////////////////

  std::string const _username;

  //////////////////////////////////////////////////////////////////////////////
  /// @brief HTTP password
  //////////////////////////////////////////////////////////////////////////////

  std::string const _password;

  //////////////////////////////////////////////////////////////////////////////
  /// @brief underlying http client
  //////////////////////////////////////////////////////////////////////////////

  std::shared_ptr<arangodb::fuerte::Connection> _connection;

  //////////////////////////////////////////////////////////////////////////////
  /// @brief thread offset value
  //////////////////////////////////////////////////////////////////////////////

  size_t _offset;

  //////////////////////////////////////////////////////////////////////////////
  /// @brief thread counter value
  //////////////////////////////////////////////////////////////////////////////

  size_t _counter;

  //////////////////////////////////////////////////////////////////////////////
  /// @brief time
  //////////////////////////////////////////////////////////////////////////////

  double _time;

  //////////////////////////////////////////////////////////////////////////////
  /// @brief maximum number of warnings to be displayed per thread
  //////////////////////////////////////////////////////////////////////////////

  static int const MaxWarnings = 5;

  bool _isStopping = false;
};
}
}

#endif
