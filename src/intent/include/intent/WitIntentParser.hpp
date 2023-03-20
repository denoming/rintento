#pragma once

#include "intent/IntentParser.hpp"

#include <memory>

namespace jar {

class WitIntentParser final : public IntentParser {
public:
    WitIntentParser();

    ~WitIntentParser() final;

    [[nodiscard]] std::optional<UtteranceSpecs>
    parse(std::string_view input) final;

private:
    class Impl;
    std::unique_ptr<Impl> _impl;
};

} // namespace jar