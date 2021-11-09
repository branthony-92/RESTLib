#include "StdAfx.h"
#include "MdlUriInfo.h"

URIInfoBody::URIInfoBody()
	: JSONInfoBody(JSONInfoBody::BodyType::Body_URIInfo)
	, m_address("")
	, m_port(0u)
{}

JSON URIInfoBody::toJSON() const 
{
	auto info = JSON::object();
	info["Address"] = m_address;
	info["Port"]    = m_port;
	return info;
}
void URIInfoBody::loadJSON(JSON info) 
{
	m_address = jsonUtils::extractValue<std::string>(info, "Address", "");
	m_port    = jsonUtils::extractValue<unsigned int>(info, "Port", 0u);
}