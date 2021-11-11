#ifndef MDLRESPONSEINFO_H
#define MDLRESPONSEINFO_H

#include "RESTModels.h"

class ResponseInfoBody : public JSONInfoBody
{
public:


	ResponseInfoBody();
	virtual ~ResponseInfoBody() {}

	virtual JSON toJSON() const override;
	virtual void loadJSON(JSON info) override;

	void addBody(std::shared_ptr<JSONInfoBody> pBody);

	std::shared_ptr<JSONInfoBody> findBody(std::string name) const;

	unsigned int getTransactionID() const { return m_transactionID; }
	void setTransactionID(unsigned int id) { m_transactionID = id; }

	void reset() {
		m_bodies.clear();
	}


private:
	unsigned int m_transactionID;
	BodyMap      m_bodies;

};
typedef std::shared_ptr<ResponseInfoBody> ReponseBodyPtr;
#endif // !MDLRESPONSEINFO_H