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

#include <fuerte/Connection.h>

#include <fuerte/ConnectionBuilder.h>
#include <fuerte/LoopProvider.h>

#include "HttpConnection.h"
#include "VstConnection.h"

namespace arangodb {
namespace fuerte {
inline namespace v1 {
using namespace arangodb::fuerte::detail;

Connection::Connection(detail::ConnectionConfiguration conf)
    : _connection(nullptr), _configuration(conf) {}

void Connection::build(std::shared_ptr<LoopProvider> loop) {
  if (_connection == nullptr) {
    switch (_configuration._connType) {
      case TransportType::Http:
        _connection.reset(new http::HttpConnection(loop->http(), _configuration));
        break;

      case TransportType::Vst:
        _connection.reset(new vst::VstConnection(loop->vst(), _configuration));
        break;

      default:
        throw std::invalid_argument("unknown connection type '" +
                                    to_string(_configuration._connType) +
                                    "'");
    }

    _connection->start();
  }
}
}
}
}
