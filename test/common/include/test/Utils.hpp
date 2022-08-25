#pragma once

#include <filesystem>

namespace fs = std::filesystem;

namespace jar {

std::unique_ptr<char[]>
readFile(fs::path filePath, std::size_t& fileSize);

}
