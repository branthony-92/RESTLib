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
	, m_pConnectionMgr(std::make_unique<ConnectionManager>())
{
	//replace spaces with underscores for the json body name
	for (char& c : name)
	{
		if (c == ' ') c = '_';
	}
	m_pHandlerInfo = std::make_shared<ContextHandlersInfoBody>(name);
}

void RESTServerContext::registerHandler(std::string path, ReqHandlerPtr pEndpoint)
{
	if (!pEndpoint || path.empty()) return;
	m_requestHandlers.insert_or_assign(path, pEndpoint);
	addHandlerName(path);
}

std::shared_ptr<HTTPRequestHandler> RESTServerContext::retrieveHandler(std::string name)
{
	auto handler = m_requestHandlers.find(name);
	if (handler == m_requestHandlers.end()) return nullptr;
	return handler->second;
}

bool RESTServerContext::hasHandlers() const
{
	return !m_requestHandlers.empty();
}

bool RESTServerContext::addHandlerName(std::string name)
{
	if (!m_pHandlerInfo) return false;
	return m_pHandlerInfo->add(name);
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
