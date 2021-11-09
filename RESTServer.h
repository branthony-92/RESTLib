#ifndef RESTSERVER_H
#define RESTSERVER_H

#include "MdlUriInfo.h"
#include "MdlServerInfo.h"
#include "MdlResponseInfo.h"
#include "MdlErrorInfo.h"

#include "RESTEndpoint.h"
#include "RESTServerContext.h"
#include "Listener.h"

#include "ServerInfoContext.h"

typedef std::lock_guard<std::mutex> TLock;

/*
	This is the actual server class that will create and own the listener
*/
class Server
{
public:
	using SSLCtxInitHandler = std::function<bool(boost::asio::ssl::context&)>;
	using TEndpointMap = std::map<std::string, ReqHandlerPtr>;
	using TEndpointMapData = std::pair<std::string, ReqHandlerPtr>;

	Server(unsigned int threads = 1);
	virtual ~Server();

	virtual bool startServer(std::string address, unsigned short port, SessionPtr pSessionPrototype, const SSLCtxInitHandler& sslInitHandler);
	virtual bool reset();
	void shutdown();

	unsigned int getServerThreadCount()const { return m_threadCount; }
	void setServerThreadCount(unsigned int count) { m_threadCount = count; }

	std::string getLastServerError() const   { return m_lastServerError; }
	void getLastServerError(std::string msg) { m_lastServerError = msg;  }

	void registerContext(RESTCtxPtr pContext);
	void unregisterContext(RESTCtxPtr pContext);
	RESTCtxList getContexts() const { return m_serverContexts; }

protected:
	ListenerPtr  m_pListener;

	RESTCtxList  m_serverContexts;
	IOCtxPtr     m_pIOContext;
	SSLCtxPtr    m_pSSLContext;
	unsigned int m_threadCount;
	std::vector<std::thread> m_ioCtxThreads;
	mutable std::mutex m_mutex;

	std::string m_lastServerError;
	ServerInfoPtr m_pInfo;
};
typedef std::shared_ptr<Server> ServerPtr;

#endif // !RESTSERVER_H