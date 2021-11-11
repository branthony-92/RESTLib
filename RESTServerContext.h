#ifndef RESTENDPOINTCONTEXT_H
#define RESTENDPOINTCONTEXT_H

#include "MdlServerInfo.h"
#include "MdlConnectionInfo.h"
#include "RESTConnection.h"
#include "RESTEndpoint.h"
#include <atomic>

enum class ServerEventCallbackID
{
	onServerStart,
	onServerReset,
	onServerStop,
};

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

	CallbackMap m_HTTPRequestCallbacks_GET;
	CallbackMap m_HTTPRequestCallbacks_PUT;
	CallbackMap m_HTTPRequestCallbacks_POST;
	CallbackMap m_HTTPRequestCallbacks_DELETE;
	CallbackMap m_HTTPRequestCallbacks_HEAD;
	
	using EventCallback = std::function<void()>;
	using EventCallbackMap = std::map<ServerEventCallbackID, EventCallback>;

	EventCallbackMap m_serverEventCallbacks;
public:

	RESTServerContext(std::string name);
	virtual ~RESTServerContext() {}

	static std::shared_ptr<RESTServerContext> make_context(std::string name = "Server_Context");

	// server callback management
	void registerServerEventCallback(ServerEventCallbackID callbackID, EventCallback callback);
	
	void onServerStart();
	void onServerReset();    
	void onServerShutdown();

	// endpoint callback management
	void registerCallback_GET(std::string path,    RequestCallback callback);
	void registerCallback_PUT(std::string path,    RequestCallback callback);
	void registerCallback_POST(std::string path,   RequestCallback callback);
	void registerCallback_DELETE(std::string path, RequestCallback callback);
	void registerCallback_HEAD(std::string path,   RequestCallback callback);

	RequestCallback* retrieveCallback_GET(std::string name);  
	RequestCallback* retrieveCallback_PUT(std::string name);    
	RequestCallback* retrieveCallback_POST(std::string name);     
	RequestCallback* retrieveCallback_DELETE(std::string name);  
	RequestCallback* retrieveCallback_HEAD(std::string name);   
	
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

	void ping();

	unsigned int getNextransactionID();
};
typedef RESTServerContext Context;
typedef std::shared_ptr<Context> TRESTCtxPtr;


#endif // !RESTENDPOINTCONTEXT_H
