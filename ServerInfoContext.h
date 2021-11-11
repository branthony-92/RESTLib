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
