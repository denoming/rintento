#pragma once

#include <spdlog/spdlog.h>

// clang-format off
#define LOGD(...) \
    SPDLOG_DEBUG(__VA_ARGS__)
#define LOGI(...) \
    SPDLOG_INFO(__VA_ARGS__)
#define LOGW(...) \
    SPDLOG_WARN(__VA_ARGS__)
#define LOGE(...) \
    SPDLOG_ERROR(__VA_ARGS__)
// clang-format on