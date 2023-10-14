#include <iostream>

#include "cli/Recognizer.hpp"

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
    std::string file;
    std::string host;
    std::string port;

    po::options_description d{"Rintento CLI"};
    // clang-format off
    d.add_options()
        ("help,h", "Display help")
        ("message,m", po::value<std::string>(&message), "Recognize intent from given message")
        ("speech,s", po::value<std::string>(&file), "Recognize intent from given audio file")
        ("host,h", po::value<std::string>(&port)->default_value("127.0.0.1"), "Recognize server host")
        ("port,p", po::value<std::string>(&port)->default_value("8080"), "Recognize server port")
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
        Recognizer recognizer{worker.executor()};
        const auto [ok, error] = recognizer.recognizeMessage(host, port, message);
        if (!ok) {
            LOGE("Recognizing given message has failed: {}", error);
        }
        return EXIT_SUCCESS;
    }

    if (vm.contains("speech")) {
        if (not fs::exists(file)) {
            LOGE("Unable to find <{}> file", file);
            return EXIT_FAILURE;
        }

        Recognizer recognizer{worker.executor()};
        const auto [ok, error] = recognizer.recognizeSpeech(host, port, file);
        if (!ok) {
            LOGE("Recognizing given speech has failed: {}", error);
        }

        return EXIT_SUCCESS;
    }

    return EXIT_FAILURE;
}