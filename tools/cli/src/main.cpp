#include <iostream>

#include "cli/Clients.hpp"

#include <jarvisto/Logger.hpp>
#include <jarvisto/LoggerInitializer.hpp>
#include <jarvisto/Worker.hpp>

#include <boost/program_options.hpp>

#include <filesystem>

namespace po = boost::program_options;
namespace fs = std::filesystem;

using namespace jar;

int
main(int argn, char* argv[])
{
    LoggerInitializer::instance().initialize();

    std::string message;
    std::string audioFile;
    uint16_t serverPort;

    po::options_description d{"Rintento CLI"};
    // clang-format off
    d.add_options()
        ("help,h", "Display help")
        ("message,m", po::value<std::string>(&message), "Recognize message")
        ("speech,s", po::value<std::string>(&audioFile), "Recognize speech")
        ("port,p", po::value<uint16_t>(&serverPort)->default_value(8080), "Recognize server port")
    ;
    // clang-format on

    po::variables_map vm;
    po::store(po::parse_command_line(argn, argv, d), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << d << std::endl;
        return EXIT_SUCCESS;
    }

    Worker worker;
    worker.start();

    if (vm.contains("message")) {
        const auto [ok, error] = clients::recognizeMessage(worker.executor(), serverPort, message);
        if (!ok) {
            LOGE("Recognizing of message has failed: {}", error);
        }
        return EXIT_SUCCESS;
    }

    if (vm.contains("speech")) {
        fs::path filePath{audioFile};
        if (!fs::exists(filePath)) {
            LOGE("File <{}> not found", filePath);
            return EXIT_FAILURE;
        }
        const auto [ok, error] = clients::recognizeSpeech(worker.executor(), serverPort, filePath);
        if (!ok) {
            LOGE("Recognizing of speech has failed: {}", error);
        }
        return EXIT_SUCCESS;
    }

    LOGI("Nothing to do");
    return EXIT_SUCCESS;
}