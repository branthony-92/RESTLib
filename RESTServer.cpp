#include "StdAfx.h"
#include "RESTServer.h"
#include "HTTPUtils.h"


Server::Server(unsigned int threads)
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

	auto pInfoCtx = std::make_shared<ServerInfoContext>();
	pInfoCtx->setServerInfo(m_pInfo);

	registerContext(pInfoCtx);
}

Server::~Server()
{
	reset();
}

bool Server::startServer(std::string address, unsigned short port, SessionPtr pSessionPrototype, const SSLCtxInitHandler& sslInitHandler)
{
	reset();
	m_pIOContext = std::make_shared<net::io_context>(m_threadCount);

	std::string schema = "http";
	// server-owner defined SSL certificate initialization
	if (pSessionPrototype->m_useSSLTLS)
	{
		m_pSSLContext = std::make_shared<ssl::context>(ssl::context_base::tlsv12_server);
		if (!sslInitHandler(*m_pSSLContext)) return false;
		schema = "https";		
	}

	m_pListener = std::make_shared<Listener>(
		*m_pIOContext,
		*m_pSSLContext,
		tcp::endpoint{ net::ip::make_address(address), port },
		pSessionPrototype);

	m_pListener->run();

	for (auto i = 0u; i < m_threadCount; i++)
	{
		// run our IO context threads
		m_ioCtxThreads.emplace_back([&] { m_pIOContext->run(); });
	}

	auto name = m_pInfo->getServerName();
	if (name.empty()) name = "Test Server";

	std::ostringstream uri;
	uri << schema << "://" << address << ":" << port;

	m_pInfo->setURLString(uri.str());

	std::cout << "Server Listening: " << uri.str() << "\n";
	for (auto& context : m_serverContexts)
	{
		// call the context server startup callbacks
		context->onServerStart();
	}
	return true;
}

bool Server::reset()
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
		for (auto& t : m_ioCtxThreads)
		{
			if (t.joinable()) t.join();
		}

		m_ioCtxThreads.clear();

		// clear the contexts last
		m_pIOContext  = nullptr;
		m_pSSLContext = nullptr;

		for (auto& context : m_serverContexts)
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

void Server::shutdown()
{
	for (auto& context : m_serverContexts)
	{
		context->onServerShutdown();
	}
	m_serverContexts.clear();
}

void Server::registerContext(RESTCtxPtr pContext)
{
	m_serverContexts.push_back(pContext);
	auto& endpoints = m_pInfo->getEndpointNames();
	endpoints.push_back(pContext->getHandlerInfo());
}

void Server::unregisterContext(RESTCtxPtr pContext)
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
	auto& endpoints = m_pInfo->getEndpointNames();
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
