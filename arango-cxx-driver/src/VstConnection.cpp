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

#include "VstConnection.h"

#include <iostream>

namespace arangodb {
namespace fuerte {
inline namespace v1 {
namespace vst {
using namespace arangodb::fuerte::detail;
using Resolver = boost::asio::ip::tcp::resolver;

// -----------------------------------------------------------------------------
// --SECTION--                                      constructors and destructors
// -----------------------------------------------------------------------------

VstConnection::VstConnection(std::shared_ptr<boost::asio::io_service> service,
                             ConnectionConfiguration configuration)
    : _service(std::move(service)),
      _socket(*_service.get()),
      _configuration(configuration) {}

// -----------------------------------------------------------------------------
// --SECTION--                                                    public methods
// -----------------------------------------------------------------------------

void VstConnection::start() {
  Resolver resolver(*_service.get());
  Resolver::query query(_configuration._host,
                        std::to_string(_configuration._port));
  Resolver::iterator endpointIterator = resolver.resolve(query);

  auto self = shared_from_this();
  _connectHandler = [self, this,
                     endpointIterator](boost::system::error_code const& err) {
    handleConnected(err, endpointIterator);
  };

  _socket.async_connect(*endpointIterator, _connectHandler);
}

void VstConnection::sendRequest(std::unique_ptr<Request> request,
                                OnSuccessCallback onSuccess,
                                OnErrorCallback onError) {
#warning TODO
}

// -----------------------------------------------------------------------------
// --SECTION--                                                   private methods
// -----------------------------------------------------------------------------

void VstConnection::handleConnected(boost::system::error_code const& err,
                                    Resolver::iterator endpointIterator) {
  if (err) {
    _socket.close();

    if (++endpointIterator != Resolver::iterator()) {
      // The connection failed. Try the next endpoint in the list.
      _socket.async_connect(*endpointIterator, _connectHandler);
      return;
    }

#warning TODO    errorHandler(err);
    return;
  }

  // write header identifier
  "VST/1.1\r\n\r\n";
}

void VstConnection::authorizeConnection() {
#warning TODO  executeRequests();
}
}
}
}
}
