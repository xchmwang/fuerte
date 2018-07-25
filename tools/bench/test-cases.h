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

#pragma once
#ifndef ARANGODB_BENCHMARK_TEST_CASES
#define ARANGODB_BENCHMARK_TEST_CASES 1

#include <fuerte/connection.h>

#include "BenchmarkOperation.h"
#include "TestConfig.h"

#include <velocypack/Slice.h>
#include <velocypack/velocypack-aliases.h>

namespace arangodb {
  
using arangodb::arangobench::BenchmarkOperation;

static bool DeleteCollection(fuerte::Connection*, std::string const&);

static bool CreateCollection(fuerte::Connection*, std::string const&, int const);

/*static bool CreateDocument(fuerte::Connection*, std::string const&,
                           std::string const&);*/

static bool CreateIndex(fuerte::Connection*, std::string const&,
                        std::string const&, std::vector<std::string> const& fields);

struct VersionTest : public BenchmarkOperation {
  VersionTest(TestConfig const& config) : BenchmarkOperation(config) { _url = "/_api/version"; }

  bool setUp(fuerte::Connection* client) override { return true; }

  void tearDown() override {}

  std::string url(int const threadNumber, size_t const threadCounter,
                  size_t const globalCounter) override {
    return _url;
  }

  arangodb::fuerte::RestVerb type(int const threadNumber, size_t const threadCounter,
                         size_t const globalCounter) override {
    return arangodb::fuerte::RestVerb::Get;
  }

  VPackSlice payload(int const threadNumber, size_t const threadCounter,
                    size_t const globalCounter) override {
    return VPackSlice::noneSlice();
  }

  std::string _url;
};

struct DocumentCrudAppendTest : public BenchmarkOperation {
  DocumentCrudAppendTest(TestConfig const& config) : BenchmarkOperation(config) {}

  bool setUp(fuerte::Connection* client) override {
    return DeleteCollection(client, _config.collection()) &&
           CreateCollection(client, _config.collection(), 2);
  }

  void tearDown() override {}

  std::string url(int const threadNumber, size_t const threadCounter,
                  size_t const globalCounter) override {
    size_t const mod = globalCounter % 4;

    if (mod == 0) {
      return std::string("/_api/document?collection=" +
                         _config.collection());
    } else {
      size_t keyId = (size_t)(globalCounter / 4);
      std::string const key = "testkey" + std::to_string(keyId);

      return std::string("/_api/document/" + _config.collection() + "/" +
                         key);
    }
  }

  arangodb::fuerte::RestVerb type(int const threadNumber, size_t const threadCounter,
                                  size_t const globalCounter) override {
    size_t const mod = globalCounter % 4;

    if (mod == 0) {
      return arangodb::fuerte::RestVerb::Post;
    } else if (mod == 1) {
      return arangodb::fuerte::RestVerb::Get;
    } else if (mod == 2) {
      return arangodb::fuerte::RestVerb::Patch;
    } else if (mod == 3) {
      return arangodb::fuerte::RestVerb::Get;
    } else {
      assert(false);
      return arangodb::fuerte::RestVerb::Get;
    }
  }

  VPackSlice payload(int const threadNumber, size_t const threadCounter,
                    size_t const globalCounter) override {
    size_t const mod = globalCounter % 4;

    if (mod == 0 || mod == 2) {
      uint64_t const n = _config.complexity();
      _builder.clear();
      _builder.openObject();

      size_t keyId = (size_t)(globalCounter / 4);
      _builder.add("_key", VPackValue("testkey" + std::to_string(keyId)));

      const std::string val("value");
      for (uint64_t i = 1; i <= n; ++i) {
        _builder.add(val + std::to_string(i), VPackValue(mod == 0));
      }
      _builder.close();
      return _builder.slice();
    } else if (mod == 1 || mod == 3) {
      return VPackSlice::noneSlice();
    } else {
      assert(false);
      return VPackSlice::noneSlice();
    }
  }
};

struct DocumentCrudWriteReadTest : public BenchmarkOperation {
  DocumentCrudWriteReadTest(TestConfig const& config) : BenchmarkOperation(config) {}

  bool setUp(fuerte::Connection* client) override {
    return DeleteCollection(client, _config.collection()) &&
           CreateCollection(client, _config.collection(), 2);
  }

  void tearDown() override {}

  std::string url(int const threadNumber, size_t const threadCounter,
                  size_t const globalCounter) override {
    size_t const mod = globalCounter % 2;

    if (mod == 0) {
      return std::string("/_api/document?collection=" +
                         _config.collection());
    } else {
      size_t keyId = (size_t)(globalCounter / 2);
      std::string const key = "testkey" + std::to_string(keyId);

      return std::string("/_api/document/" + _config.collection() + "/" +
                         key);
    }
  }

  arangodb::fuerte::RestVerb type(int const threadNumber, size_t const threadCounter,
                                  size_t const globalCounter) override {
    size_t const mod = globalCounter % 2;

    if (mod == 0) {
      return arangodb::fuerte::RestVerb::Post;
    } else {
      return arangodb::fuerte::RestVerb::Get;
    }
  }

  VPackSlice payload(int const threadNumber, size_t const threadCounter,
                    size_t const globalCounter) override {
    size_t const mod = globalCounter % 2;

    if (mod == 0) {
      uint64_t const n = _config.complexity();
      _builder.clear();
      _builder.openObject();
      
      size_t keyId = (size_t)(globalCounter / 2);
      _builder.add("_key", VPackValue("testkey" + std::to_string(keyId)));

      const std::string val("value");
      for (uint64_t i = 1; i <= n; ++i) {
        _builder.add(val + std::to_string(i), VPackValue(true));
      }
      _builder.close();

      return _builder.slice();
    } else {
      return VPackSlice::noneSlice();
    }
  }
};

struct ShapesTest : public BenchmarkOperation {
  ShapesTest(TestConfig const& config) : BenchmarkOperation(config) {}

  bool setUp(fuerte::Connection* client) override {
    return DeleteCollection(client, _config.collection()) &&
           CreateCollection(client, _config.collection(), 2);
  }

  void tearDown() override {}

  std::string url(int const threadNumber, size_t const threadCounter,
                  size_t const globalCounter) override {
    size_t const mod = globalCounter % 3;

    if (mod == 0) {
      return std::string("/_api/document?collection=" +
                         _config.collection());
    } else {
      size_t keyId = (size_t)(globalCounter / 3);
      std::string const key = "testkey" + std::to_string(keyId);

      return std::string("/_api/document/" + _config.collection() + "/" +
                         key);
    }
  }

