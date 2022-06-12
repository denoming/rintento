#pragma once

#include <string>
#include <functional>

namespace jar {

using RecognitionCalback = std::function<void(std::string intent)>;

}