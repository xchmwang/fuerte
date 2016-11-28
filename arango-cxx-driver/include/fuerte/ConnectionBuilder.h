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

#ifndef ARANGO_CXX_DRIVER_CONNECTION_BUILDER_H
#define ARANGO_CXX_DRIVER_CONNECTION_BUILDER_H 1

#include <fuerte/types.h>

// The connection Builder is a class that allows the easy configuration of
// connections. We decided to use the builder pattern because the connections
// have too many options to put them all in a single constructor. When you have
// passed all your options call connect() in order to receive a shared_ptr to a
// connection. Remember to use the "->" operator to access the connections
// members.

namespace arangodb {
namespace fuerte {
inline namespace v1 {
class Connection;
class LoopProvider;

class ConnectionBuilder {
 public:
  std::shared_ptr<Connection> connect(std::shared_ptr<LoopProvider>);

  ConnectionBuilder& setHost(std::string const&);

  ConnectionBuilder& setPort(uint64_t port) {
    _conf._port = port;
    return *this;
  }

  ConnectionBuilder& setUser(std::string const& u) {
    _conf._user = u;
    return *this;
  }

  ConnectionBuilder& setPassword(std::string const& p) {
    _conf._password = p;
    return *this;
  }

  ConnectionBuilder& setMaxChunkSize(std::size_t c) {
    _conf._maxChunkSize = c;
    return *this;
  }

 private:
  detail::ConnectionConfiguration _conf;
};
}
}
}

#endif