  arangodb::fuerte::RestVerb type(int const threadNumber, size_t const threadCounter,
                         size_t const globalCounter) override {
    size_t const mod = globalCounter % 3;

    if (mod == 0) {
      return arangodb::fuerte::RestVerb::Post;
    } else if (mod == 1) {
      return arangodb::fuerte::RestVerb::Get;
    } else {
      return arangodb::fuerte::RestVerb::Delete;
    }
  }

  VPackSlice payload(int const threadNumber, size_t const threadCounter,
                    size_t const globalCounter) override {
    size_t const mod = globalCounter % 3;

    if (mod == 0) {
      uint64_t const n = _config.complexity();
      _builder.clear();
      _builder.openObject();
      
      size_t keyId = (size_t)(globalCounter / 2);
      _builder.add("_key", VPackValue("testkey" + std::to_string(keyId)));
      
      const std::string val("value");
      for (uint64_t i = 1; i <= n; ++i) {
        uint64_t mod = _config.operations() / 10;
        if (mod < 100) {
          mod = 100;
        }
        
        _builder.add(val + std::to_string((globalCounter + i) % mod),
                     VPackValue("some bogus string value to fill up the datafile..."));
      }
      _builder.close();
      
      return _builder.slice();
    } else {
      return VPackSlice::noneSlice();
    }
  }
};

struct ShapesAppendTest : public BenchmarkOperation {
  ShapesAppendTest(TestConfig const& config) : BenchmarkOperation(config) {}

  bool setUp(fuerte::Connection* client) override {
    return DeleteCollection(client, _config.collection()) &&
           CreateCollection(client, _config.collection(), 2);
  }

  void tearDown() override {}

  std::string url(int const threadNumber, size_t const threadCounter,
                  size_t const globalCounter) override {
    size_t const mod = globalCounter % 2;

    if (mod == 0) {
      return std::string("/_api/document?collection=" +
                         _config.collection());
    } else {
      size_t keyId = (size_t)(globalCounter / 2);
      std::string const key = "testkey" + std::to_string(keyId);

      return std::string("/_api/document/" + _config.collection() + "/" +
                         key);
    }
  }

  arangodb::fuerte::RestVerb type(int const threadNumber, size_t const threadCounter,
                         size_t const globalCounter) override {
    size_t const mod = globalCounter % 2;

    if (mod == 0) {
      return arangodb::fuerte::RestVerb::Post;
    }
    return arangodb::fuerte::RestVerb::Get;
  }

  VPackSlice payload(int const threadNumber, size_t const threadCounter,
                    size_t const globalCounter) override {
    size_t const mod = globalCounter % 2;

    if (mod == 0) {
      uint64_t const n = _config.complexity();
      _builder.clear();
      _builder.openObject();
      
      size_t keyId = (size_t)(globalCounter / 4);
      _builder.add("_key", VPackValue("testkey" + std::to_string(keyId)));
      
      const std::string val("value");
      for (uint64_t i = 1; i <= n; ++i) {
        uint64_t mod = _config.operations() / 10;
        if (mod < 100) {
          mod = 100;
        }
        _builder.add(val + std::to_string((globalCounter + i) % mod),
        VPackValue("some bogus string value to fill up the datafile..."));
      }
      _builder.close();
      return _builder.slice();
    } else {
      return VPackSlice::noneSlice();
    }
  }
};
/*
struct RandomShapesTest : public BenchmarkOperation {
  RandomShapesTest(TestConfig const& config) : BenchmarkOperation(config) {
    _randomValue = RandomGenerator::interval(UINT32_MAX);
  }

  bool setUp(fuerte::Connection* client) override {
    return DeleteCollection(client, _config.collection()) &&
           CreateCollection(client, _config.collection(), 2);
  }

  void tearDown() override {}

  std::string url(int const threadNumber, size_t const threadCounter,
                  size_t const globalCounter) override {
    size_t const mod = globalCounter % 3;

    if (mod == 0) {
      return std::string("/_api/document?collection=") +
             _config.collection();
    } else {
      size_t keyId = (size_t)(globalCounter / 3);
      std::string const key = "testkey" + std::to_string(keyId);

      return std::string("/_api/document/") + _config.collection() +
             std::string("/") + key;
    }
  }

  arangodb::fuerte::RestVerb type(int const threadNumber, size_t const threadCounter,
                         size_t const globalCounter) override {
    size_t const mod = globalCounter % 3;

    if (mod == 0) {
      return arangodb::fuerte::RestVerb::Post;
    } else if (mod == 1) {
      return arangodb::fuerte::RestVerb::Get;
    } else {
      return arangodb::fuerte::RestVerb::DELETE_REQ;
    }
  }

  VPackSlice payload(int const threadNumber, size_t const threadCounter,
                    size_t const globalCounter) override {
    size_t const mod = globalCounter % 3;

    if (mod == 0) {
      uint64_t const n = _config.complexity();
      TRI_string_buffer_t* buffer;

      buffer = TRI_CreateSizedStringBuffer(256);
      TRI_AppendStringStringBuffer(buffer, "{\"_key\":\"");

      size_t keyId = (size_t)(globalCounter / 3);
      std::string const key = "testkey" + std::to_string(keyId);
      TRI_AppendString2StringBuffer(buffer, key.c_str(), key.size());
      TRI_AppendStringStringBuffer(buffer, "\"");

      uint32_t const t =
          _randomValue % (uint32_t)(globalCounter + threadNumber + 1);

      for (uint64_t i = 1; i <= n; ++i) {
        TRI_AppendStringStringBuffer(buffer, ",\"value");
        TRI_AppendUInt64StringBuffer(buffer, (uint64_t)(globalCounter + i));
        if (t % 3 == 0) {
          TRI_AppendStringStringBuffer(buffer, "\":true");
        } else if (t % 3 == 1) {
          TRI_AppendStringStringBuffer(buffer, "\":null");
        } else
          TRI_AppendStringStringBuffer(
              buffer,
              "\":\"some bogus string value to fill up the datafile...\"");
      }

      TRI_AppendStringStringBuffer(buffer, "}");

      *length = TRI_LengthStringBuffer(buffer);
      *mustFree = true;
      char* ptr = TRI_StealStringBuffer(buffer);
      TRI_FreeStringBuffer(buffer);

      return (char const*)ptr;
    } else {
      *length = 0;
      *mustFree = false;
      return (char const*)nullptr;
    }
  }

  uint32_t _randomValue;
};*/

struct DocumentCrudTest : public BenchmarkOperation {
  DocumentCrudTest(TestConfig const& config) : BenchmarkOperation(config) {}

