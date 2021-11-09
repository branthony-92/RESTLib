#ifndef RESTCONNECTION_H
#define RESTCONNECTION_H

#include "MdlConnectionInfo.h"
#include "MdlTokenInfo.h"

class ConnectionManager {
public:
	enum class ConnectionStatus {
		Uninitialized,
		Open,
		StaleToken,
		BadToken,
		Closed,

		Invalid = -1,
	};

	class Connection;
	typedef std::shared_ptr<ConnectionManager::Connection>  TConnectionPtr;
	typedef std::map<unsigned int, TConnectionPtr>          TConnectionMap;
	typedef std::pair<unsigned int, TConnectionPtr>         TConnectionMapData;

	ConnectionManager();
	virtual ~ConnectionManager();

	// connection management
	TConnectionPtr openNewConnection(TokenInfoBody::Lifetime type, unsigned int timeout);
	TConnectionPtr getConnection(unsigned int id);

	TConnectionMap getConnections() const { return m_connections; }

	void closeConnection(const unsigned int id);
	ConnectionStatus validateConnection(const unsigned int id, const std::string& token);
	std::shared_ptr<TokenInfoBody> refreshConnection(const unsigned int id);

	void removeClosedConnections();
	
private:
	unsigned int generateID();
	std::string generateTokenValue();

private:
	TConnectionMap m_connections;
};
typedef std::unique_ptr<ConnectionManager> TConnectionMgrPtr;

class ConnectionManager::Connection
{

public:
	Connection(unsigned int id = -1)
		: m_connectionID(id)
		, m_pConnectionToken(nullptr)
		, m_connectionStatus(ConnectionStatus::Uninitialized)
	{
	}
	Connection(const ConnectionManager::Connection& other)
		: m_connectionID(other.getID())
		, m_pConnectionToken(other.getToken())
		, m_connectionStatus(other.getStatus())
	{
	}

	Connection& operator=(const ConnectionManager::Connection& other)
	{
		m_connectionID = other.getID();
		m_pConnectionToken = other.getToken();
		m_connectionStatus = other.getStatus();

		return *this;
	}
	virtual ~Connection() {}

public:
	unsigned int getID()						const { return m_connectionID; }
	std::shared_ptr<TokenInfoBody> getToken()	const { return m_pConnectionToken; }
	ConnectionStatus getStatus()				const { return m_connectionStatus; }

	void setID(unsigned int id)				             { m_connectionID = id; }
	void setToken(std::shared_ptr<TokenInfoBody> token)	 { m_pConnectionToken = token; }
	void setStatus(ConnectionStatus state)				 { m_connectionStatus = state; }

	bool isTimedOut() const;


private:
	unsigned int                    m_connectionID;
	std::shared_ptr<TokenInfoBody>  m_pConnectionToken;
	ConnectionStatus				m_connectionStatus;
};
#endif // !RESTCONNECTION_H