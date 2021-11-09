#ifndef RESTSERVEREXCEPTION_H
#define RESTSERVEREXCEPTION_H

enum class ServerErrorCode
{
	NoError = 0,
	BadContext,
	BadEndpoint,
	BadMethod,
	BadBody,
	BadQuery,
	MethodNotSupported,
	AccessDenied,
	Generic,
};

class RESTServerException : public std::runtime_error
{
public:

	RESTServerException(std::string msg, ServerErrorCode errID = ServerErrorCode::NoError)
		: std::runtime_error(msg.c_str())
		, m_error(errID)
	{}

	RESTServerException(std::runtime_error ex, ServerErrorCode errID = ServerErrorCode::Generic)
		: std::runtime_error(ex)
		, m_error(errID)
	{}

	ServerErrorCode errorCode() { return m_error; }

private:
	ServerErrorCode m_error;
};

#endif // !RESTSERVEREXCEPTION_H