  bool setUp(fuerte::Connection* client) override {
    return DeleteCollection(client, _config.collection()) &&
           CreateCollection(client, _config.collection(), 2);
  }

  void tearDown() override {}

  std::string url(int const threadNumber, size_t const threadCounter,
                  size_t const globalCounter) override {
    size_t const mod = globalCounter % 5;

    if (mod == 0) {
      return std::string("/_api/document?collection=" +
                         _config.collection());
    } else {
      size_t keyId = (size_t)(globalCounter / 5);
      std::string const key = "testkey" + std::to_string(keyId);

      return std::string("/_api/document/" + _config.collection() + "/" +
                         key);
    }
  }

  arangodb::fuerte::RestVerb type(int const threadNumber, size_t const threadCounter,
                         size_t const globalCounter) override {
    size_t const mod = globalCounter % 5;

    if (mod == 0) {
      return arangodb::fuerte::RestVerb::Post;
    } else if (mod == 1) {
      return arangodb::fuerte::RestVerb::Get;
    } else if (mod == 2) {
      return arangodb::fuerte::RestVerb::Patch;
    } else if (mod == 3) {
      return arangodb::fuerte::RestVerb::Get;
    } else if (mod == 4) {
      return arangodb::fuerte::RestVerb::Delete;
    } else {
      assert(false);
      return arangodb::fuerte::RestVerb::Get;
    }
  }

  VPackSlice payload(int const threadNumber, size_t const threadCounter,
                    size_t const globalCounter) override {
    size_t const mod = globalCounter % 5;

    if (mod == 0 || mod == 2) {
      uint64_t const n = _config.complexity();
      _builder.clear();
      _builder.openObject();
      
      size_t keyId = (size_t)(globalCounter / 5);
      _builder.add("_key", VPackValue("testkey" + std::to_string(keyId)));
      
      const std::string val("value");
      for (uint64_t i = 1; i <= n; ++i) {
        _builder.add(val + std::to_string(i), VPackValue(mod == 0));
      }
      _builder.close();
      
      return _builder.slice();
    } else if (mod == 1 || mod == 3 || mod == 4) {
      return VPackSlice::noneSlice();
    } else {
      assert(false);
      return VPackSlice::noneSlice();
    }
  }
};

struct EdgeCrudTest : public BenchmarkOperation {
  EdgeCrudTest(TestConfig const& config) : BenchmarkOperation(config) {}

  bool setUp(fuerte::Connection* client) override {
    return DeleteCollection(client, _config.collection()) &&
           CreateCollection(client, _config.collection(), 3);
  }

  void tearDown() override {}

  std::string url(int const threadNumber, size_t const threadCounter,
                  size_t const globalCounter) override {
    size_t const mod = globalCounter % 4;

    if (mod == 0) {
      return std::string("/_api/document?collection=" +
                         _config.collection());
    } else {
      size_t keyId = (size_t)(globalCounter / 4);
      std::string const key = "testkey" + std::to_string(keyId);

      return std::string("/_api/document/" + _config.collection() + "/" +
                         key);
    }
  }

  arangodb::fuerte::RestVerb type(int const threadNumber, size_t const threadCounter,
                         size_t const globalCounter) override {
    size_t const mod = globalCounter % 4;

    if (mod == 0) {
      return arangodb::fuerte::RestVerb::Post;
    } else if (mod == 1) {
      return arangodb::fuerte::RestVerb::Get;
    } else if (mod == 2) {
      return arangodb::fuerte::RestVerb::Patch;
    } else if (mod == 3) {
      return arangodb::fuerte::RestVerb::Get;
    }
    /*
    else if (mod == 4) {
      return arangodb::fuerte::RestVerb::DELETE_REQ;
    }
    */
    else {
      assert(false);
      return arangodb::fuerte::RestVerb::Get;
    }
  }

  VPackSlice payload(int const threadNumber, size_t const threadCounter,
                    size_t const globalCounter) override {
    size_t const mod = globalCounter % 4;

    if (mod == 0 || mod == 2) {
      uint64_t const n = _config.complexity();
      _builder.clear();
      _builder.openObject();
      
      size_t keyId = (size_t)(globalCounter / 4);
      _builder.add("_key", VPackValue("testkey" + std::to_string(keyId)));
      
      if (mod == 0) {
        // append edge information
        _builder.add("_from", VPackValue(_config.collection() + "/testfrom" + std::to_string(globalCounter)));
        _builder.add("_to", VPackValue(_config.collection() + "/testto" + std::to_string(globalCounter)));
      }

      const std::string val("value");
      for (uint64_t i = 1; i <= n; ++i) {
        _builder.add(val + std::to_string(i), VPackValue(mod == 0));
      }
      _builder.close();
      
      return _builder.slice();
    } else if (mod == 1 || mod == 3 || mod == 4) {
      return VPackSlice::noneSlice();
    } else {
      assert(false);
      return VPackSlice::noneSlice();
    }
  }
};

struct SkiplistTest : public BenchmarkOperation {
  SkiplistTest(TestConfig const& config) : BenchmarkOperation(config) {}

  bool setUp(fuerte::Connection* client) override {
    return DeleteCollection(client, _config.collection()) &&
           CreateCollection(client, _config.collection(), 2) &&
           CreateIndex(client, _config.collection(), "skiplist",
                       {"value"});
  }

  void tearDown() override {}

  std::string url(int const threadNumber, size_t const threadCounter,
                  size_t const globalCounter) override {
    size_t const mod = globalCounter % 4;

    if (mod == 0) {
      return std::string("/_api/document?collection=" +
                         _config.collection());
    } else {
      size_t keyId = (size_t)(globalCounter / 4);
      std::string const key = "testkey" + std::to_string(keyId);

      return std::string("/_api/document/" + _config.collection() + "/" +
                         key);
    }
  }

