#ifndef MDLURIINFO_H
#define MDLURIINFO_H

#include "RESTModels.h"

class URIInfoBody : public JSONInfoBody
{
public:
	URIInfoBody();

	virtual JSON toJSON() const override;
	virtual void loadJSON(JSON info) override;

	std::string  getAddress() { return m_address; }
	unsigned int getPort()    { return m_port;    } 

	void setAddress(std::string val) { m_address = val; }
	void setPort(unsigned int val)   { m_port = val;    }

private:
	std::string  m_address;
	unsigned int m_port;
};

#endif // !MDLURIINFO_H