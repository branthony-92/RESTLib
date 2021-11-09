#ifndef MDLSERVERINFO_H
#define MDLSERVERINFO_H

#include "RESTModels.h"
#include "MdlUriInfo.h"

#include <list>

class ContextHandlersInfoBody : public JSONInfoBody
{
	std::set<std::string> m_handlerNames;
public:
	ContextHandlersInfoBody(std::string name);

	JSON toJSON() const override;
	void loadJSON(JSON info) override;

	std::set<std::string>& getHandlerNames()        { return m_handlerNames; }
	void setHandlerNames(std::set<std::string> val) { m_handlerNames = val; }

	bool add(std::string name);
};
typedef std::shared_ptr<ContextHandlersInfoBody> HandlersInfoBodyPtr;

class ServerInfoBody : public JSONInfoBody
{
public:
	enum class ServerStatus {
		Uninitialized,
		Listening,
		Error,
	};

	ServerInfoBody();

	std::string                           getServerName()       const { return m_serverName;          }
	double		                          getAPIVer()           const { return m_APIVersion;          }
	std::string                           getURLString()        const { return m_URLString;           }
	std::shared_ptr<URIInfoBody>          getURI()		        const { return m_pURI;				  }
	unsigned int                          getIdleTimeout()      const { return m_serverIdleTimoutSec; }
	ServerStatus	                      getState()			const { return m_serverState;         }
	std::list<HandlersInfoBodyPtr>&       getEndpointNames()	 { return m_endpointNames;       }

	void setServerName(std::string val)               { m_serverName = val;          }
	void setAPIVer(double val)                        { m_APIVersion = val;          }
	void setURLString(std::string val)                { m_URLString = val;           }
	void setURI(std::shared_ptr<URIInfoBody> val)     { m_pURI = val;			     }
	void setIdleTimeout(unsigned int val)             { m_serverIdleTimoutSec = val; }
	void setState(ServerStatus val)                   { m_serverState = val;         }
	void setEndpointNames(std::list<HandlersInfoBodyPtr> val)  { m_endpointNames = val;       }

private:
	std::string                       m_serverName;
	double		                      m_APIVersion;
	std::string                       m_URLString;
	std::shared_ptr<URIInfoBody>      m_pURI;
	unsigned int                      m_serverIdleTimoutSec;
	ServerStatus	                  m_serverState;
	std::list<HandlersInfoBodyPtr>    m_endpointNames;

	std::string stringFromState(ServerStatus state) const;
	JSON toJSON() const override;

	void loadJSON(JSON info) override;
};
typedef std::shared_ptr<ServerInfoBody> ServerInfoPtr;

#endif // !MDLSERVERINFO_H