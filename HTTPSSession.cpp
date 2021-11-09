#include "StdAfx.h"
#include "HTTPSSession.h"


void GenericSessionHTTPS::run()
{
    // We need to be executing within a strand to perform async operations
    // on the I/O objects in this session. Although not strictly necessary
    // for single-threaded contexts, this example code is written to be
    // thread-safe by default.
    net::dispatch(
        m_pHTTPS_Stream->get_executor(),
        beast::bind_front_handler(
            &GenericSessionHTTPS::onRun,
            shared_from_this()));
}

void GenericSessionHTTPS::onRun()
{
    // Set the timeout.
    beast::get_lowest_layer(*m_pHTTPS_Stream).expires_after(std::chrono::seconds(30));

    // Perform the SSL handshake
    m_pHTTPS_Stream->async_handshake(
        ssl::stream_base::server,
        beast::bind_front_handler(
            &GenericSessionHTTPS::onHandshake,
            shared_from_this()));
}

void GenericSessionHTTPS::onHandshake(beast::error_code ec)
{
    if (ec) {
        auto what = ec.message();
    }

    doRead();
}
void GenericSessionHTTPS::doRead()
{
    // Make the request empty before reading,
   // otherwise the operation behavior is undefined.
    m_request = {};

    // Set the timeout.
    beast::get_lowest_layer(*m_pHTTPS_Stream).expires_after(std::chrono::seconds(30));

    // Read a request
    http::async_read(*m_pHTTPS_Stream, m_buffer, m_request,
        beast::bind_front_handler(
            &GenericSessionHTTPS::onRead,
            shared_from_this()));
}

void GenericSessionHTTPS::onRead(beast::error_code ec, std::size_t bytesTransferred)
{
    boost::ignore_unused(bytesTransferred);

    // This means they closed the connection
    if (ec == http::error::end_of_stream)
        return doClose();

    if (ec)
        return doClose();

    // Send the response
    handleRequest(std::move(m_request));
}

void GenericSessionHTTPS::onWrite(bool close, beast::error_code ec, std::size_t bytesTransferred)
{
    boost::ignore_unused(bytesTransferred);
    return doClose();

    if (ec)
        return;

    if (close)
    {
        // This means we should close the connection, usually because
        // the response indicated the "Connection: close" semantic.
        return doClose();
    }

    // We're done with the response so delete it
    m_pResponse = nullptr;

    // Read another request
    doRead();
}

void GenericSessionHTTPS::doClose()

{
    // Set the timeout.
    beast::get_lowest_layer(*m_pHTTPS_Stream).expires_after(std::chrono::seconds(30));

    // Perform the SSL shutdown
    m_pHTTPS_Stream->async_shutdown(
        beast::bind_front_handler(
            &GenericSessionHTTPS::onShutdown,
            shared_from_this()));
}

void GenericSessionHTTPS::onShutdown(beast::error_code ec)
{
    if (ec)
        return;

    // At this point the connection is closed gracefully
}

beast::string_view GenericSessionHTTPS::mime_type(beast::string_view path)
{
    using beast::iequals;
    auto const ext = [&path]
    {
        auto const pos = path.rfind(".");
        if (pos == beast::string_view::npos)
            return beast::string_view{};
        return path.substr(pos);
    }();
    if (iequals(ext, ".htm"))  return "text/html";
    if (iequals(ext, ".html")) return "text/html";
    if (iequals(ext, ".php"))  return "text/html";
    if (iequals(ext, ".css"))  return "text/css";
    if (iequals(ext, ".txt"))  return "text/plain";
    if (iequals(ext, ".js"))   return "application/javascript";
    if (iequals(ext, ".json")) return "application/json";
    if (iequals(ext, ".xml"))  return "application/xml";
    if (iequals(ext, ".swf"))  return "application/x-shockwave-flash";
    if (iequals(ext, ".flv"))  return "video/x-flv";
    if (iequals(ext, ".png"))  return "image/png";
    if (iequals(ext, ".jpe"))  return "image/jpeg";
    if (iequals(ext, ".jpeg")) return "image/jpeg";
    if (iequals(ext, ".jpg"))  return "image/jpeg";
    if (iequals(ext, ".gif"))  return "image/gif";
    if (iequals(ext, ".bmp"))  return "image/bmp";
    if (iequals(ext, ".ico"))  return "image/vnd.microsoft.icon";
    if (iequals(ext, ".tiff")) return "image/tiff";
    if (iequals(ext, ".tif"))  return "image/tiff";
    if (iequals(ext, ".svg"))  return "image/svg+xml";
    if (iequals(ext, ".svgz")) return "image/svg+xml";
    return "application/text";
}

