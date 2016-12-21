#include <fuerte/Request.h>

namespace arangodb {
namespace fuerte {
inline namespace v1 {
// helper
static bool specialHeader(Request& request, std::string const&,
                          std::string const&) {
  // static std::regex -- test
  if (/* maching condition*/ false) {
    // do special stuff on
    // request->bla
    return true;
  }
  return false;
}

// set value in the header
void setHeaderValue(Request request, std::string const& key,
                    std::string const& val) {
  if (!specialHeader(request, key, val)) {
    request._header._meta.emplace(key, val);
  }
};

// external interface
Request createAuthRequest(std::string const& user,
                          std::string const& password) {
#if 0
  Request request;
  request._header._type = MessageType::Authentication;
  request._header._user = user;
  request._header._password = password;
  return request;
#endif
}

Request createRequest(RestVerb verb, std::string const& database,
                      std::string const& path, std::string const& user,
                      std::string const& password, mapss parameter,
                      mapss meta) {
#if 0
  // version must be set by protocol
  Request request;
  request._header._version = 0;
  request._header._type = MessageType::Request;
  request._header._responseCode = 0;
  request._header._database = database;
  request._header._requestType = verb;
  request._header._requestPath = path;
  request._header._requestPath = path;
  request._header._parameter = parameter;
  request._header._meta = meta;
  return request;
#endif
}

Request createResponse(unsigned code) {
#if 0
  Request request;
  // version must be set by protocol
  request._header._type = MessageType::Response;
  request._header._responseCode = code;
  return request;
#endif
}

#if 0
NetBuffer toNetworkVst(Request const&) { return "implement me"; }

NetBuffer toNetworkHttp(Request const&) { return "implement me"; }

Request fromBufferVst(uint8_t const* begin, std::size_t length) {
  auto num_slice = vst::validateAndCount(begin, begin + length);
  // work work
  return Request{};
}

boost::optional<Request> fromNetworkVst(NetBuffer const& buffer,
                                        vst::ReadBufferInfo& info,
                                        vst::MessageMap& messageMap) {
  auto buff_begin = reinterpret_cast<uint8_t const*>(buffer.data());
  auto buff_end = buff_begin + buffer.size();
  auto chunkEndOffset = vst::isChunkComplete(buff_begin, buff_end, info);
  if (chunkEndOffset) {  // chunk complete yeahy
    auto header = vst::readVstHeader(buff_begin, info);
    auto vpack_begin = buff_begin + header.headerLength;
    if (header.isFirst &&
        header.chunk == 1) {  // single chunk (message complete)
      return fromBufferVst(vpack_begin, chunkEndOffset);
    } else {  // multichunk
      auto message_iter = messageMap.find(header.messageID);
      if (message_iter == messageMap.end()) {
        // add new message
        std::pair<typename vst::MessageMap::iterator, bool> emplace_result =
            messageMap.emplace(
                header.messageID,
                vst::IncompleteMessage(header.messageLength, header.chunk));
        emplace_result.first->second.buffer.append(vpack_begin,
                                                   header.chunkLength);
      } else {
        // continue old message
        vst::IncompleteMessage& m = message_iter->second;
        m.buffer.append(vpack_begin, header.chunkLength);
        message_iter->second.buffer.append(vpack_begin, header.chunkLength);
        if (m.numberOfChunks == header.chunk) {
          return fromBufferVst(m.buffer.data(), m.buffer.byteSize());
        }
      }
    }
    // todo store in info how much of the buffer we have processed
    // so the calling job can update its buffer
  }
  return boost::none;
}

boost::optional<Request> fromNetworkHttp(NetBuffer const& buffer) {
  Request request;
  // parse body and convert to vpack
  return request;
}
#endif
}
}
}
