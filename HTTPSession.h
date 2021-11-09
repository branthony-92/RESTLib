#ifndef HTTP_SESSION_H
#define HTTP_SESSION_H

#include "Session.h"

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

    std::shared_ptr<JSONInfoBody> pResponse = nullptr;
    try
    {
        beast::string_view target = req.target();
        ReqHandlerPtr pHandler = nullptr;
        // find the endpoint

        beast::string_view methodStr = req.method_string();
        std::cout << "Request received: " << methodStr << " " << target << "\n";

        auto epString = extractEndpoint(target);

        // retrieve the JSON body from the request body if there is one
        ParsedBody body = extractBody(req);

        // extract the queries
        ParameterMap queries(extractQueries(target));

        // find the method and handle the request
        http::verb method = req.method();

        RESTCtxPtr pContext = nullptr;

        if (epString.find("/api/") != std::string::npos)
        {
            auto handlerInfo = findHandler(epString);
            if (handlerInfo.pCtx && handlerInfo.pHanlder)
            {
                pContext = handlerInfo.pCtx;
                pHandler = handlerInfo.pHanlder;
            }
        }
        else
        {
            pHandler = std::make_shared<ResourceRequestHandler>();

        }
        if (!pHandler) throw std::runtime_error("Failed to retrieve request handler");

        pHandler->resetResponseBody();

        auto handleReq = [&](RESTCtxPtr pCtx) -> std::shared_ptr<JSONInfoBody> {
            switch (method)
            {
                case boost::beast::http::verb::delete_:
                    return pHandler->handleRequest_Delete(epString, queries, ParameterMap(body), pCtx);
                case boost::beast::http::verb::get:
                    return pHandler->handleRequest_Get(epString, queries, ParameterMap(body), pCtx);
                case boost::beast::http::verb::head:
                    return pHandler->handleRequest_Head(epString, queries, ParameterMap(body), pCtx);
                case boost::beast::http::verb::post:
                    return pHandler->handleRequest_Post(epString, queries, ParameterMap(body), pCtx);
                case boost::beast::http::verb::put:
                    return pHandler->handleRequest_Put(epString, queries, ParameterMap(body), pCtx);
                default:
                    break;
            }
            return nullptr;
        };

        switch (pHandler->getType())
        {
        case HTTPRequestHandler::RequestType::APIRequest:
        {
            pResponse = handleReq(pContext);

            if (!pResponse) throw std::runtime_error("Failed to retrieve response");

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
        case HTTPRequestHandler::RequestType::ResourceRequest:
        {
            std::string path = "";

            // fetch the desired resource
            // try all of our owned contexts
            for (auto& pCtx : m_serverContexts)
            {
                pResponse = handleReq(pCtx);
      
                auto pResourceBody = std::dynamic_pointer_cast<ResourceInfoBody>(pResponse);
                if (!pResourceBody) continue;

                // use the first path we find from the request
                if (!pResourceBody->getPath().empty())
                {
                    path = pResourceBody->getPath();
                    break;
                }
            }
            beast::error_code ec;
            http::file_body::value_type body;

            if (path.empty())
            {
                not_found(path);
                return;
            }

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
        case HTTPRequestHandler::RequestType::UnknownRequest:
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