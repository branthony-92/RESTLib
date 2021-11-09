#include "StdAfx.h"
#include "MdlTokenInfo.h"

TokenInfoBody::TokenInfoBody()
	: JSONInfoBody(JSONInfoBody::BodyType::Body_TokenInfo)
	, m_val("")
	, m_expirationTime()
	, m_timeout(0)
	, m_type(Lifetime::Invalid)
{}

JSON TokenInfoBody::toJSON() const 
{
	auto info = JSON::object();

	// send the timeout as a string cause it's so big
	auto expirationTimepoint = std::chrono::duration_cast<std::chrono::milliseconds>(m_expirationTime.time_since_epoch()).count();
	std::ostringstream oss;
	oss << expirationTimepoint;

	info["Token_Value"]			 = m_val;
	info["Expiration_Timepoint"] = oss.str();
	info["Lifetime"]			 = m_timeout;
	info["Lifetime_Type"]		 = static_cast<unsigned int>(m_type);

	return info;
}

void TokenInfoBody::loadJSON(JSON info)
{
	m_type    = jsonUtils::extractValue<Lifetime, unsigned int>(info, "Lifetime_Type", Lifetime::Invalid);
	m_val     = jsonUtils::extractValue<std::string>(info, "Token_Value", "");
	m_timeout = jsonUtils::extractValue<unsigned int>(info, "Lifetime", UINT32_MAX);

	auto exp = jsonUtils::extractValue<std::string>(info, "Expiration_Timepoint", "0");

	try
	{
		auto durSinceEpoc = std::stoll(exp);
		std::chrono::milliseconds t(durSinceEpoc);
		m_expirationTime = TimeStamp(t);
	}
	catch (std::exception)
	{
		m_expirationTime = TimeStamp(std::chrono::milliseconds(0));
	}
}
