#pragma once

#include "common/IRecognitionFactory.hpp"

#include <jarvisto/Network.hpp>
#include <jarvisto/SecureContext.hpp>

#include <string>
#include <optional>

namespace jar::wit {

class RecognitionFactory final : public IRecognitionFactory {
public:
    RecognitionFactory();

    [[nodiscard]] bool
    canRecognizeMessage() const final;

    [[nodiscard]] std::shared_ptr<Recognition>
    message(io::any_io_executor executor, std::shared_ptr<DataChannel> channel) final;

    [[nodiscard]] bool
    canRecognizeSpeech() const final;

    [[nodiscard]] std::shared_ptr<Recognition>
    speech(io::any_io_executor executor, std::shared_ptr<DataChannel> channel) final;

private:
    std::optional<std::string> _remoteHost;
    std::optional<std::string> _remotePort;
    std::optional<std::string> _remoteAuth;
    SecureContext _context;
};

} // namespace jar::wit