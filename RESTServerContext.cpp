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
	, m_callbacks_GET()
	, m_callbacks_PUT()
	, m_callbacks_POST()
	, m_callbacks_UPDATE()
	, m_callbacks_DELETE()
	, m_callbacks_HEAD()
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
	m_callbacks_GET.insert_or_assign(path, callback);
	addHandlerName(path);
}
void RESTServerContext::registerCallback_PUT(std::string path, RequestCallback callback)
{
	m_callbacks_PUT.insert_or_assign(path, callback);
	addHandlerName(path);
}
void RESTServerContext::registerCallback_POST(std::string path, RequestCallback callback)
{
	m_callbacks_POST.insert_or_assign(path, callback);
	addHandlerName(path);
}
void RESTServerContext::registerCallback_UPDATE(std::string path, RequestCallback callback)
{
	m_callbacks_UPDATE.insert_or_assign(path, callback);
	addHandlerName(path);
}
void RESTServerContext::registerCallback_DELETE(std::string path, RequestCallback callback)
{
	m_callbacks_DELETE.insert_or_assign(path, callback);
	addHandlerName(path);
}
void RESTServerContext::registerCallback_HEAD(std::string path, RequestCallback callback)
{
	m_callbacks_HEAD.insert_or_assign(path, callback);
	addHandlerName(path);
}

RequestCallback* RESTServerContext::retrieveCallback_GET(std::string name)
{
	auto callback = m_callbacks_GET.find(name);
	if (callback == m_callbacks_GET.end()) return nullptr;
	return &callback->second;
}  
RequestCallback* RESTServerContext::retrieveCallback_PUT(std::string name)
{
	auto callback = m_callbacks_PUT.find(name);
	if (callback == m_callbacks_PUT.end()) return nullptr;
	return &callback->second;
}    
RequestCallback* RESTServerContext::retrieveCallback_POST(std::string name)
{
	auto callback = m_callbacks_POST.find(name);
	if (callback == m_callbacks_POST.end()) return nullptr;
	return &callback->second;
}   
RequestCallback* RESTServerContext::retrieveCallback_UPDATE(std::string name)
{
	auto callback = m_callbacks_UPDATE.find(name);
	if (callback == m_callbacks_UPDATE.end()) return nullptr;
	return &callback->second;
}  
RequestCallback* RESTServerContext::retrieveCallback_DELETE(std::string name)
{
	auto callback = m_callbacks_DELETE.find(name);
	if (callback == m_callbacks_DELETE.end()) return nullptr;
	return &callback->second;
}  
RequestCallback* RESTServerContext::retrieveCallback_HEAD(std::string name)
{
	auto callback = m_callbacks_HEAD.find(name);
	if (callback == m_callbacks_HEAD.end()) return nullptr;
	return &callback->second;
}
