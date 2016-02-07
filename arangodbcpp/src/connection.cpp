/*
 * Copyright 2016 <copyright holder> <email>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#include <cstring>
#include <velocypack/Slice.h>
#include <velocypack/Parser.h>
#include <velocypack/Builder.h>
#include <velocypack/Sink.h>
#include <velocypack/Dumper.h>

#include <curlpp/Infos.hpp>

#include "arangodbcpp/connection.h"

namespace arangodb
{
	
namespace dbinterface
{

std::string Connection::toJson(VPack &v,bool bSort)
{
	using arangodb::velocypack::Slice;
	using arangodb::velocypack::Dumper;
	using arangodb::velocypack::StringSink;
	using arangodb::velocypack::Options;
	Slice slice{ v->data() };
	std::string tmp;
	StringSink sink(&tmp);
	Options opts;
	opts.sortAttributeNames = bSort;
	Dumper::dump(slice,&sink,&opts);
	return tmp;
}


void Connection::setBuffer()
{
	setBuffer(this,&Connection::WriteMemoryCallback);
}

void Connection::setJsonContent()
{
	HeaderList headers;
	headers.push_back("Content-Type: application/json");
	setHeaderOpts(headers);
}

/**

	Clears the buffer that holds received data and
	any error messages

	Configures whether the next operation will be done
	synchronously or asyncronously

*/
void Connection::setReady(bool bAsync)
{
	m_buf.clear();
	m_flgs = 0;
	if (bAsync)
	{
		m_flgs = F_Multi;
		m_async.add(&m_request);
	}
}

void Connection::reset()
{
	if (m_flgs & F_Multi)
	{
		m_async.remove(&m_request);
	}
	m_request.reset();
	m_flgs = 0;
}

/**

	Flags an error has occured and transfers the error message
	to the default write buffer

*/
void Connection::errFound(const std::string &inp,bool bRun)
{
	if (m_flgs & F_Multi)
	{
		m_async.remove(&m_request);
	}
	m_buf.clear();
	m_buf.insert(m_buf.begin(),inp.cbegin(),inp.cend());
	m_flgs = bRun?F_RunError:F_LogicError;
}

void Connection::doRun()
{
	if (m_flgs & F_Multi)
	{
		asyncDo();
	}
	else
	{
		syncDo();
	}
}

/**

	Synchronous operation which will complete before returning

*/
void Connection::syncDo()
{
	try
	{
		m_request.perform();
		m_flgs = F_Done;
	}
	catch ( curlpp::LogicError & e ) 
	{
		errFound(e.what(),false);
		return;
  }

  catch ( curlpp::LibcurlRuntimeError &e)
	{
		errFound(e.what());
		return;
	}
  
  catch ( curlpp::RuntimeError & e ) 
	{
		errFound(e.what());
		return;
  }
}

/**

	Asynchronous operation which may need to be run multiple times
	before completing

*/
void Connection::asyncDo()
{
	try
	{
		{
			int nLeft;
			m_flgs |= F_Running;
			if (!m_async.perform(&nLeft))
			{
				errFound("Asynchronous operation failed");
				return;
			}
			if (!nLeft)
			{
				if (m_buf.empty())
				{
					errFound("Asynchronous operation failed");
					return;
				}
				m_async.remove(&m_request);
				m_flgs = F_Done;
				return;
			}
		}
		{
			struct timeval timeout;
			int rc; /* select() return code */

			fd_set fdread;
			fd_set fdwrite;
			fd_set fdexcep;
			int maxfd;

			FD_ZERO ( &fdread );
			FD_ZERO ( &fdwrite );
			FD_ZERO ( &fdexcep );

			/* set a suitable timeout to play around with */
			timeout.tv_sec = 1;
			timeout.tv_usec = 0;

			
			/* get file descriptors from the transfers */
			m_async.fdset ( &fdread, &fdwrite, &fdexcep, &maxfd );

			rc = select ( maxfd + 1, &fdread, &fdwrite, &fdexcep, &timeout );
			if (rc == -1)
			{
				m_async.remove(&m_request);
				errFound("Asynchronous select error");
				return;
			}
		}
	}
	catch ( curlpp::LogicError & e ) 
	{
		errFound(e.what(),false);
		return;
  }
  
  catch ( curlpp::LibcurlRuntimeError &e)
	{
		errFound(e.what());
		return;
	}
  
  catch ( curlpp::RuntimeError & e ) 
	{
		errFound(e.what());
		return;
  }
}

void Connection::setPostField(const std::string &inp)
{
	setOpt(curlpp::options::PostFields(inp));
	setOpt(curlpp::options::PostFieldSize(inp.length()));
}

/**

	Sets the curlpp callback function that receives the data returned
	from the operation performed

*/
void Connection::setBuffer(size_t (*f)(char *p,size_t sz,size_t m) )
{
	curlpp::types::WriteFunctionFunctor fnc(f);
	setOpt(curlpp::options::WriteFunction(fnc));
}

/**

	Curlpp callback function that receives the data returned
	from the operation performed into the default write buffer

*/
size_t Connection::WriteMemoryCallback( char *ptr, size_t size, size_t nmemb )
{
	size_t realsize = size * nmemb;
	
	if (realsize != 0)
	{
		size_t offset = m_buf.size();
		m_buf.resize(offset + realsize);
		memcpy(&m_buf[offset],ptr,realsize);
	}

	return realsize;
}

/**

	Converts the contents of the default write buffer to a string
	
	This should either be JSon or an error message

*/ 
const std::string Connection::getBufString() const
{
	std::string tmp;
	tmp.insert(tmp.begin(),m_buf.cbegin(),m_buf.cend());
	return tmp;
}

/**

	Converts JSon held in the default write buffer
	to a shared velocypack buffer

*/
Connection::VPack Connection::fromJSon(bool bSorted) const
{
	if (!m_buf.empty())
	{
		using arangodb::velocypack::Builder;
		using arangodb::velocypack::Parser;
		using arangodb::velocypack::Options;
		Options options;
		options.sortAttributeNames = bSorted;
		Parser parser { &options };
		parser.parse(&m_buf[0],m_buf.size());
		std::shared_ptr<Builder> vp { parser.steal() };
		return vp->buffer();
	}
	return VPack { new VBuffer() };
}

Connection::Connection()
{
}

Connection::~Connection()
{
}

}
}