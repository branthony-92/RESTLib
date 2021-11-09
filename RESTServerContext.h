#ifndef RESTENDPOINTCONTEXT_H
#define RESTENDPOINTCONTEXT_H

#include "MdlServerInfo.h"
#include "MdlConnectionInfo.h"
#include "RESTConnection.h"

#include <atomic>

class HTTPRequestHandler;


class RESTServerContext
{
protected:

	std::string m_name;
	std::shared_ptr<ContextHandlersInfoBody>  m_pHandlerInfo;
	std::chrono::system_clock::time_point m_lastTransaction;

	std::atomic_bool      m_stopFlag;
	std::atomic_bool      m_resetFlag;
	TConnectionMgrPtr     m_pConnectionMgr;

	unsigned int m_transactionID;

	std::map<std::string, std::shared_ptr<HTTPRequestHandler>> m_requestHandlers;

public:
	RESTServerContext(std::string name);

	virtual ~RESTServerContext() {}

	// endpoint management
	void registerHandler(std::string path, std::shared_ptr<HTTPRequestHandler> pEndpoint);
	std::shared_ptr<HTTPRequestHandler> retrieveHandler(std::string name);

	bool hasHandlers() const;

	bool addHandlerName(std::string name);
	std::set<std::string> getHandlerNames() const { return m_pHandlerInfo->getHandlerNames(); }
	HandlersInfoBodyPtr getHandlerInfo() const { return m_pHandlerInfo; }
	
	virtual  std::string findResource(std::string target);

	// connection management
	std::shared_ptr<ConnectionInfoBody> handleConnectionRequest(TokenInfoBody::Lifetime type, unsigned int timeout);
	std::shared_ptr<ConnectionInfoBody> handleConnectionRefreshRequest(unsigned int id);
	std::shared_ptr<ConnectionInfoBody> getConnectionInfo(unsigned int id);

	bool checkConnection(unsigned int id, std::string token);

	std::string getName() const { return m_name; }

	void stop()  { m_stopFlag = true; }
	void reset() { m_resetFlag = true; }

	virtual void onServerStart()    { /* do nothing by default */}
	virtual void onServerReset()    { /* do nothing by default */}
	virtual void onServerShutdown() { /* do nothing by default */}

	void ping();

	unsigned int getNextransactionID();
};
typedef std::shared_ptr<RESTServerContext> TRESTCtxPtr;

#endif // !RESTENDPOINTCONTEXT_H
