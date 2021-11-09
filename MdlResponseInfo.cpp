#include "StdAfx.h"
#include "MdlResponseInfo.h"


ResponseInfoBody::ResponseInfoBody()
	: JSONInfoBody(JSONInfoBody::BodyType::Body_ReponseInfo)
	, m_bodies()
	, m_transactionID()
{
}

JSON ResponseInfoBody::toJSON() const
{
	auto info = JSON::object();
	info["Transaction_ID"] = m_transactionID;

	for (auto b : m_bodies)
	{
		auto name = b.first;
		auto pBody = b.second;
		info[name] = pBody->toJSON();
	}
	return info;
}

void ResponseInfoBody::loadJSON(JSON info)
{
	m_transactionID = jsonUtils::extractValue<unsigned int>(info, "Transaction_ID", UINT32_MAX);

	for (JSON::iterator iter = info.begin(); iter != info.end(); iter++ )
	{
		// look for the bodies we may want and add it to the response body list	
		iter.key();
		auto pBody = JSONInfoBody::createBody(iter.key());
		if (pBody)
		{
			pBody->loadJSON(iter.value());
			m_bodies.insert_or_assign(pBody->c_name, pBody);
		}
	}
}

void ResponseInfoBody::addBody(std::shared_ptr<JSONInfoBody> pBody)
{
	m_bodies.insert_or_assign(pBody->c_name, pBody);
}

std::shared_ptr<JSONInfoBody> ResponseInfoBody::findBody(std::string name) const
{
	auto bodyIter = m_bodies.find(name);
	if (bodyIter != m_bodies.end())
	{
		auto pBody = bodyIter->second;
		return pBody;
	}
	return nullptr;
}
