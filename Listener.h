#ifndef LISTENER_H
#define LISTENER_H

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>
#include <boost/config.hpp>

#include "HTTPSession.h"
#include "HTTPSSession.h"

#include "MdlResponseInfo.h"

namespace beast = boost::beast;             // from <boost/beast.hpp>
namespace http = beast::http;               // from <boost/beast/http.hpp>
namespace net = boost::asio;                // from <boost/asio.hpp>
namespace ssl = boost::asio::ssl;           // from <boost/asio/ssl.hpp>
using     tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

class RESTServerContext;
typedef std::shared_ptr<RESTServerContext> TRESTCtxPtr;
typedef std::shared_ptr<ssl::context>      SSLCtxPtr;
typedef std::shared_ptr<net::io_context>   IOCtxPtr;

/* 
    Accepts incoming connections and launches the sessions 
    The listening routine is pretty generic so we only really 
    need to pass it a session prototype to customize session behaviour
*/
class Listener : public std::enable_shared_from_this<Listener>
{
    net::io_context&        m_IOContext;
    ssl::context&           m_SSLContext;
    tcp::acceptor           m_acceptor;
    SessionPtr              m_pSessionPrototype;
    std::vector<SessionPtr> m_liveSessions;

public:
    Listener(
        net::io_context& ioc,
        ssl::context& ctx,
        tcp::endpoint endpoint,
        SessionPtr pSessionPrototype);
    virtual ~Listener();

    // Start accepting incoming connections
    void run() { do_accept(); }

private:
    void do_accept();

    void on_accept(beast::error_code ec, tcp::socket socket);

    // Report a failure
    void  fail(beast::error_code ec, char const* what);

    void handleClosedSessions();
};
typedef std::shared_ptr<Listener> ListenerPtr;


#endif // !LISTENER_H