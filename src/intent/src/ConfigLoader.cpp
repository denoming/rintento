#include "intent/ConfigLoader.hpp"

#include <jarvisto/Logger.hpp>
#include <jarvisto/Utils.hpp>

#include <boost/property_tree/json_parser.hpp>

#include <fstream>

namespace fs = std::filesystem;
namespace pt = boost::property_tree;

namespace jar {

bool
ConfigLoader::load()
{
    bool rv{false};
    if (const auto filePathOpt = getEnvVar("RINTENTO_CONFIG"); filePathOpt) {
        fs::path filePath{*filePathOpt};
        rv = load(filePath);
    } else {
        LOGE("Set the path to config file using RINTENTO_CONFIG env variable");
    }
    return rv;
}

bool
ConfigLoader::load(std::istream& stream)
{
    try {
        pt::ptree tree;
        pt::read_json(stream, tree);
        doParse(tree);
    } catch (const pt::json_parser_error& e) {
        LOGE("Unable to parse <{}> config file on <{}> line: {}",
             e.filename(),
             e.line(),
             e.message());
        return false;
    }
    return true;
}

bool
ConfigLoader::load(std::string_view str)
{
    if (str.empty()) {
        LOGE("Config is empty");
        return false;
    }
    std::istringstream stream{std::string{str}, std::ios::in};
    return load(stream);
}

bool
ConfigLoader::load(fs::path path)
{
    std::error_code error;
    if (!fs::exists(path, error)) {
        LOGE("Config file <{}> doesn't exist", path);
        return false;
    }

    std::ifstream stream{path};
    if (!stream) {
        LOGE("Unable to open file stream for <{}> config file", path);
        return false;
    }

    return load(stream);
}

} // namespace jar