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

#ifndef ARANGO_CXX_DRIVER_VST_CONNECTION_H
#define ARANGO_CXX_DRIVER_VST_CONNECTION_H 1

#include <fuerte/ConnectionInterface.h>

#include <boost/asio.hpp>

namespace arangodb {
namespace fuerte {
inline namespace v1 {
namespace vst {
class VstConnection : public ConnectionInterface,
                      public std::enable_shared_from_this<VstConnection> {
 public:
  VstConnection(std::shared_ptr<boost::asio::io_service>,
                detail::ConnectionConfiguration);

 public:
  void start() override;

  void sendRequest(std::unique_ptr<Request>, OnSuccessCallback,
                   OnErrorCallback) override;

 private:
  void handleConnected(const boost::system::error_code& err,
                       boost::asio::ip::tcp::resolver::iterator);
  void authorizeConnection();

 private:
  struct pendingRequests {
    std::unique_ptr<Request> _request;
    OnSuccessCallback _onSuccessCallback;
    OnErrorCallback _onErrorCallback;
  };

 private:
  enum class State { CONNECTED, UNCONNECTED } _state = State::UNCONNECTED;
  std::shared_ptr<boost::asio::io_service> _service;
  boost::asio::ip::tcp::socket _socket;
  detail::ConnectionConfiguration _configuration;
  std::function<void(boost::system::error_code const&)> _connectHandler;
};
}
}
}
}

#endif
