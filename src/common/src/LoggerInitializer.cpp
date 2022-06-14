#include "common/LoggerInitializer.hpp"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/pattern_formatter.h>
#include <spdlog/fmt/fmt.h>

#include <filesystem>

namespace fs = std::filesystem;

class ShotFilenameAndLine : public spdlog::custom_flag_formatter {
public:
    void
    format(const spdlog::details::log_msg& msg,
           const std::tm&,
           spdlog::memory_buf_t& buffer) override
    {
        static constexpr const char* kFullFormat{"{:.<25}"};
        static constexpr const char* kNullFormat{"{:^5}"};

        if (msg.source.empty()) {
            fmt::format_to(std::back_inserter(buffer), kNullFormat, std::string{"?:?"});
        } else {
            auto fileNameAndLine{fs::path{msg.source.filename}.filename().string()};
            fileNameAndLine += ':';
            fileNameAndLine += std::to_string(msg.source.line);
            fmt::format_to(std::back_inserter(buffer), kFullFormat, fileNameAndLine);
        }
    }

    [[nodiscard]] std::unique_ptr<custom_flag_formatter>
    clone() const override
    {
        return spdlog::details::make_unique<ShotFilenameAndLine>();
    }
};

namespace jar {

void
LoggerInitializer::initialize()
{
    static constexpr const char* kFormat{"[%Y-%m-%d %H:%M:%S.%e] [%P:%t] %^[%L]%$ [%*] %v"};

    auto formatter = std::make_unique<spdlog::pattern_formatter>();
    formatter->add_flag<ShotFilenameAndLine>('*').set_pattern(kFormat);

    auto logger = spdlog::stdout_color_mt("MAIN");
    assert(logger);
    logger->set_level(spdlog::level::debug);
    logger->set_formatter(std::move(formatter));
    spdlog::set_default_logger(logger);
}

} // namespace jar