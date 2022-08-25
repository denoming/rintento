#pragma once

#include <boost/asio.hpp>
#include <boost/beast/ssl.hpp>

#include <thread>

namespace net = boost::asio;
namespace ssl = net::ssl;

namespace jar {

class TestWorker {
public:
    TestWorker();

    inline net::io_context&
    context();

    inline const net::io_context&
    context() const;

    inline ssl::context&
    sslContext();

    inline const ssl::context&
    sslContext() const;

    inline net::any_io_executor&
    executor();

    inline const net::any_io_executor&
    executor() const;

    void
    start();

    void
    stop();

private:
    std::thread _thread;
    ssl::context _sslContext;
    net::io_context _context;
    net::any_io_executor _executor;
};

//
// Inlines
//

net::io_context&
TestWorker::context()
{
    return _context;
}

const net::io_context&
TestWorker::context() const
{
    return _context;
}

ssl::context&
TestWorker::sslContext()
{
    return _sslContext;
}

const ssl::context&
TestWorker::sslContext() const
{
    return _sslContext;
}

net::any_io_executor&
TestWorker::executor()
{
    return _executor;
}

const net::any_io_executor&
TestWorker::executor() const
{
    return _executor;
}

} // namespace jar