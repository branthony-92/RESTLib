#include "StdAfx.h"
#include "RESTConnection.h"

#include <iostream>
#include <vector>
#include <random>
#include <functional> //for std::function
#include <algorithm>  //for std::generate_n

typedef std::vector<char> char_array;
const char_array c_tokenChars = {	
	'0','1','2','3','4','5','6','7','8','9',
	'A','B','C','D','E','F','G','H','I','J','K','L','M',
	'N','O','P','Q','R','S','T','U','V','W','X','Y','Z',
	'a','b','c','d','e','f','g','h','i','j','k','l','m',
	'n','o','p','q','r','s','t','u','v','w','x','y','z'		
};
const auto c_tokenLength = 64u;

ConnectionManager::ConnectionManager()
	: m_connections()
{
}

ConnectionManager::~ConnectionManager()
{
}


std::shared_ptr<TokenInfoBody> ConnectionManager::refreshConnection(const unsigned int id)
{
	auto pConnection = getConnection(id);
	if (!pConnection) return nullptr;
	
	auto pToken = pConnection->getToken();

	std::chrono::seconds duration(pToken->getTimeout());
	pToken->setExpirationTime(std::chrono::system_clock::now() + duration);

	pToken->setVal(generateTokenValue());
	pConnection->setStatus(ConnectionStatus::Open);

	return pToken;
}

void ConnectionManager::removeClosedConnections()
{
	for (auto iter = m_connections.begin(); iter != m_connections.end(); iter++)
	{
		auto pConnection = iter->second;
		if (pConnection->getStatus() == ConnectionStatus::Closed)
		{
			iter = m_connections.erase(iter);
		}
	}
}

unsigned int ConnectionManager::generateID()
{
	std::random_device gen;
	auto id = gen();
	return id;
}

std::string ConnectionManager::generateTokenValue()
{
	//1) create a non-deterministic random number generator      
	std::default_random_engine rng(std::random_device{}());

	//2) create a random number "shaper" that will give
	//   us uniformly distributed indices into the character set
	std::uniform_int_distribution<> dist(0u, c_tokenChars.size() - 1u);

	//3) create a function that ties them together, to get:
	//   a non-deterministic uniform distribution from the 
	//   character set of your choice.
	auto randchar = [&]() {return c_tokenChars[dist(rng)]; };

	//4) set the length of the string you want and profit!        
	std::string str(c_tokenLength, 0);

	std::generate_n(str.begin(), c_tokenLength, randchar);
	return str;
}

ConnectionManager::ConnectionStatus ConnectionManager::validateConnection(const unsigned int id, const std::string& token)
{
	auto pConnection = getConnection(id);
	if (!pConnection) return ConnectionStatus::Invalid;

	// check the ID
	if (pConnection->getID() != id)
	{
		m_connections.erase(m_connections.find(id));
		return ConnectionStatus::Invalid;
	}

	// check the token
	if (!pConnection->getToken()->compare(token))
	{
		m_connections.erase(m_connections.find(id));
		return ConnectionStatus::BadToken;
	}

	// check the age
	if (pConnection->isTimedOut())
	{
		pConnection->setStatus(ConnectionStatus::StaleToken);
	}

	return pConnection->getStatus();
}


void ConnectionManager::closeConnection(const unsigned int id)
{
	auto pConnection = getConnection(id);
	if (!pConnection) return;

	pConnection->setStatus(ConnectionStatus::Closed);
}

ConnectionManager::TConnectionPtr ConnectionManager::openNewConnection(TokenInfoBody::Lifetime type, unsigned int timeout)
{
	auto connectionID = generateID();


	// check if this ID is already in use
	TConnectionPtr pConnection = getConnection(connectionID);
	while (pConnection != nullptr)
	{
		// generate a new ID until we get a new one
		connectionID = generateID();
		pConnection = getConnection(connectionID);
	}

	pConnection = std::make_shared<Connection>(connectionID);

	// generate the token
	auto pToken = std::make_shared<TokenInfoBody>();
	pToken->setType(type);
	pToken->setTimeout(timeout);
	pToken->setVal(generateTokenValue());

	std::chrono::seconds duration(timeout);
	pToken->setExpirationTime(std::chrono::system_clock::now() + duration);

	pConnection->setToken(pToken);
	pConnection->setStatus(ConnectionStatus::Open);

	m_connections.insert_or_assign(connectionID, pConnection);
	return pConnection;
}

ConnectionManager::TConnectionPtr ConnectionManager::getConnection(unsigned int id)
{
	TConnectionPtr pConnection = nullptr;
	auto connectionIter = m_connections.find(id);

	if (connectionIter != m_connections.end())
	{
		pConnection = m_connections[id];
	}
	return pConnection;
}

bool ConnectionManager::Connection::isTimedOut() const
{
	auto now = std::chrono::system_clock::now();
	return (now > m_pConnectionToken->getExpirationTime());
}
