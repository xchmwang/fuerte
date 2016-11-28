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

#include "fuerte/ConnectionBuilder.h"

#include <boost/algorithm/string.hpp>

#include "fuerte/Connection.h"

namespace arangodb {
namespace fuerte {
inline namespace v1 {
std::shared_ptr<Connection> ConnectionBuilder::connect(
    std::shared_ptr<LoopProvider> loop) {
  std::shared_ptr<Connection> connection(new Connection(_conf));
  connection->build(std::move(loop));
  return connection;
}

ConnectionBuilder& ConnectionBuilder::setHost(std::string const& str) {
  std::vector<std::string> strings;
  boost::split(strings, str, boost::is_any_of(":"));

  // get protocol
  std::string const& proto = strings[0];

  if (proto == "vst" || proto == "vst+tcp") {
    _conf._connType = TransportType::Vst;
    _conf._ssl = false;
  } else if (proto == "vsts" || proto == "vst+ssl") {
    _conf._connType = TransportType::Vst;
    _conf._ssl = true;
  } else if (proto == "http" || proto == "http+tcp") {
    _conf._connType = TransportType::Http;
    _conf._ssl = false;
  } else if (proto == "https" || proto == "http+ssl") {
    _conf._connType = TransportType::Http;
    _conf._ssl = true;
  } else {
    throw std::invalid_argument("invalid protocol '" + proto + "'");
  }

  std::string& url = strings[1];

  if (url.size() < 2) {
    throw std::invalid_argument("invalid connection string '" + str + "'");
  }

  _conf._host = url.erase(0, 2);

  std::string& port = strings[2];

  while (port.back() == '/') {
    port.pop_back();
  }

  size_t pos;
  _conf._port = std::stoi(port, &pos);

  if (pos != port.size()) {
    throw std::invalid_argument("invalid port '" + str + "'");
  }

  return *this;
}
}
}
}
