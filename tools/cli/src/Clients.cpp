#include "cli/Clients.hpp"

#include "intent/Constants.hpp"
#include "intent/Utils.hpp"
#include "jarvis/Logger.hpp"

#include <boost/json.hpp>

#include <fstream>

namespace json = boost::json;

namespace jar::clients {

static Result
getResult(const std::string& input)
{
    sys::error_code error;
    auto value = json::parse(input, error);
    if (error) {
        return {};
    }

    if (auto object = value.if_object(); object) {
        bool status{false};
        if (auto statusObj = object->if_contains("status"); statusObj) {
            if (auto statusIf = statusObj->if_bool(); statusIf) {
                status = *statusIf;
            }
        }
        std::string statusError;
        if (auto errorObj = object->if_contains("error"); errorObj) {
            if (auto errorIf = errorObj->if_string(); errorIf) {
                statusError.assign(errorIf->begin(), errorIf->end());
            }
        }
        return std::make_tuple(status, std::move(statusError));
    }

    return {};
}

template<typename EndpointSequence>
static Result
recognizeMessage(io::any_io_executor executor,
                 const EndpointSequence& endpoints,
                 std::string_view message)
{
    beast::tcp_stream stream{executor};
    LOGD("Connect to the given host");
    stream.connect(endpoints);

    const auto target = format::messageTarget(message);
    http::request<http::empty_body> req{http::verb::get, target, net::kHttpVersion11};
    req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    LOGD("Write recognize request");
    http::write(stream, req);

    http::response<http::string_body> res;
    {
        beast::flat_buffer buffer;
        LOGD("Read response to recognize request");
        http::read(stream, buffer, res);
    }

    LOGD("Close connection to host");
    stream.close();

    return getResult(res.body());
}

template<typename EndpointSequence>
Result
recognizeSpeech(io::any_io_executor executor,
                const EndpointSequence& endpoints,
                fs::path speechFile)
{
    beast::tcp_stream stream{executor};
    LOGD("Connect to the given host");
    stream.connect(endpoints);

    const auto target = format::speechTarget();
    http::request<http::empty_body> req{http::verb::post, target, net::kHttpVersion11};
    req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    req.set(http::field::transfer_encoding, "chunked");
    req.set(http::field::expect, "100-continue");
    {
        LOGD("Write recognize request");
        http::request_serializer<http::empty_body> reqSer{req};
        http::write_header(stream, reqSer);
    }

    {
        beast::flat_buffer buffer;
        http::response<http::empty_body> res;
        LOGD("Read response to recognize request");
        http::read(stream, buffer, res);
        if (res.result() != http::status::continue_) {
            LOGE("100 continue expected");
            stream.close();
            return {};
        }
    }

    auto fileSize = fs::file_size(speechFile);
    if (fileSize == 0) {
        LOGE("Failed to get speech data file size");
        return {};
    }
    std::fstream fs{speechFile, std::ios::in | std::ios::binary};
    if (!fs.is_open()) {
        LOGE("Failed to open speech data file");
        return {};
    }
    auto fileData = std::make_unique<char[]>(fileSize);
    LOGD("Read speech data file");
    fs.read(reinterpret_cast<char*>(fileData.get()), fileSize);

    {
        const auto buffer = io::const_buffer(fileData.get(), fileSize);
        LOGD("Write <{}> bytes of speech data to socket", fileSize);
        io::write(stream, http::make_chunk(buffer));
        LOGD("Finalize writing of speech fata");
        io::write(stream, http::make_chunk_last());
    }

    http::response<http::string_body> res;
    {
        beast::flat_buffer buffer;
        LOGD("Read result of recognizing of speech");
        http::read(stream, buffer, res);
    }

    LOGD("Close connection to host");
    stream.close();

    return getResult(res.body());
}

Result
recognizeMessage(io::any_io_executor executor, std::uint16_t port, std::string_view message)
{
    tcp::endpoint endpoint{io::ip::make_address_v4("127.0.0.1"), port};
    return recognizeMessage(executor, std::vector{endpoint}, message);
}

Result
recognizeMessage(io::any_io_executor executor,
                 std::string_view host,
                 std::string_view port,
                 std::string_view message)
{
    tcp::resolver resolver{executor};
    auto const results = resolver.resolve(host, port);
    if (results.empty()) {
        LOGE("Failed to resolve given host");
        return {};
    }
    return recognizeMessage(executor, results, message);
}

Result
recognizeSpeech(io::any_io_executor executor, std::uint16_t port, fs::path speechFile)
{
    tcp::endpoint endpoint{io::ip::make_address_v4("127.0.0.1"), port};
    return recognizeSpeech(executor, std::vector{endpoint}, speechFile);
}

Result
recognizeSpeech(io::any_io_executor executor,
                std::string_view host,
                std::string_view port,
                fs::path speechFile)
{
    tcp::resolver resolver{executor};
    auto const results = resolver.resolve(host, port);
    if (results.empty()) {
        LOGE("Failed to resolve given host");
        return {};
    }
    return recognizeSpeech(executor, results, speechFile);
}

} // namespace jar::clients