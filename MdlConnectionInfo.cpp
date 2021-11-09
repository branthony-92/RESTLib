#include "StdAfx.h"
#include "MdlConnectionInfo.h"


ConnectionInfoBody::ConnectionInfoBody()
	: JSONInfoBody(JSONInfoBody::BodyType::Body_ConnectionInfo)
	, m_pToken(nullptr)
	, m_id(0)
{}

JSON ConnectionInfoBody::toJSON() const
{
	JSON info = JSON::object();

	info["Connection_ID"] = m_id;

	if (m_pToken)
	{
		info[m_pToken->c_name] = m_pToken->toJSON();
	}
	return info;
}

void ConnectionInfoBody::loadJSON(JSON info)
{
	m_id = jsonUtils::extractValue<unsigned int>(info, "Connection_UD", UINT32_MAX);

	m_pToken = std::make_shared<TokenInfoBody>();
	auto tokenBody = jsonUtils::extractValue<JSON>(info, m_pToken->c_name, JSON::object());
	m_pToken->loadJSON(tokenBody);
}