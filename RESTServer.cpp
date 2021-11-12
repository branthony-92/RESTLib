#include "StdAfx.h"
#include "RESTServer.h"
#include "HTTPUtils.h"

REST_Server::REST_Server(unsigned int threads)
	: m_pListener(nullptr)
	, m_pIOContext(nullptr)
	, m_pSSLContext(nullptr)
	, m_threadCount(threads)
	, m_ioCtxThreads()
	, m_serverContexts()
	, m_pInfo(std::make_shared<ServerInfoBody>())
{

	// make sure we have at least 1 thread
	m_threadCount = std::max(1u, m_threadCount);
	m_ioCtxThreads.reserve(m_threadCount);
	m_pIOContext = std::make_shared<net::io_context>(m_threadCount);
	m_pSSLContext = std::make_shared<ssl::context>(ssl::context_base::tlsv12_server);
}

REST_Server::~REST_Server()
{
	reset();
}

bool REST_Server::reset()
{
	try
	{
		// stop the context
		if (m_pIOContext && !m_pIOContext->stopped())
		{
			m_pIOContext->stop();
		}

		m_pListener = nullptr;

		// join the threads
		for (auto &t : m_ioCtxThreads)
		{
			if (t.joinable())
				t.join();
		}

		m_ioCtxThreads.clear();

		// clear the contexts last
		m_pIOContext = nullptr;
		m_pSSLContext = nullptr;

		for (auto &context : m_serverContexts)
		{
			// call the context server reset callbacks
			context->onServerReset();
		}

		return true;
	}
	catch (std::exception /*& err*/)
	{
		return false;
	}
}

void REST_Server::registerContext(TRESTCtxPtr pContext)
{
	m_serverContexts.push_back(pContext);
	auto &endpoints = m_pInfo->getEndpointNames();
	endpoints.push_back(pContext->getHandlerInfo());
}

void REST_Server::shutdown()
{
	for (auto &context : m_serverContexts)
	{
		context->onServerShutdown();
	}
	m_serverContexts.clear();
}

void REST_Server::unregisterContext(TRESTCtxPtr pContext)
{
	for (auto ctxIter = m_serverContexts.begin(); ctxIter != m_serverContexts.end(); ctxIter++)
	{
		auto pCtx = *ctxIter;

		if (pContext == pCtx)
		{
			m_serverContexts.erase(ctxIter);
			break;
		}
	}
	auto &endpoints = m_pInfo->getEndpointNames();
	for (auto epIter = endpoints.begin(); epIter != endpoints.end(); epIter++)
	{
		auto pEpInfo = *epIter;

		if (pContext->getHandlerInfo() == pEpInfo)
		{
			endpoints.erase(epIter);
			break;
		}
	}
}

bool REST_Server::startServer_HTTP(std::string address, unsigned short port, RESTCtxList contexts)
{
	std::string schema = "http";
	SessionPtr pSessionPrototype = std::make_shared<GenericSessionHTTP>(contexts);

	for (auto pContext : contexts)
	{
		registerContext(pContext);
	}

	boost::system::error_code ec;
	auto ipAddr = boost::asio::ip::address::from_string( address, ec );
	if ( ec )
	{
		std::cerr << "passed address is invalid: " << ec.message() << std::endl;
		ipAddr = boost::asio::ip::address::from_string("127.0.0.1");
		address = "127.0.0.1";
	}

	m_pListener = std::make_shared<Listener>(
		*m_pIOContext,
		*m_pSSLContext,
		tcp::endpoint{ipAddr, port},
		pSessionPrototype);

	m_pListener->run();

	for (auto i = 0u; i < m_threadCount; i++)
	{
		// run our IO context threads
		m_ioCtxThreads.emplace_back([&]
			{ m_pIOContext->run(); });
	}

	auto name = m_pInfo->getServerName();
	if (name.empty())
		name = "Test Server";

	std::ostringstream uri;
	uri << schema << "://" << address << ":" << port;

	m_pInfo->setURLString(uri.str());

	std::cout << "Server Listening: " << uri.str() << "\n";
	for (auto pContext : contexts)
	{
		// call the context server startup callbacks
		pContext->onServerStart();
	}
	return true;
}

bool REST_Server::startServer_HTTPS(std::string address, unsigned short port, RESTCtxList contexts, SSLCtxInitHandler sslInitHandler)
{
	std::string schema = "https";

	SessionPtr pSessionPrototype = std::make_shared<GenericSessionHTTPS>(contexts);

	for (auto pContext : contexts)
	{
		registerContext(pContext);
	}

	// server-owner defined SSL certificate initialization
	if (pSessionPrototype->m_useSSLTLS)
	{
		if (!sslInitHandler(*m_pSSLContext))
			throw std::runtime_error("Failed to start server");
	}

	boost::system::error_code ec;
	auto ipAddr = boost::asio::ip::address::from_string( address, ec );
	if ( ec )
	{
		std::cerr << "passed address is invalid: " << ec.message() << std::endl;
		ipAddr = boost::asio::ip::address::from_string("127.0.0.1");
		address = "127.0.0.1";
	}

	m_pListener = std::make_shared<Listener>(
		*m_pIOContext,
		*m_pSSLContext,
		tcp::endpoint{ipAddr, port},
		pSessionPrototype);

	m_pListener->run();

	for (auto i = 0u; i < m_threadCount; i++)
	{
		// run our IO context threads
		m_ioCtxThreads.emplace_back([&]
			{ m_pIOContext->run(); });
	}

	auto name = m_pInfo->getServerName();
	if (name.empty())
		name = "Test Server";

	std::ostringstream uri;
	uri << schema << "://" << address << ":" << port;

	m_pInfo->setURLString(uri.str());

	std::cout << "Server Listening: " << uri.str() << "\n";
	for (auto pContext : contexts)
	{
		// call the context server startup callbacks
		pContext->onServerStart();
	}
	return true;
}

std::shared_ptr<REST_Server> REST_Server::make_server(unsigned int threadCount, std::string address, unsigned short port, RESTCtxList contexts)
{
	auto pServer = std::make_shared<REST_Server>(threadCount);
	pServer->startServer_HTTP(address, port, contexts);
	return pServer;
}

std::shared_ptr<REST_Server> REST_Server::make_server(unsigned int threadCount, std::string address, unsigned short port, RESTCtxList contexts, SSLCtxInitHandler fHandler)
{
	auto pServer = std::make_shared<REST_Server>(threadCount);
	pServer->startServer_HTTPS(address, port, contexts, fHandler);
	return pServer;
}