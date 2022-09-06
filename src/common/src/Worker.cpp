#include "common/Worker.hpp"

namespace jar {

void
Worker::start(std::size_t threadsNum)
{
    assert(threadsNum > 0);
    for (std::size_t n = 0; n < threadsNum; ++n) {
        _threads.emplace_back(
            [](net::io_context& context) {
                auto guard = net::make_work_guard(context);
                context.run();
            },
            std::ref(_context));
    }
}

void
Worker::stop()
{
    _context.stop();

    for (auto& thread : _threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}

net::io_context&
Worker::context()
{
    return _context;
}

const net::io_context&
Worker::context() const
{
    return _context;
}

net::io_context::executor_type
Worker::executor()
{
    return _context.get_executor();
}

} // namespace jar