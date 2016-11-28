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

#ifndef ARANGO_CXX_DRIVER_SERVER_H
#define ARANGO_CXX_DRIVER_SERVER_H 1

#ifdef MOVE_TO_SERVER
  class Database;
  std::shared_ptr<Database> database(std::string name);
  std::shared_ptr<Database> createDatabase(std::string name);
  bool deleteDatabase(std::string name);
#endif

#ifdef MOVE_TO_SERVER
// get or create?!
std::shared_ptr<Database> Connection::getDatabase(std::string name) {
  return std::shared_ptr<Database>(new Database(shared_from_this(), name));
}
std::shared_ptr<Database> Connection::createDatabase(std::string name) {
  return std::shared_ptr<Database>(new Database(shared_from_this(), name));
}
bool Connection::deleteDatabase(std::string name) { return false; }
#endif

#endif

