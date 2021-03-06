#ifndef SESSION_H
#define SESSION_H

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>
#include <boost/config.hpp>

#include <list>

#include "MdlResponseInfo.h"
#include "RESTEndpoint.h"

#include "RESTServerContext.h"

namespace beast = boost::beast;             // from <boost/beast.hpp>
namespace http = beast::http;               // from <boost/beast/http.hpp>
namespace net = boost::asio;                // from <boost/asio.hpp>
namespace ssl = boost::asio::ssl;           // from <boost/asio/ssl.hpp>
using     tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

typedef std::list<TRESTCtxPtr>              RESTCtxList;
typedef std::shared_ptr<ssl::context>       SSLCtxPtr;
typedef std::shared_ptr<net::io_context>    IOCtxPtr;


/*  The class Session Handles an HTTP server connection
*
    Extend from this class to define your session specific logic which will be called via the run() function.
    This class will be used by the Listener class as a prototype so the implimentation classes must override the
    clone() method to return a shared pointer to that subclass.
 */
class SessionBase
{
protected:

    std::atomic_bool  m_closed;
    RESTCtxList&      m_serverContexts;

public:
    const std::string c_sessionName;
    const bool        m_useSSLTLS;

    explicit SessionBase(RESTCtxList& contexts, bool useSSL = false)
        : m_closed(true)
        , m_serverContexts(contexts)
        , m_useSSLTLS(useSSL)
    {}

    // clone must move a socket, the SSL context is only needed for HTTPS sessions
    virtual std::shared_ptr<SessionBase> clone(tcp::socket&& sock, ssl::context& SSLCtx) = 0; 

    virtual void run() = 0;

    std::string getName() const { return c_sessionName; }
    bool isClosed() const { return m_closed.load(); };

    void setClosed(bool closed) { m_closed = closed; }

    static JSON parseBody(std::string contentType, const std::string body);
    static JSON extractQueries(beast::string_view target);
    static std::string extractEndpoint(beast::string_view target);


protected:
    template< class Body, class Allocator>
    JSON extractBody(http::request<Body, http::basic_fields<Allocator>> req)
    {
        JSON bodyData = JSON::object();

        return bodyData;
    }
   
    JSON extractBody(http::request<http::string_body, http::basic_fields<std::allocator<char>>> req)
    {
        JSON bodyData = JSON::object();

        auto& header = req.base();

        if (header.find("Content-Type") == header.end())
        {
            return bodyData;
        }
        auto body = req.body();
        if (body.empty()) return bodyData;

        auto content = header.at("Content-Type");
        auto contentType = std::string(content.data(), content.length());

        bodyData = parseBody(contentType, body);
        return bodyData;
    }
};
typedef std::shared_ptr<SessionBase> SessionPtr;

#endif // !SESSION_H

