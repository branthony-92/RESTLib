#include "StdAfx.h"
#include "ServerInfoContext.h"

ServerInfoContext::ServerInfoContext()
	: RESTServerContext("Server_Info_Context")
	, m_pServerInfo(nullptr)
{
	auto pHandler = std::make_shared<ServerInfoRequestEndpoint>();

	registerHandler(pHandler->getName(), pHandler);
}


std::shared_ptr<JSONInfoBody> ServerInfoRequestEndpoint::handleRequest_Get(std::string target, ParameterMap queries, ParameterMap body, TRESTCtxPtr pCtx)
{
	auto pInfoCtx = std::dynamic_pointer_cast<ServerInfoContext>(pCtx);
	if (!pInfoCtx)
	{
		throw std::runtime_error("Failed to retrieve handler context");
	}

	auto pServerInfo = pInfoCtx->getServerInfo();
	return pServerInfo;
}
