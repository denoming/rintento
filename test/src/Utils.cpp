#include "test/Utils.hpp"

#include <fstream>

namespace jar {

std::unique_ptr<char[]>
readFile(fs::path filePath, std::size_t& fileSize)
{
    std::fstream fs{filePath, std::ios::in | std::ios::binary};
    fileSize = static_cast<long>(fs::file_size(filePath));
    auto fileData = std::make_unique<char[]>(fileSize);
    fs.read(reinterpret_cast<char*>(fileData.get()), fileSize);
    return fileData;
}

} // namespace jar