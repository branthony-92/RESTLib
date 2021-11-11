#include "StdAfx.h"
#include "RESTServerContext.h"
#include "Listener.h"

RESTServerContext::RESTServerContext(std::string name)
	: m_name(name)
	, m_lastTransaction()
	, m_transactionID(0u)
	, m_stopFlag(false)
	, m_resetFlag(false)
	, m_pHandlerInfo(nullptr)
	, m_HTTPRequestCallbacks_GET()
	, m_HTTPRequestCallbacks_PUT()
	, m_HTTPRequestCallbacks_POST()
	, m_HTTPRequestCallbacks_DELETE()
	, m_HTTPRequestCallbacks_HEAD()
	, m_serverEventCallbacks()
	, m_pConnectionMgr(std::make_unique<ConnectionManager>())
{
	//replace spaces with underscores for the json body name
	for (char& c : name)
	{
		if (c == ' ') c = '_';
	}
	m_pHandlerInfo = std::make_shared<ContextHandlersInfoBody>(name);
}



std::string RESTServerContext::findResource(std::string target)
{
	return "";
}

std::shared_ptr<ConnectionInfoBody> RESTServerContext::handleConnectionRequest(TokenInfoBody::Lifetime type, unsigned int timeout)
{
	auto pConnection = m_pConnectionMgr->openNewConnection(type, timeout);

	if (!pConnection)
	{
		return nullptr;
	}
	auto pInfo = std::make_shared<ConnectionInfoBody>();

	// The client will get these values in return  and will need them 
	// for all future requests
	pInfo->setConnectionID(pConnection->getID());
	pInfo->setToken(pConnection->getToken());

	return pInfo;
}

std::shared_ptr<ConnectionInfoBody> RESTServerContext::handleConnectionRefreshRequest(unsigned int id)
{
	auto pToken = m_pConnectionMgr->refreshConnection(id);

	if (!pToken) return nullptr;

	auto pInfo = std::make_shared<ConnectionInfoBody>();
	pInfo->setConnectionID(id);
	pInfo->setToken(pToken);

	return pInfo;
}

std::shared_ptr<ConnectionInfoBody> RESTServerContext::getConnectionInfo(unsigned int id)
{
	auto pConnection = m_pConnectionMgr->getConnection(id);

	if (!pConnection)
	{
		return nullptr;
	}
	auto pToken = pConnection->getToken();
	if (!pToken) return nullptr;

	auto pInfo = std::make_shared<ConnectionInfoBody>();
	pInfo->setConnectionID(id);
	pInfo->setToken(pToken);

	return pInfo;
}

bool RESTServerContext::checkConnection(unsigned int id, std::string token)
{
	auto status = m_pConnectionMgr->validateConnection(id, token);
	auto valid = false;
	switch (status)
	{
	case ConnectionManager::ConnectionStatus::Open:
		// conneciton is fine
		valid = true;
		break;
	case ConnectionManager::ConnectionStatus::Uninitialized:
	case ConnectionManager::ConnectionStatus::StaleToken:
	case ConnectionManager::ConnectionStatus::BadToken:
	case ConnectionManager::ConnectionStatus::Closed:
	case ConnectionManager::ConnectionStatus::Invalid:
	default:
		break;
	}
	return valid;
}

void RESTServerContext::ping()
{
	m_lastTransaction = std::chrono::system_clock::now();
}

unsigned int RESTServerContext::getNextransactionID()
{
	return m_transactionID++;
}

// endpoint management
bool RESTServerContext::hasHandlers() const
{
	if (!m_pHandlerInfo) return false;
	return !m_pHandlerInfo->getHandlerNames().empty();
}

bool RESTServerContext::addHandlerName(std::string name)
{
	if (!m_pHandlerInfo) return false;
	return m_pHandlerInfo->add(name);
}

void RESTServerContext::registerCallback_GET(std::string path, RequestCallback callback)
{
	m_HTTPRequestCallbacks_GET.insert_or_assign(path, callback);
	addHandlerName(path);
}
void RESTServerContext::registerCallback_PUT(std::string path, RequestCallback callback)
{
	m_HTTPRequestCallbacks_PUT.insert_or_assign(path, callback);
	addHandlerName(path);
}
void RESTServerContext::registerCallback_POST(std::string path, RequestCallback callback)
{
	m_HTTPRequestCallbacks_POST.insert_or_assign(path, callback);
	addHandlerName(path);
}
void RESTServerContext::registerCallback_DELETE(std::string path, RequestCallback callback)
{
	m_HTTPRequestCallbacks_DELETE.insert_or_assign(path, callback);
	addHandlerName(path);
}
void RESTServerContext::registerCallback_HEAD(std::string path, RequestCallback callback)
{
	m_HTTPRequestCallbacks_HEAD.insert_or_assign(path, callback);
	addHandlerName(path);
}

RequestCallback* RESTServerContext::retrieveCallback_GET(std::string name)
{
	auto callback = m_HTTPRequestCallbacks_GET.find(name);
	if (callback == m_HTTPRequestCallbacks_GET.end()) return nullptr;
	return &callback->second;
}  
RequestCallback* RESTServerContext::retrieveCallback_PUT(std::string name)
{
	auto callback = m_HTTPRequestCallbacks_PUT.find(name);
	if (callback == m_HTTPRequestCallbacks_PUT.end()) return nullptr;
	return &callback->second;
}    
RequestCallback* RESTServerContext::retrieveCallback_POST(std::string name)
{
	auto callback = m_HTTPRequestCallbacks_POST.find(name);
	if (callback == m_HTTPRequestCallbacks_POST.end()) return nullptr;
	return &callback->second;
}   
RequestCallback* RESTServerContext::retrieveCallback_DELETE(std::string name)
{
	auto callback = m_HTTPRequestCallbacks_DELETE.find(name);
	if (callback == m_HTTPRequestCallbacks_DELETE.end()) return nullptr;
	return &callback->second;
}  
RequestCallback* RESTServerContext::retrieveCallback_HEAD(std::string name)
{
	auto callback = m_HTTPRequestCallbacks_HEAD.find(name);
	if (callback == m_HTTPRequestCallbacks_HEAD.end()) return nullptr;
	return &callback->second;
}

// server callback management
void RESTServerContext::registerServerEventCallback(ServerEnventID callbackID, EventCallback callback)
{
	m_serverEventCallbacks.insert_or_assign(callbackID, callback);
}

void RESTServerContext::onServerStart()
{
	auto callback = m_serverEventCallbacks.find(ServerEnventCallbackID::onServerStart);
	if (callback == m_serverEventCallbacks.end()) return;
	(Callback->second)();
}
void RESTServerContext::onServerReset()
{
	auto callback = m_serverEventCallbacks.find(ServerEnventCallbackID::onServerReset);
	if (callback == m_serverEventCallbacks.end()) return;
	(Callback->second)();
}   
void RESTServerContext::onServerShutdown()
{
	auto callback = m_serverEventCallbacks.find(ServerEnventCallbackID::onServerStop);
	if (callback == m_serverEventCallbacks.end()) return;
	(Callback->second)();
}