  arangodb::fuerte::RestVerb type(int const threadNumber, size_t const threadCounter,
                         size_t const globalCounter) override {
    size_t const mod = globalCounter % 4;

    if (mod == 0) {
      return arangodb::fuerte::RestVerb::Post;
    } else if (mod == 1) {
      return arangodb::fuerte::RestVerb::Get;
    } else if (mod == 2) {
      return arangodb::fuerte::RestVerb::Patch;
    } else if (mod == 3) {
      return arangodb::fuerte::RestVerb::Get;
    } else {
      assert(false);
      return arangodb::fuerte::RestVerb::Get;
    }
  }

  VPackSlice payload(int const threadNumber, size_t const threadCounter,
                    size_t const globalCounter) override {
    size_t const mod = globalCounter % 4;

    if (mod == 0 || mod == 2) {
      _builder.clear();
      _builder.openObject();
      
      size_t keyId = (size_t)(globalCounter / 4);
      _builder.add("_key", VPackValue("testkey" + std::to_string(keyId)));
      _builder.add("value", VPackValue((uint32_t)threadCounter));
      
      _builder.close();
      return _builder.slice();
      
    } else if (mod == 1 || mod == 3 || mod == 4) {
      return VPackSlice::noneSlice();
    } else {
      assert(false);
      return VPackSlice::noneSlice();
    }
  }
};

struct HashTest : public BenchmarkOperation {
  HashTest(TestConfig const& config) : BenchmarkOperation(config) {}

  bool setUp(fuerte::Connection* client) override {
    return DeleteCollection(client, _config.collection()) &&
           CreateCollection(client, _config.collection(), 2) &&
           CreateIndex(client, _config.collection(), "hash",
                       {"value"});
  }

  void tearDown() override {}

  std::string url(int const threadNumber, size_t const threadCounter,
                  size_t const globalCounter) override {
    size_t const mod = globalCounter % 4;

    if (mod == 0) {
      return std::string("/_api/document?collection=" +
                         _config.collection());
    } else {
      size_t keyId = (size_t)(globalCounter / 4);
      std::string const key = "testkey" + std::to_string(keyId);

      return std::string("/_api/document/" + _config.collection() + "/" +
                         key);
    }
  }

  arangodb::fuerte::RestVerb type(int const threadNumber, size_t const threadCounter,
                         size_t const globalCounter) override {
    size_t const mod = globalCounter % 4;

    if (mod == 0) {
      return arangodb::fuerte::RestVerb::Post;
    } else if (mod == 1) {
      return arangodb::fuerte::RestVerb::Get;
    } else if (mod == 2) {
      return arangodb::fuerte::RestVerb::Patch;
    } else if (mod == 3) {
      return arangodb::fuerte::RestVerb::Get;
    } else {
      assert(false);
      return arangodb::fuerte::RestVerb::Get;
    }
  }

  VPackSlice payload(int const threadNumber, size_t const threadCounter,
                    size_t const globalCounter) override {
    size_t const mod = globalCounter % 4;

    if (mod == 0 || mod == 2) {
      _builder.clear();
      _builder.openObject();
      
      size_t keyId = (size_t)(globalCounter / 4);
      _builder.add("_key", VPackValue("testkey" + std::to_string(keyId)));
      _builder.add("value", VPackValue((uint32_t)threadCounter));
      
      _builder.close();
      return _builder.slice();
    } else if (mod == 1 || mod == 3 || mod == 4) {
      return VPackSlice::noneSlice();
    } else {
      assert(false);
      return VPackSlice::noneSlice();
    }
  }
};

struct DocumentImportTest : public BenchmarkOperation {
  DocumentImportTest(TestConfig const& config) : BenchmarkOperation(config), _url() {
    _url = "/_api/import?collection=" + _config.collection() +
           "&type=documents";

    uint64_t const n = _config.complexity();

    _builder.clear();
    for (uint64_t i = 0; i < n; ++i) {
      _builder.openObject();
      _builder.add("key1", VPackValue(i));
      _builder.add("key2", VPackValue(i));
      _builder.close();
    }
  }

  bool setUp(fuerte::Connection* client) override {
    return DeleteCollection(client, _config.collection()) &&
           CreateCollection(client, _config.collection(), 2);
  }

  void tearDown() override {}

  std::string url(int const threadNumber, size_t const threadCounter,
                  size_t const globalCounter) override {
    return _url;
  }

  arangodb::fuerte::RestVerb type(int const threadNumber, size_t const threadCounter,
                         size_t const globalCounter) override {
    return arangodb::fuerte::RestVerb::Post;
  }

  VPackSlice payload(int const threadNumber, size_t const threadCounter,
                    size_t const globalCounter) override {
    return _builder.slice();
  }

  std::string _url;
};

struct DocumentCreationTest : public BenchmarkOperation {
  DocumentCreationTest(TestConfig const& config) : BenchmarkOperation(config), _url() {
    _url = "/_api/document?collection=" + _config.collection();

    uint64_t const n = _config.complexity();

    _builder.clear();
    _builder.openObject();
    for (uint64_t i = 1; i <= n; ++i) {
      _builder.add("test" + std::to_string(i), VPackValue("some test value"));
    }
    _builder.close();
  }

  bool setUp(fuerte::Connection* client) override {
    return DeleteCollection(client, _config.collection()) &&
           CreateCollection(client, _config.collection(), 2);
  }

  void tearDown() override {}

  std::string url(int const threadNumber, size_t const threadCounter,
                  size_t const globalCounter) override {
    return _url;
  }

  arangodb::fuerte::RestVerb type(int const threadNumber, size_t const threadCounter,
                         size_t const globalCounter) override {
    return arangodb::fuerte::RestVerb::Post;
  }

  VPackSlice payload(int const threadNumber, size_t const threadCounter,
                    size_t const globalCounter) override {
    return _builder.slice();
  }

  std::string _url;
};

struct CollectionCreationTest : public BenchmarkOperation {
  CollectionCreationTest(TestConfig const& config) : BenchmarkOperation(config), _url() {
    _url = "/_api/collection";
  }

  bool setUp(fuerte::Connection* client) override { return true; }

  void tearDown() override {}

  std::string url(int const threadNumber, size_t const threadCounter,
                  size_t const globalCounter) override {
    return _url;
  }

  arangodb::fuerte::RestVerb type(int const threadNumber, size_t const threadCounter,
                         size_t const globalCounter) override {
    return arangodb::fuerte::RestVerb::Post;
  }

  VPackSlice payload(int const threadNumber, size_t const threadCounter,
                    size_t const globalCounter) override {
    _builder.clear();
    _builder.openObject();
    _builder.add("name", VPackValue(_config.collection() + std::to_string(++_counter)));
    _builder.close();

    return _builder.slice();
  }

