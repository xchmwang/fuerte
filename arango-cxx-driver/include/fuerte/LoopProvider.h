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

#ifndef ARANGO_CXX_DRIVER_LOOP_PROVIDER_H
#define ARANGO_CXX_DRIVER_LOOP_PROVIDER_H 1

#include <fuerte/types.h>

namespace boost {
namespace asio {
class io_service;
}
}

namespace arangodb {
namespace fuerte {
inline namespace v1 {
namespace http {
class HttpCommunicator;
}

class LoopProvider {
 public:
  std::shared_ptr<http::HttpCommunicator> http();
  std::shared_ptr<boost::asio::io_service> vst();

  void run();
  void shutdown();

 private:
  std::shared_ptr<http::HttpCommunicator> _http;
  std::shared_ptr<boost::asio::io_service> _vst;
};
}
}
}

#endif
