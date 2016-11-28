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

#ifndef ARANGO_CXX_DRIVER_REQUEST_H
#define ARANGO_CXX_DRIVER_REQUEST_H 1

#include <fuerte/types.h>

#include <boost/optional.hpp>

namespace arangodb {
namespace fuerte {
inline namespace v1 {
struct RequestHeader {
  RestVerb _type = RestVerb::Get;
  std::string _path;
  std::string _database;
  mapss _parameter;
  mapss _meta;
};

class Request {
 public:
  Request(RequestHeader&& header = RequestHeader())
      : _header(std::move(header)) {}

 public:
  ContentType contentType() { return ContentType::Html; }

  mapss const& headers() { return _header._meta; }

  std::string const& body() {
    static std::string empty;
    return empty;
  };

#if 0
  void addPayload(VSlice const& slice) {
    VBuffer buffer;
    buffer.append(slice.start(), slice.byteSize());
    payload.push_back(std::move(buffer));
  }
#endif

 public:
  RequestHeader _header;
  std::vector<VBuffer> payload;
  uint64_t messageid;
};
}
}
}

#endif