  static std::atomic<uint64_t> _counter;

  std::string _url;
};

std::atomic<uint64_t> CollectionCreationTest::_counter(0);

struct TransactionAqlTest : public BenchmarkOperation {
  TransactionAqlTest(TestConfig const& config) : BenchmarkOperation(config) {}

  bool setUp(fuerte::Connection* client) override {
    _c1 = std::string(_config.collection() + "1");
    _c2 = std::string(_config.collection() + "2");
    _c3 = std::string(_config.collection() + "3");

    return DeleteCollection(client, _c1) && DeleteCollection(client, _c2) &&
           DeleteCollection(client, _c3) && CreateCollection(client, _c1, 2) &&
           CreateCollection(client, _c2, 2) && CreateCollection(client, _c3, 2);
  }

  void tearDown() override {}

  std::string url(int const threadNumber, size_t const threadCounter,
                  size_t const globalCounter) override {
    return std::string("/_api/cursor");
  }

  arangodb::fuerte::RestVerb type(int const threadNumber, size_t const threadCounter,
                         size_t const globalCounter) override {
    return arangodb::fuerte::RestVerb::Post;
  }

  VPackSlice payload(int const threadNumber, size_t const threadCounter,
                    size_t const globalCounter) override {
    size_t const mod = globalCounter % 8;
    _builder.clear();
    _builder.openObject();
    _builder.add(VPackValue("query"));
    if (mod == 0) {
      _builder.add(VPackValue("FOR c IN " + _c1 + " RETURN 1"));
    } else if (mod == 1) {
      _builder.add(VPackValue("FOR c IN " + _c2 + " RETURN 1"));
    } else if (mod == 2) {
      _builder.add(VPackValue("FOR c IN " + _c3 + " RETURN 1"));
    } else if (mod == 3) {
      _builder.add(VPackValue("FOR c1 IN " + _c1 + " FOR c2 IN " + _c2 + " RETURN 1"));
    } else if (mod == 4) {
      _builder.add(VPackValue("FOR c2 IN " + _c2 + " FOR c1 IN " + _c1 + " RETURN 1"));
    } else if (mod == 5) {
      _builder.add(VPackValue("FOR c3 IN " + _c3 + " FOR c1 IN " + _c1 + " RETURN 1"));
    } else if (mod == 6) {
      _builder.add(VPackValue("FOR c2 IN " + _c2 + " FOR c3 IN " + _c3 + " RETURN 1"));
    } else if (mod == 7) {
      _builder.add(VPackValue("FOR c1 IN " + _c1 + " FOR c2 IN " + _c2 + " FOR c3 IN " + _c3 + " RETURN 1"));
    }
    _builder.close();

    return _builder.slice();
  }

  std::string _c1;
  std::string _c2;
  std::string _c3;
};

struct TransactionCountTest : public BenchmarkOperation {
  TransactionCountTest(TestConfig const& config) : BenchmarkOperation(config) {}

  bool setUp(fuerte::Connection* client) override {
    return DeleteCollection(client, _config.collection()) &&
           CreateCollection(client, _config.collection(), 2);
  }

  void tearDown() override {}

  std::string url(int const threadNumber, size_t const threadCounter,
                  size_t const globalCounter) override {
    return std::string("/_api/transaction");
  }

  arangodb::fuerte::RestVerb type(int const threadNumber, size_t const threadCounter,
                         size_t const globalCounter) override {
    return arangodb::fuerte::RestVerb::Post;
  }

  VPackSlice payload(int const threadNumber, size_t const threadCounter,
                    size_t const globalCounter) override {
    _builder.clear();
    _builder.openObject();
    _builder.add("collections", VPackValue(VPackValueType::Object));
    _builder.add("write", VPackValue(_config.collection()));
    _builder.close(); // collections
    _builder.add("action", VPackValue("function () { var c = "
      "require(\\\"internal\\\").db[\\\"" +_config.collection() + "\\\"]; var startcount = c.count(); for (var "
                                   "i = 0; i < 50; ++i) { if (startcount + i !== "
                                   "c.count()) { throw \\\"error, counters deviate!\\\"; } c.save({ "
                                   "}); } }"));
    _builder.close();
    return _builder.slice();
  }
};

