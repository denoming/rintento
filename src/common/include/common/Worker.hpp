#pragma once

#include <boost/asio/io_context.hpp>

#include <thread>
#include <vector>

namespace net = boost::asio;

namespace jar {

class Worker {
public:
    Worker() = default;

    void
    start(std::size_t threadsNum = std::thread::hardware_concurrency());

    void
    stop();

    net::io_context::executor_type
    executor();

private:
    net::io_context _context;
    std::vector<std::thread> _threads;
};

} // namespace jar