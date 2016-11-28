////////////////////////////////////////////////////////////////////////////////
/// DISCLAIMER
///
/// Copyright 2016 ArangoDB GmbH, Cologne, Germany
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
/// @author Frank Celler
/// @author Jan Uhde
/// @author John Bufton
////////////////////////////////////////////////////////////////////////////////

#ifndef ARANGO_CXX_DRIVER_TYPES_H
#define ARANGO_CXX_DRIVER_TYPES_H 1

#include <cassert>
#include <map>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include <velocypack/Buffer.h>
#include <velocypack/Builder.h>
#include <velocypack/Slice.h>

namespace arangodb {
namespace fuerte {
inline namespace v1 {
class Response;
class Request;

using Error = long;
using NetBuffer = std::string;
using OnErrorCallback = std::function<void(Error, std::unique_ptr<Request>,
                                           std::unique_ptr<Response>)>;
using OnSuccessCallback =
    std::function<void(std::unique_ptr<Request>, std::unique_ptr<Response>)>;
using Ticket = uint64_t;
using VBuffer = arangodb::velocypack::Buffer<uint8_t>;
using VBuilder = arangodb::velocypack::Builder;
using VSlice = arangodb::velocypack::Slice;
using VValue = arangodb::velocypack::Value;
using mapss = std::unordered_map<std::string, std::string>;

// -----------------------------------------------------------------------------
// --SECTION--                                         enum class ErrorCondition
// -----------------------------------------------------------------------------

enum class ErrorCondition : Error {
  CouldNotConnect = 1000,
  Timeout = 1001,
  CurlError = 1002
};

// -----------------------------------------------------------------------------
// --SECTION--                                                   class Callbacks
// -----------------------------------------------------------------------------

class Callbacks {
 public:
  Callbacks() {}

  Callbacks(OnSuccessCallback onSuccess, OnErrorCallback onError)
      : _onSuccess(onSuccess), _onError(onError) {}

 public:
  OnSuccessCallback _onSuccess;
  OnErrorCallback _onError;
};

// -----------------------------------------------------------------------------
// --SECTION--                                               enum class RestVerb
// -----------------------------------------------------------------------------

enum class RestVerb {
  Illegal = -1,
  Delete = 0,
  Get = 1,
  Post = 2,
  Put = 3,
  Head = 4,
  Patch = 5,
  Options = 6
};

inline RestVerb to_RestVerb(std::string const& val) {
  auto p = val.c_str();

  if (strcasecmp(p, "delete") == 0) {
    return RestVerb::Delete;
  }

  if (strcasecmp(p, "get") == 0) {
    return RestVerb::Get;
  }

  if (strcasecmp(p, "post") == 0) {
    return RestVerb::Post;
  }

  if (strcasecmp(p, "put") == 0) {
    return RestVerb::Put;
  }

  if (strcasecmp(p, "head") == 0) {
    return RestVerb::Head;
  }

  if (strcasecmp(p, "patch") == 0) {
    return RestVerb::Patch;
  }

  if (strcasecmp(p, "options") == 0) {
    return RestVerb::Options;
  }

  return RestVerb::Illegal;
}

inline std::string to_string(RestVerb type) {
  switch (type) {
    case RestVerb::Illegal:
      return "illegal";

    case RestVerb::Delete:
      return "delete";

    case RestVerb::Get:
      return "get";

    case RestVerb::Post:
      return "post";

    case RestVerb::Put:
      return "put";

    case RestVerb::Head:
      return "head";

    case RestVerb::Patch:
      return "patch";

    case RestVerb::Options:
      return "options";
  }

  return "undefined";
}

// -----------------------------------------------------------------------------
// --SECTION--                                                       MessageType
// -----------------------------------------------------------------------------

enum class MessageType {
  Undefined = 0,
  Request = 1,
  Response = 2,
  Authentication = 1000
};

inline std::string to_string(MessageType type) {
  switch (type) {
    case MessageType::Undefined:
      return "undefined";

    case MessageType::Request:
      return "request";

    case MessageType::Response:
      return "response";

    case MessageType::Authentication:
      return "authentication";
  }

  return "undefined";
}

// -----------------------------------------------------------------------------
// --SECTION--                                                     TransportType
// -----------------------------------------------------------------------------

enum class TransportType { Undefined = 0, Http = 1, Vst = 2 };

inline std::string to_string(TransportType type) {
  switch (type) {
    case TransportType::Undefined:
      return "undefined";

    case TransportType::Http:
      return "http";

    case TransportType::Vst:
      return "vst";
  }

  return "undefined";
}

// -----------------------------------------------------------------------------
// --SECTION--                                                     TransportType
// -----------------------------------------------------------------------------

enum class ContentType { Unset, Custom, VPack, Dump, Json, Html, Text };

inline std::string to_string(ContentType type) {
  switch (type) {
    case ContentType::Unset:
      return "unset";

    case ContentType::Custom:
      return "custom";

    case ContentType::VPack:
      return "vpack";

    case ContentType::Dump:
      return "dump";

    case ContentType::Json:
      return "json";

    case ContentType::Html:
      return "html";

    case ContentType::Text:
      return "text";
  }

  return "undefined";
}

// -----------------------------------------------------------------------------
// --SECTION--                                           ConnectionConfiguration
// -----------------------------------------------------------------------------

namespace detail {
struct ConnectionConfiguration {
  ConnectionConfiguration() : _host("localhost"), _user("root") {}

  TransportType _connType = TransportType::Vst;
  bool _ssl = true;
  uint64_t _port = 8529;
  uint64_t _maxChunkSize = 5000;  // in bytes

  std::string _host;
  std::string _user;
  std::string _password;
};
}
}
}
}

#endif
