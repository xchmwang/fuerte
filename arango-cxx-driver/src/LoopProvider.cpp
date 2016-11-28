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

#include <fuerte/LoopProvider.h>

#include "HttpCommunicator.h"

namespace arangodb {
namespace fuerte {
inline namespace v1 {
std::shared_ptr<http::HttpCommunicator> LoopProvider::http() {
  _http = std::make_shared<http::HttpCommunicator>();
  return _http;
}

void LoopProvider::run() {
  while (true) {
    if (_http != nullptr) {
      int left = _http->workOnce();

      if (left == 0) {
        break;
      }

      _http->wait();
    }
  }
}

void LoopProvider::shutdown() {}
}
}
}
