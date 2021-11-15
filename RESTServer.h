#ifndef RESTSERVER_H
#define RESTSERVER_H

#include "MdlUriInfo.h"
#include "MdlServerInfo.h"
#include "MdlResponseInfo.h"
#include "MdlErrorInfo.h"

#include "RESTServerContext.h"
#include "Listener.h"

#include "ServerInfoContext.h"

typedef std::lock_guard<std::mutex> TLock;

class REST_Server
{
public:

	using SSLCtxInitHandler = std::function<bool(boost::asio::ssl::context&)>;

	REST_Server(unsigned int threads = 1);
	virtual ~REST_Server();

	static std::shared_ptr<REST_Server> make_server(unsigned int threadCount, std::string address, unsigned short port);

	// server control methods
	virtual bool startServer_HTTP();
	virtual bool startServer_HTTPS(SSLCtxInitHandler fHandler);
	virtual bool reset();
	virtual void shutdown();

	unsigned int getServerThreadCount()const { return m_threadCount; }
	void setServerThreadCount(unsigned int count) { m_threadCount = count; }

	std::string getLastServerError() const   { return m_lastServerError; }
	void getLastServerError(std::string msg) { m_lastServerError = msg;  }

	void registerContext(TRESTCtxPtr pContext);
	void unregisterContext(TRESTCtxPtr pContext);
	RESTCtxList getContexts() const { return m_serverContexts; }

	void setServerDomain(std::string address, unsigned short port);

protected:

	ListenerPtr  m_pListener;
	SSLCtxPtr 	 m_pSSLContext;
	RESTCtxList  m_serverContexts;
	IOCtxPtr     m_pIOContext;
	unsigned int m_threadCount;
	std::vector<std::thread> m_ioCtxThreads;

	mutable std::mutex m_mutex;

	std::string m_lastServerError;
	ServerInfoPtr m_pInfo;
};
typedef REST_Server Server;
typedef std::shared_ptr<Server> RESTServerPtr;


#endif // !RESTSERVER_H