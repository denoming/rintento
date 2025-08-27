#include "common/ServiceLogger.hpp"

namespace jar {

ServiceLogger::ServiceLogger(std::string ident)
    : _ident{std::move(ident)}
{
}

void
ServiceLogger::addSinks(std::shared_ptr<spdlog::logger> logger, const int logLevel)
{
    addConsoleSink(std::move(logger), logLevel);
}

} // namespace jar