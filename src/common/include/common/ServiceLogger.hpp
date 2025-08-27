#pragma once

#include <jarvisto/core/LoggerFactory.hpp>

#include <string>

namespace jar {

class ServiceLogger final : public LoggerFactory {
public:
    explicit ServiceLogger(std::string ident);

private:
    void
    addSinks(std::shared_ptr<spdlog::logger> logger, int logLevel) override;

private:
    std::string _ident;
};

} // namespace jar