#ifndef MDLTOKENINFO_H
#define MDLTOKENINFO_H

#include "RESTModels.h"
#include <chrono>

typedef std::chrono::system_clock::time_point TimeStamp;
class TokenInfoBody : public JSONInfoBody
{
public:
	enum class Lifetime {
		Transient,
		Persistent,
		Invalid = -1,
	};

	TokenInfoBody();

	JSON toJSON() const override;
	void loadJSON(JSON info) override;

	std::string  getVal()            const { return m_val;			  }
	TimeStamp	 getExpirationTime() const { return m_expirationTime; }
	unsigned int getTimeout()	     const { return m_timeout;		  }
	Lifetime	 getType()		     const { return m_type;           }
	
	void setVal(std::string val)	      { m_val = val;            }
	void setExpirationTime(TimeStamp val) { m_expirationTime = val; }
	void setTimeout(unsigned int val)     { m_timeout = val;        }
	void setType(Lifetime val)		      { m_type = val;           }

	bool compare(const std::string& other) { return m_val.compare(other) == 0; }

private:
	std::string  m_val;
	TimeStamp	 m_expirationTime;
	unsigned int m_timeout;
	Lifetime	 m_type;
};

#endif // MDLTOKENINFO_H