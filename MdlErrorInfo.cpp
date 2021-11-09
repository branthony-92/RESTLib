#include "StdAfx.h"
#include "MdlErrorInfo.h"

ErrorInfoBody::ErrorInfoBody()
	: JSONInfoBody(JSONInfoBody::BodyType::Body_ErrorInfo)
	, m_errorMessage("")
	, m_errorCode(ServerErrorCode::NoError)
{}

JSON ErrorInfoBody::toJSON() const
{
	auto info = JSON::object();
	auto code = static_cast<unsigned int>(m_errorCode);
	info["Error_Code"] = code;
	info["Error_Message"] = m_errorMessage;
	return info;
}

void ErrorInfoBody::loadJSON(JSON info)
{
	m_errorCode    = jsonUtils::extractValue<ServerErrorCode, unsigned int>(info, "Error_Code", ServerErrorCode::Generic);
	m_errorMessage = jsonUtils::extractValue<std::string>(info, "Error_Message", "");
}

void ErrorInfoBody::fromException(RESTServerException& err)
{
	m_errorCode = err.errorCode();
	m_errorMessage = err.what();
}