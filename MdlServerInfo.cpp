#include "StdAfx.h"
#include "MdlServerInfo.h"

ContextHandlersInfoBody::ContextHandlersInfoBody(std::string name)
	: JSONInfoBody(name)
{}

JSON ContextHandlersInfoBody::toJSON() const
{
	JSON info = JSON::object();
	info["Context_Name"] = c_name;
	info["Endpoints"] = JSON::array();

	for (auto& h : m_handlerNames)
	{
		info["Endpoints"].push_back(h);
	}
	return info;
}

void ContextHandlersInfoBody::loadJSON(JSON info)
{
	m_handlerNames = jsonUtils::extractValue<std::set<std::string>>(info, "Endpoints", std::set<std::string>());
}

bool ContextHandlersInfoBody::add(std::string name)
{
	if (name.empty()) return false;
	if (m_handlerNames.count(name) > 0) return false;
	m_handlerNames.insert(name);
	return true;
}


ServerInfoBody::ServerInfoBody()
	: JSONInfoBody(JSONInfoBody::BodyType::Body_ServerInfo)
	, m_serverName("")
	, m_APIVersion(1.0)
	, m_URLString("")
	, m_pURI(std::make_shared<URIInfoBody>())
	, m_serverIdleTimoutSec(static_cast<unsigned int>(-1)) // basically forever
	, m_serverState(ServerStatus::Uninitialized)
	, m_endpointNames()
{}

std::string ServerInfoBody::stringFromState(ServerStatus state) const
{
	std::string name = "";

	switch (state)
	{
	case ServerStatus::Uninitialized: name = "Uninitialized"; break;
	case ServerStatus::Listening:     name = "Listening"; break;
	case ServerStatus::Error:         name = "Listening"; break;
	default:                          name = "Unknown "; break;
	}
	return name;
}

JSON ServerInfoBody::toJSON() const
{
	JSON info = JSON::object();
	JSON uriInfo = JSON::object();

	info["Server_Name"]     = m_serverName;
	info["API_Version"]     = m_APIVersion;
	info["API_Root_URL"]    = m_URLString;
	info["Idle_Timout_Sec"] = m_serverIdleTimoutSec;
	info["Server_Status"]   = stringFromState(m_serverState);

	JSON ar = JSON::array();
	for (auto& e : m_endpointNames)
	{
		JSON h = e->toJSON();
		ar.push_back(h);
	}
	info["Endpoint_List"] = ar;

	if (m_pURI)
	{
		info[m_pURI->c_name] = m_pURI->toJSON();
	}
	return info;
}

void ServerInfoBody::loadJSON(JSON info)
{
	m_serverName          = jsonUtils::extractValue<std::string>(info, "Server_Name", "");
	m_APIVersion          = jsonUtils::extractValue<double>(info, "API_Version", 0.0);
	m_serverIdleTimoutSec = jsonUtils::extractValue<unsigned int>(info, "Idle_Timout_Sec", UINT32_MAX);
	m_URLString			  = jsonUtils::extractValue<std::string>(info, "API_Root_URL", "");

	auto endpoints = jsonUtils::extractValue<std::vector<JSON>>(info, "Endpoint_List", std::vector<JSON>());

	m_endpointNames.clear();
	for (JSON& e : endpoints)
	{
		auto name = jsonUtils::extractValue<std::string>(e, "Context_Name", "");
		if (name.empty()) continue;

		auto pHanderInfo = std::make_shared<ContextHandlersInfoBody>(name);
		jsonUtils::extractValue<std::vector<std::string>>(e, "Hanlders", std::vector<std::string>());

		m_endpointNames.push_back(pHanderInfo);
	}
	m_pURI = std::make_shared<URIInfoBody>();

	auto uriBody = jsonUtils::extractValue<JSON>(info, m_pURI->c_name, JSON::object());
	m_pURI->loadJSON(uriBody);
}