  /*
struct TransactionDeadlockTest : public BenchmarkOperation {
  TransactionDeadlockTest(TestConfig const& config) : BenchmarkOperation(config) {}

  bool setUp(fuerte::Connection* client) override {
    _c1 = std::string(_config.collection() + "1");
    _c2 = std::string(_config.collection() + "2");

    return DeleteCollection(client, _c1) && DeleteCollection(client, _c2) &&
           CreateCollection(client, _c1, 2) &&
           CreateCollection(client, _c2, 2) &&
           CreateDocument(client, _c2, "{ \"_key\": \"sum\", \"count\": 0 }");
  }

  void tearDown() override {}

  std::string url(int const threadNumber, size_t const threadCounter,
                  size_t const globalCounter) override {
    return std::string("/_api/transaction");
  }

  arangodb::fuerte::RestVerb type(int const threadNumber, size_t const threadCounter,
                         size_t const globalCounter) override {
    return arangodb::fuerte::RestVerb::Post;
  }

  VPackSlice payload(int const threadNumber, size_t const threadCounter,
                    size_t const globalCounter) override {
    size_t const mod = globalCounter % 2;
    TRI_string_buffer_t* buffer;
    buffer = TRI_CreateSizedStringBuffer(256);

    TRI_AppendStringStringBuffer(buffer, "{ \"collections\": { ");
    TRI_AppendStringStringBuffer(buffer, "\"write\": [ \"");

    if (mod == 0) {
      TRI_AppendStringStringBuffer(buffer, _c1.c_str());
    } else {
      TRI_AppendStringStringBuffer(buffer, _c2.c_str());
    }

    TRI_AppendStringStringBuffer(buffer,
                                 "\" ] }, \"action\": \"function () { ");
    TRI_AppendStringStringBuffer(buffer,
                                 "var c = require(\\\"internal\\\").db[\\\"");
    if (mod == 0) {
      TRI_AppendStringStringBuffer(buffer, _c2.c_str());
    } else {
      TRI_AppendStringStringBuffer(buffer, _c1.c_str());
    }
    TRI_AppendStringStringBuffer(buffer, "\\\"]; c.any();");

    TRI_AppendStringStringBuffer(buffer, " }\" }");

    *length = TRI_LengthStringBuffer(buffer);
    *mustFree = true;
    char* ptr = TRI_StealStringBuffer(buffer);
    TRI_FreeStringBuffer(buffer);

    return (char const*)ptr;
  }

  std::string _c1;
  std::string _c2;
};

struct TransactionMultiTest : public BenchmarkOperation {
  TransactionMultiTest(TestConfig const& config) : BenchmarkOperation(config) {}

  bool setUp(fuerte::Connection* client) override {
    _c1 = std::string(_config.collection() + "1");
    _c2 = std::string(_config.collection() + "2");

    return DeleteCollection(client, _c1) && DeleteCollection(client, _c2) &&
           CreateCollection(client, _c1, 2) &&
           CreateCollection(client, _c2, 2) &&
           CreateDocument(client, _c2, "{ \"_key\": \"sum\", \"count\": 0 }");
  }

  void tearDown() override {}

  std::string url(int const threadNumber, size_t const threadCounter,
                  size_t const globalCounter) override {
    return std::string("/_api/transaction");
  }

  arangodb::fuerte::RestVerb type(int const threadNumber, size_t const threadCounter,
                         size_t const globalCounter) override {
    return arangodb::fuerte::RestVerb::Post;
  }

  VPackSlice payload(int const threadNumber, size_t const threadCounter,
                    size_t const globalCounter) override {
    size_t const mod = globalCounter % 2;
    TRI_string_buffer_t* buffer;
    buffer = TRI_CreateSizedStringBuffer(256);

    TRI_AppendStringStringBuffer(buffer, "{ \"collections\": { ");

    if (mod == 0) {
      TRI_AppendStringStringBuffer(buffer, "\"exclusive\": [ \"");
    } else {
      TRI_AppendStringStringBuffer(buffer, "\"read\": [ \"");
    }

    TRI_AppendStringStringBuffer(buffer, _c1.c_str());
    TRI_AppendStringStringBuffer(buffer, "\", \"");
    TRI_AppendStringStringBuffer(buffer, _c2.c_str());
    TRI_AppendStringStringBuffer(buffer,
                                 "\" ] }, \"action\": \"function () { ");
    TRI_AppendStringStringBuffer(buffer,
                                 "var c1 = require(\\\"internal\\\").db[\\\"");
    TRI_AppendStringStringBuffer(buffer, _c1.c_str());
    TRI_AppendStringStringBuffer(
        buffer, "\\\"]; var c2 = require(\\\"internal\\\").db[\\\"");
    TRI_AppendStringStringBuffer(buffer, _c2.c_str());
    TRI_AppendStringStringBuffer(buffer, "\\\"]; ");

    if (mod == 0) {
      TRI_AppendStringStringBuffer(buffer,
                                   "var n = Math.floor(Math.random() * 25) + "
                                   "1; c1.save({ count: n }); var d = "
                                   "c2.document(\\\"sum\\\"); c2.update(d, { "
                                   "count: d.count + n });");
    } else {
      TRI_AppendStringStringBuffer(buffer,
                                   "var r1 = 0; c1.toArray().forEach(function "
                                   "(d) { r1 += d.count }); var r2 = "
                                   "c2.document(\\\"sum\\\").count; if (r1 !== "
                                   "r2) { throw \\\"error, counters deviate!\\\"; }");
    }

    TRI_AppendStringStringBuffer(buffer, " }\" }");

    *length = TRI_LengthStringBuffer(buffer);
    *mustFree = true;
    char* ptr = TRI_StealStringBuffer(buffer);
    TRI_FreeStringBuffer(buffer);

    return (char const*)ptr;
  }

  std::string _c1;
  std::string _c2;
};

struct TransactionMultiCollectionTest : public BenchmarkOperation {
  TransactionMultiCollectionTest(TestConfig const& config) : BenchmarkOperation(config) {}

  bool setUp(fuerte::Connection* client) override {
    _c1 = std::string(_config.collection() + "1");
    _c2 = std::string(_config.collection() + "2");

    return DeleteCollection(client, _c1) && DeleteCollection(client, _c2) &&
           CreateCollection(client, _c1, 2) && CreateCollection(client, _c2, 2);
  }

  void tearDown() override {}

  std::string url(int const threadNumber, size_t const threadCounter,
                  size_t const globalCounter) override {
    return std::string("/_api/transaction");
  }

  arangodb::fuerte::RestVerb type(int const threadNumber, size_t const threadCounter,
                         size_t const globalCounter) override {
    return arangodb::fuerte::RestVerb::Post;
  }

  VPackSlice payload(int const threadNumber, size_t const threadCounter,
                    size_t const globalCounter) override {
    TRI_string_buffer_t* buffer;
    buffer = TRI_CreateSizedStringBuffer(256);

    TRI_AppendStringStringBuffer(buffer, "{ \"collections\": { ");

    TRI_AppendStringStringBuffer(buffer, "\"write\": [ \"");
    TRI_AppendStringStringBuffer(buffer, _c1.c_str());
    TRI_AppendStringStringBuffer(buffer, "\", \"");
    TRI_AppendStringStringBuffer(buffer, _c2.c_str());
    TRI_AppendStringStringBuffer(buffer,
                                 "\" ] }, \"action\": \"function () { ");

    TRI_AppendStringStringBuffer(buffer,
                                 "var c1 = require(\\\"internal\\\").db[\\\"");
    TRI_AppendStringStringBuffer(buffer, _c1.c_str());
    TRI_AppendStringStringBuffer(
        buffer, "\\\"]; var c2 = require(\\\"internal\\\").db[\\\"");
    TRI_AppendStringStringBuffer(buffer, _c2.c_str());
    TRI_AppendStringStringBuffer(buffer, "\\\"]; ");

    TRI_AppendStringStringBuffer(buffer, "var doc = {");
    uint64_t const n = _config.complexity();
    for (uint64_t i = 0; i < n; ++i) {
      if (i > 0) {
        TRI_AppendStringStringBuffer(buffer, ", ");
      }
      TRI_AppendStringStringBuffer(buffer, "value");
      TRI_AppendUInt64StringBuffer(buffer, i);
      TRI_AppendStringStringBuffer(buffer, ": ");
      TRI_AppendUInt64StringBuffer(buffer, i);
    }
    TRI_AppendStringStringBuffer(buffer, " }; ");

    TRI_AppendStringStringBuffer(buffer, "c1.save(doc); c2.save(doc); }\" }");

    *length = TRI_LengthStringBuffer(buffer);
    *mustFree = true;
    char* ptr = TRI_StealStringBuffer(buffer);
    TRI_FreeStringBuffer(buffer);

    return (char const*)ptr;
  }

  std::string _c1;
  std::string _c2;
};

struct StreamCursorTest : public BenchmarkOperation {
  StreamCursorTest(TestConfig const& config) : BenchmarkOperation(config) {}

  bool setUp(fuerte::Connection* client) override {
    return DeleteCollection(client, _config.collection()) &&
           CreateCollection(client, _config.collection(), 2);
  }

  void tearDown() override {}

  std::string url(int const threadNumber, size_t const threadCounter,
                  size_t const globalCounter) override {
    return std::string("/_api/cursor");
  }

  arangodb::fuerte::RestVerb type(int const threadNumber, size_t const threadCounter,
                         size_t const globalCounter) override {
    return arangodb::fuerte::RestVerb::Post;
  }

  VPackSlice payload(int const threadNumber, size_t const threadCounter,
                    size_t const globalCounter) override {
    
    TRI_string_buffer_t* buffer;
    buffer = TRI_CreateSizedStringBuffer(256);
    
    size_t const mod = globalCounter % 2;

    if (globalCounter == 0) {
      TRI_AppendStringStringBuffer(buffer,
                                  "{\"query\":\"FOR i IN 1..500 INSERT { _key: TO_STRING(i)");

      uint64_t const n = _config.complexity();
      for (uint64_t i = 1; i <= n; ++i) {
        TRI_AppendStringStringBuffer(buffer, ",\\\"value");
        TRI_AppendUInt64StringBuffer(buffer, i);
        TRI_AppendStringStringBuffer(buffer, "\\\":true");
      }

      TRI_AppendStringStringBuffer(buffer, " } INTO ");
      TRI_AppendStringStringBuffer(buffer, _config.collection().c_str());
      TRI_AppendStringStringBuffer(buffer, "\"}"); //OPTIONS { ignoreErrors: true }");
    } else if (mod == 0) {
      TRI_AppendStringStringBuffer(buffer,
                                  "{\"query\":\"UPDATE { _key: \\\"1\\\" } WITH { \\\"foo\\\":1");

      uint64_t const n = _config.complexity();
      for (uint64_t i = 1; i <= n; ++i) {
        TRI_AppendStringStringBuffer(buffer, ",\\\"value");
        TRI_AppendUInt64StringBuffer(buffer, i);
        TRI_AppendStringStringBuffer(buffer, "\\\":true");
      }

      TRI_AppendStringStringBuffer(buffer, " } INTO ");
      TRI_AppendStringStringBuffer(buffer, _config.collection().c_str());
      TRI_AppendStringStringBuffer(buffer, " OPTIONS { ignoreErrors: true }\"}");
    } else {
      TRI_AppendStringStringBuffer(buffer,
                                  "{\"query\":\"FOR doc IN ");
      TRI_AppendStringStringBuffer(buffer, _config.collection().c_str());
      TRI_AppendStringStringBuffer(buffer, " RETURN doc\",\"options\":{\"stream\":true}}");
    }

    *length = TRI_LengthStringBuffer(buffer);
    *mustFree = true;
    char* ptr = TRI_StealStringBuffer(buffer);
    TRI_FreeStringBuffer(buffer);

    return (char const*)ptr;
  }
};

struct AqlInsertTest : public BenchmarkOperation {
  AqlInsertTest(TestConfig const& config) : BenchmarkOperation(config) {}

  bool setUp(fuerte::Connection* client) override {
    return DeleteCollection(client, _config.collection()) &&
           CreateCollection(client, _config.collection(), 2);
  }

  void tearDown() override {}

  std::string url(int const threadNumber, size_t const threadCounter,
                  size_t const globalCounter) override {
    return std::string("/_api/cursor");
  }

  arangodb::fuerte::RestVerb type(int const threadNumber, size_t const threadCounter,
                         size_t const globalCounter) override {
    return arangodb::fuerte::RestVerb::Post;
  }

  VPackSlice payload(int const threadNumber, size_t const threadCounter,
                    size_t const globalCounter) override {
    TRI_string_buffer_t* buffer;
    buffer = TRI_CreateSizedStringBuffer(256);

    TRI_AppendStringStringBuffer(buffer,
                                 "{\"query\":\"INSERT { _key: \\\"test");
    TRI_AppendInt64StringBuffer(buffer, (int64_t)globalCounter);
    TRI_AppendStringStringBuffer(buffer, "\\\"");

    uint64_t const n = _config.complexity();
    for (uint64_t i = 1; i <= n; ++i) {
      TRI_AppendStringStringBuffer(buffer, ",\\\"value");
      TRI_AppendUInt64StringBuffer(buffer, i);
      TRI_AppendStringStringBuffer(buffer, "\\\":true");
    }

    TRI_AppendStringStringBuffer(buffer, " } INTO ");
    TRI_AppendStringStringBuffer(buffer, _config.collection().c_str());
    TRI_AppendStringStringBuffer(buffer, "\"}");

    *length = TRI_LengthStringBuffer(buffer);
    *mustFree = true;
    char* ptr = TRI_StealStringBuffer(buffer);
    TRI_FreeStringBuffer(buffer);

    return (char const*)ptr;
  }
};

struct AqlV8Test : public BenchmarkOperation {
  AqlV8Test(TestConfig const& config) : BenchmarkOperation(config) {}

  bool setUp(fuerte::Connection* client) override {
    return DeleteCollection(client, _config.collection()) &&
           CreateCollection(client, _config.collection(), 2);
  }

  void tearDown() override {}

  std::string url(int const threadNumber, size_t const threadCounter,
                  size_t const globalCounter) override {
    return std::string("/_api/cursor");
  }

  arangodb::fuerte::RestVerb type(int const threadNumber, size_t const threadCounter,
                         size_t const globalCounter) override {
    return arangodb::fuerte::RestVerb::Post;
  }

  VPackSlice payload(int const threadNumber, size_t const threadCounter,
                    size_t const globalCounter) override {
    TRI_string_buffer_t* buffer;
    buffer = TRI_CreateSizedStringBuffer(256);

    TRI_AppendStringStringBuffer(buffer,
                                 "{\"query\":\"INSERT { _key: \\\"test");
    TRI_AppendInt64StringBuffer(buffer, (int64_t)globalCounter);
    TRI_AppendStringStringBuffer(buffer, "\\\"");

    uint64_t const n = _config.complexity();
    for (uint64_t i = 1; i <= n; ++i) {
      TRI_AppendStringStringBuffer(buffer, ",\\\"value");
      TRI_AppendUInt64StringBuffer(buffer, i);
      TRI_AppendStringStringBuffer(buffer, "\\\":RAND(),\\\"test");
      TRI_AppendUInt64StringBuffer(buffer, i);
      TRI_AppendStringStringBuffer(buffer, "\\\":RANDOM_TOKEN(32)");
    }

    TRI_AppendStringStringBuffer(buffer, " } INTO ");
    TRI_AppendStringStringBuffer(buffer, _config.collection().c_str());
    TRI_AppendStringStringBuffer(buffer, "\"}");

    *length = TRI_LengthStringBuffer(buffer);
    *mustFree = true;
    char* ptr = TRI_StealStringBuffer(buffer);
    TRI_FreeStringBuffer(buffer);

    return (char const*)ptr;
  }
};*/

////////////////////////////////////////////////////////////////////////////////
/// @brief delete a collection
////////////////////////////////////////////////////////////////////////////////

static bool DeleteCollection(fuerte::Connection* conn,
                             std::string const& name) {
  
  auto req = fuerte::createRequest(arangodb::fuerte::RestVerb::Delete, "/_api/collection/" + name);
  auto res = conn->sendRequest(std::move(req)); // throws on error

  bool failed = true;
  int statusCode = res->statusCode();
  if (statusCode == 200 || statusCode == 201 || statusCode == 202 ||
      statusCode == 404) {
    failed = false;
  }

  return !failed;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief create a collection
////////////////////////////////////////////////////////////////////////////////

static bool CreateCollection(fuerte::Connection* conn, std::string const& name,
                             int const type) {
  auto req = fuerte::createRequest(arangodb::fuerte::RestVerb::Post, "/_api/collection/" + name);
  VPackBuffer<uint8_t> buffer;
  VPackBuilder builder(buffer);
  builder.openObject();
  builder.add("name", VPackValue(type));
  builder.add("type", VPackValue(name));
  //builder.add("replicationFactor", VPackValue(1));
  //builder.add("numberOfShards", VPackValue(1));
  builder.add("waitForSync", VPackValue(false));
  builder.close();
  req->addVPack(std::move(buffer));
  
  auto res = conn->sendRequest(std::move(req));
  
  bool failed = true;
  int statusCode = res->statusCode();
  if (statusCode == 200 || statusCode == 201 || statusCode == 202) {
    failed = false;
  }
  
  return !failed;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief create an index
////////////////////////////////////////////////////////////////////////////////

static bool CreateIndex(fuerte::Connection* conn, std::string const& name,
                        std::string const& type, std::vector<std::string> const& fields) {
  auto req = fuerte::createRequest(arangodb::fuerte::RestVerb::Post, "/_api/index?collection=" + name);
  VPackBuffer<uint8_t> buffer;
  VPackBuilder builder(buffer);
  builder.openObject();
  builder.add("type", VPackValue(type));
  builder.add("fields", VPackValue(VPackValueType::Array));
  for (std::string const& field : fields) {
    builder.add(VPackValue(field));
  }
  builder.close();
  builder.add("unique", VPackValue(false));
  builder.close();
  req->addVPack(std::move(buffer));
  
  auto res = conn->sendRequest(std::move(req));
  
  bool failed = true;
  int statusCode = res->statusCode();
  if (statusCode == 200 || statusCode == 201) {
    failed = false;
  }
  
  return !failed;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief create a document
////////////////////////////////////////////////////////////////////////////////

/*static bool CreateDocument(fuerte::Connection* client,
                           std::string const& collection,
                           std::string const& payload) {
  std::unordered_map<std::string, std::string> headerFields;
  SimpleHttpResult* result = nullptr;

  result = client->request(arangodb::fuerte::RestVerb::Post,
                           "/_api/document?collection=" + collection,
                           payload.c_str(), payload.size(), headerFields);

  bool failed = true;

  if (result != nullptr) {
    if (result->getHttpReturnCode() == 200 ||
        result->getHttpReturnCode() == 201 ||
        result->getHttpReturnCode() == 202) {
      failed = false;
    }

    delete result;
  }

  return !failed;
}*/

////////////////////////////////////////////////////////////////////////////////
/// @brief return the test case for a name
////////////////////////////////////////////////////////////////////////////////

static BenchmarkOperation* GetTestCase(std::string const& name,
                                       TestConfig const& config) {
  if (name == "version") {
    return new VersionTest(config);
  }
  if (name == "import-document") {
    return new DocumentImportTest(config);
  }
  if (name == "document") {
    return new DocumentCreationTest(config);
  }
  if (name == "collection") {
    return new CollectionCreationTest(config);
  }
  if (name == "hash") {
    return new HashTest(config);
  }
  if (name == "skiplist") {
    return new SkiplistTest(config);
  }
  if (name == "edge") {
    return new EdgeCrudTest(config);
  }
  if (name == "shapes") {
    return new ShapesTest(config);
  }
  if (name == "shapes-append") {
    return new ShapesAppendTest(config);
  }
  /*if (name == "random-shapes") {
    return new RandomShapesTest(config);
  }*/
  if (name == "crud") {
    return new DocumentCrudTest(config);
  }
  if (name == "crud-append") {
    return new DocumentCrudAppendTest(config);
  }
  if (name == "crud-write-read") {
    return new DocumentCrudWriteReadTest(config);
  }
  if (name == "aqltrx") {
    return new TransactionAqlTest(config);
  }
  if (name == "counttrx") {
    return new TransactionCountTest(config);
  }
  /*if (name == "multitrx") {
    return new TransactionMultiTest(config);
  }
  if (name == "deadlocktrx") {
    return new TransactionDeadlockTest(config);
  }
  if (name == "multi-collection") {
    return new TransactionMultiCollectionTest(config);
  }
  if (name == "aqlinsert") {
    return new AqlInsertTest(config);
  }
  if (name == "aqlv8") {
    return new AqlV8Test(config);
  }
  if (name == "stream-cursor") {
    return new StreamCursorTest(config);
  }*/

  return nullptr;
}
}

#endif
