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
#pragma once
#ifndef ARANGO_CXX_DRIVER_CONNECTION_POOL
#define ARANGO_CXX_DRIVER_CONNECTION_POOL

#include <mutex>
#include <unordered_map>
#include "connection.h"

namespace arangodb { namespace fuerte { inline namespace v1 {
/// @brief connection pooling
class ConnectionPool {
  // Send a request to the server and return immediately.
  // When a response is received or an error occurs, the corresponding callback
  // is called.
  MessageID sendRequest(std::unique_ptr<Request> r, RequestCallback cb);

  /// @brief wait for request to finish
  void waitForResponse(MessageID) const;

 private:
  size_t const _maxOpenConnections;
  size_t const _maxConnectionsPerHost;
  std::mutex _mutex;
  std::unordered_map<std::string, std::vector<std::shared_ptr<Connection>>>
      _conntections;
};
}}};  // namespace arangodb::fuerte::v1

#endif