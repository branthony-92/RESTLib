#pragma once

#include "MdlServerInfo.h"
#include "RESTEndpoint.h"
#include "RESTServerContext.h"

class ServerInfoContext : public RESTServerContext
{
	ServerInfoPtr m_pServerInfo;

public:
	ServerInfoContext();

	ServerInfoPtr getServerInfo() const { return m_pServerInfo; }
	void setServerInfo(ServerInfoPtr pInfo) { m_pServerInfo = pInfo; }


};

class ServerInfoRequestEndpoint : public HTTPRequestHandler
{
public:
	ServerInfoRequestEndpoint()
		: HTTPRequestHandler("/api/", HTTPRequestHandler::RequestType::APIRequest)
	{}

	virtual std::shared_ptr<JSONInfoBody> handleRequest_Get(std::string target, ParameterMap queries, ParameterMap body, TRESTCtxPtr pCtx) override;
};



