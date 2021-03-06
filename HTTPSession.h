#ifndef HTTP_SESSION_H
#define HTTP_SESSION_H

#include "Session.h"

namespace {

    void logtrace(std::string fn)
    {
        std::cout << fn << " Entered\n";
    }

}
class GenericSessionHTTP : public SessionBase, public std::enable_shared_from_this<GenericSessionHTTP>
{
    std::shared_ptr<beast::tcp_stream>  m_pHTTP_Stream;
    http::request<http::string_body>    m_request;
    beast::flat_buffer                  m_buffer;

    std::shared_ptr<void> m_pResponse;
public:
    GenericSessionHTTP(RESTCtxList& contexts)
        : SessionBase(contexts, false)
        , m_pHTTP_Stream(nullptr)
    {}
    virtual std::shared_ptr<SessionBase> clone(tcp::socket&& sock, ssl::context& SSLCtx) override // clone must move a socket 
    {
        auto pSession = std::make_shared<GenericSessionHTTP>(std::move(sock), m_serverContexts);
        return pSession;
    }

    virtual void run() override;
    void doRead();
    void onRead(beast::error_code ec, std::size_t bytesTransferred);


    template< class Body, class Allocator>
    void handleRequest(http::request<Body, http::basic_fields<Allocator>>&& req);

    void onWrite(bool close, beast::error_code ec, std::size_t bytesTransferred);
    void doClose();

    explicit GenericSessionHTTP(tcp::socket&& socket, RESTCtxList& contexts)
        : SessionBase(contexts, false)
        , m_pHTTP_Stream(std::make_shared<beast::tcp_stream>(std::move(socket)))
    {}

protected:
    beast::string_view  mime_type(beast::string_view path);
};


template< class Body, class Allocator>
void GenericSessionHTTP::handleRequest(http::request<Body, http::basic_fields<Allocator>>&& req)
{
    // Returns a server error response
    // Returns a not found response
    auto const not_found = [&](beast::string_view target)
    {
        http::response<http::string_body> res{ http::status::not_found, req.version() };
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, "text/html");
        res.keep_alive(req.keep_alive());
        res.body() = "The resource '" + std::string(target) + "' was not found.";
        res.prepare_payload();
        http::write(*m_pHTTP_Stream, res);
    };

    auto const method_error = [&](beast::string_view what)
    {
        http::response<http::string_body> res{ http::status::method_not_allowed, req.version() };
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, "text/html");
        res.keep_alive(req.keep_alive());
        res.body() = "Method not supported" + std::string(what) + "'";
        res.prepare_payload();
        http::write(*m_pHTTP_Stream, res);
    };

    auto const server_error = [&](beast::string_view what)
    {
        http::response<http::string_body> res{ http::status::internal_server_error, req.version() };
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, "text/html");
        res.keep_alive(req.keep_alive());
        res.body() = "An error occurred: '" + std::string(what) + "'";
        res.prepare_payload();
        http::write(*m_pHTTP_Stream, res);
    };

    logtrace("GenericSessionHTTP::handleRequest");
    
    std::shared_ptr<JSONInfoBody> pResponse = nullptr;
    try
    {
        beast::string_view target = req.target();
        // find the endpoint

        beast::string_view methodStr = req.method_string();

        auto epString = extractEndpoint(target);

        // retrieve the JSON body from the request body if there is one
        auto body(extractBody(req));

        // extract the queries
        auto queries(extractQueries(target));

        std::cout << "Request received: " << methodStr << " " << target << "\n";
        
        // find the method and handle the request
        http::verb method = req.method();
        RequestData reqData = {epString, queries, body};
       
        for (auto pCtx : m_serverContexts)
        {
            if (!pCtx) continue;
            
            RequestCallback* callback = nullptr;

            switch (method)
            {
                case boost::beast::http::verb::delete_:
                    callback = pCtx->retrieveCallback_DELETE(epString);
                    break;
                case boost::beast::http::verb::get:
                    callback = pCtx->retrieveCallback_GET(epString);
                    break;
                case boost::beast::http::verb::head:
                    callback = pCtx->retrieveCallback_HEAD(epString);
                    break;
                case boost::beast::http::verb::post:
                    callback = pCtx->retrieveCallback_POST(epString);
                    break;
                case boost::beast::http::verb::put:
                    callback = pCtx->retrieveCallback_PUT(epString);
                    break;
                default:
                    break;
            }
            if (callback)
            {
                pResponse = (*callback)(reqData);
                break;
            }
        }

        if (!pResponse) throw std::runtime_error("Failed to retrieve response");
        switch (pResponse->getReqTypeTag())
        {
        case JSONInfoBody::RequestTypeTag::APIRequest:
        {
            // dump the json body
            auto pResp = std::make_shared<http::response<http::string_body>>(http::status::ok, req.version());
            pResp->set(http::field::server, BOOST_BEAST_VERSION_STRING);
            pResp->set(http::field::content_type, "text/html");
            pResp->keep_alive(req.keep_alive());
            pResp->body() = pResponse->toJSON().dump();
            pResp->prepare_payload();
            m_pResponse = pResp;

            // Write the response
            http::async_write(*m_pHTTP_Stream, *pResp,
                beast::bind_front_handler(
                    &GenericSessionHTTP::onWrite,
                    shared_from_this(),
                    pResp->need_eof()
                )
            );
            break;
        }
        case JSONInfoBody::RequestTypeTag::ResourceRequest:
        {
            std::string path = "";
            auto pResourceBody = std::dynamic_pointer_cast<ResourceInfoBody>(pResponse);
            if (!pResourceBody) throw std::runtime_error("Could not retrieve resource message body");

            path = pResourceBody->getPath();
            
            if (path.empty())
            {
                not_found(path);
                return;
            }

            beast::error_code ec;
            http::file_body::value_type body;

            std::cout << "Resource requested: " << path << "\n";
            body.open(path.c_str(), beast::file_mode::scan, ec);
            if (ec)
            {
                not_found(path);
                return;
            }

            // Respond to GET request
            const auto type = mime_type(path);
            const auto size = body.size();
            auto pResp = std::make_shared<http::response<http::file_body>>(
                std::piecewise_construct,
                std::make_tuple(std::move(body)),
                std::make_tuple(http::status::ok, req.version()));
            pResp->set(http::field::server, BOOST_BEAST_VERSION_STRING);
            pResp->set(http::field::content_type, type);
            pResp->content_length(size);
            pResp->keep_alive(req.keep_alive());
            m_pResponse = pResp;

            // Write the response
            http::async_write(*m_pHTTP_Stream, *pResp,
                beast::bind_front_handler(
                    &GenericSessionHTTP::onWrite,
                    shared_from_this(),
                    pResp->need_eof()));
        }
        case JSONInfoBody::RequestTypeTag::UnknownRequest:
        default:
            server_error("Unknown handler type");
            break;
        }
    }
    catch (std::exception& err)
    {
        server_error(err.what());
    }
}

#endif  