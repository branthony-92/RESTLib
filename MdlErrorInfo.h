#ifndef MDLERRORINFO_H
#define MDLERRORINFO_H

#include "RESTModels.h"
#include "RESTServerException.h"

class ErrorInfoBody : public JSONInfoBody
{
public:
	ErrorInfoBody();

	virtual JSON toJSON() const override;
	virtual void loadJSON(JSON info) override;

	void fromException(RESTServerException& err);

	std::string		getMsg()  const {  return m_errorMessage; }
	ServerErrorCode getCode() const { return m_errorCode;     }  

	void getMsg(std::string val)      { m_errorMessage = val; }
	void getCode(ServerErrorCode val) { m_errorCode = val; }

protected:
	std::string		m_errorMessage;
	ServerErrorCode m_errorCode;
};

#endif // !MDLERRORINFO_H