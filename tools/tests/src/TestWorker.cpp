#include "tests/TestWorker.hpp"

namespace jar {

TestWorker::TestWorker()
    : _sslContext{ssl::context::tlsv12_client}
    , _executor{_context.get_executor()}
{
    _sslContext.set_default_verify_paths();
    _sslContext.set_verify_mode(ssl::verify_peer);
}

void
TestWorker::start()
{
    _thread = std::thread{[this]() {
        auto guard = net::make_work_guard(_context);
        _context.run();
    }};
}

void
TestWorker::stop()
{
    _context.stop();
    if (_thread.joinable()) {
        _thread.join();
    }
}

} // namespace jar
