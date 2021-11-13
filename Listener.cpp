#include "StdAfx.h"
#include "Listener.h"

Listener::Listener(net::io_context& ioc, ssl::context& ctx, tcp::endpoint endpoint, SessionPtr pSessionPrototype)
    : m_IOContext(ioc)
    , m_SSLContext(ctx)
    , m_acceptor(ioc)
    , m_pSessionPrototype(pSessionPrototype)
{
    beast::error_code ec;

    // Open the acceptor
    m_acceptor.open(endpoint.protocol(), ec);
    if (ec)
    {
        fail(ec, "open");
        return;
    }

    // Allow address reuse
    m_acceptor.set_option(net::socket_base::reuse_address(true), ec);
    if (ec)
    {
        fail(ec, "set_option");
        return;
    }

    // Bind to the server address
    m_acceptor.bind(endpoint, ec);
    if (ec)
    {
        fail(ec, "bind");
        return;
    }

    // Start listening for connections
    m_acceptor.listen(net::socket_base::max_listen_connections, ec);
    if (ec)
    {
        fail(ec, "listen");
        return;
    }
}

Listener::~Listener()
{
}

void Listener::do_accept()
{
    // The new connection gets its own strand
    m_acceptor.async_accept(
        net::make_strand(m_IOContext),
        beast::bind_front_handler(
            &Listener::on_accept,
            shared_from_this()));
}

void Listener::on_accept(beast::error_code ec, tcp::socket socket)
{
    if (ec)
    {
        fail(ec, "accept");
    }
    else
    {
        //handleClosedSessions();

        // Create the session and run it
        auto pSession = m_pSessionPrototype->clone(std::move(socket), m_SSLContext);
        pSession->run();
        //m_liveSessions.push_back(pSession);
    }

    // Accept another connection
    do_accept();
}

// Report a failure
void Listener::fail(beast::error_code ec, char const* what)
{
    // ssl::error::stream_truncated, also known as an SSL "short read",
    // indicates the peer closed the connection without performing the
    // required closing handshake (for example, Google does this to
    // improve performance). Generally this can be a security issue,
    // but if your communication protocol is self-terminated (as
    // it is with both HTTP and WebSocket) then you may simply
    // ignore the lack of close_notify.
    //
    // https://github.com/boostorg/beast/issues/38
    //
    // https://security.stackexchange.com/questions/91435/how-to-handle-a-malicious-ssl-tls-shutdown
    //
    // When a short read would cut off the end of an HTTP message,
    // Beast returns the error beast::http::error::partial_message.
    // Therefore, if we see a short read here, it has occurred
    // after the message has been completed, so it is safe to ignore it.

    if (ec == net::ssl::error::stream_truncated)
        return;

    std::cerr << what << ": " << ec.message() << "\n";
}

void Listener::handleClosedSessions() {
    auto sessions = m_liveSessions;
    if (sessions.empty()) return;
    auto sessionIter = sessions.begin();

    while (sessionIter != sessions.end());
    {
        auto pSession = *sessionIter++;
        if (pSession->isClosed())
        {
            sessionIter = sessions.erase(sessionIter);
        }
    }
}

