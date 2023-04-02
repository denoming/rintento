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
    sys::error_code error;

    beast::tcp_stream stream{executor};
    LOGD("Connecting to the given endpoints");
    tcp::endpoint endpoint = stream.connect(endpoints, error);
    if (error) {
        LOGE("Connecting to the given endpoint has failed: {}", error.what());
    } else {
        LOGD("Connected to <{}> endpoint", endpoint.address().to_string());
    }

    {
        const auto target = format::messageTarget(message);
        http::request<http::empty_body> req{http::verb::get, target, net::kHttpVersion11};
        req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
        LOGD("Writing recognize message request upon <{}{}> target",
             endpoint.address().to_string(),
             target);
        std::size_t bytesTransferred = http::write(stream, req, error);
        if (error) {
            LOGE("Writing has failed: {}", error.what());
        } else {
            LOGD("The <{}> bytes has been written", bytesTransferred);
        }
    }

    http::response<http::string_body> res;
    {
        beast::flat_buffer buffer;
        LOGD("Reading recognize message response");
        std::size_t bytesTransferred = http::read(stream, buffer, res, error);
        if (error) {
            LOGE("Reading has failed: {}", error.what());
        } else {
            LOGD("The <{}> bytes has been read", bytesTransferred);
        }
    }

    LOGD("Close connection to <{}> endpoint", endpoint.address().to_string());
    stream.close();

    return getResult(res.body());
}

template<typename EndpointSequence>
Result
recognizeSpeech(io::any_io_executor executor,
                const EndpointSequence& endpoints,
                fs::path speechFile)
{
    sys::error_code error;

    beast::tcp_stream stream{executor};
    LOGD("Connecting to the given endpoints");
    tcp::endpoint endpoint = stream.connect(endpoints, error);
    if (error) {
        LOGE("Connecting to the given endpoint has failed: {}", error.what());
        return {false, "Connecting has failed"};
    } else {
        LOGD("Connected to <{}> endpoint", endpoint.address().to_string());
    }

    {
        const auto target = format::speechTarget();
        http::request<http::empty_body> req{http::verb::post, target, net::kHttpVersion11};
        req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
        req.set(http::field::transfer_encoding, "chunked");
        req.set(http::field::expect, "100-continue");

        LOGD("Writing recognize speech request header upon <{}{}> target",
             endpoint.address().to_string(),
             target);
        http::request_serializer<http::empty_body> reqSer{req};
        std::size_t bytesTransferred = http::write_header(stream, reqSer, error);
        if (error) {
            LOGE("Writing request header has failed: {}", error.what());
            return {false, "Writing request has failed"};
        } else {
            LOGD("The <{}> bytes has been written", bytesTransferred);
        }
    }

    {
        beast::flat_buffer buffer;
        http::response<http::empty_body> res;
        LOGD("Reading recognize speech response header");
        std::size_t bytesTransferred = http::read(stream, buffer, res, error);
        if (error) {
            LOGE("Reading response header has failed: {}", error.what());
            return {false, "Reading response header has failed"};
        } else {
            LOGD("The <{}> bytes has been read", bytesTransferred);
        }

        if (res.result() != http::status::continue_) {
            LOGE("Invalid response header status");
            return {false, "100 continue expected"};
        }
    }

    {
        std::ifstream is{speechFile, std::ios::binary | std::ios::in | std::ios::ate};
        if (!is) {
            LOGE("Unable to open <{}> speech file", speechFile);
            return {false, "Unable to open speech file"};
        }

        constexpr const std::size_t kBufferSize{5 * 1024};
        std::array<char, kBufferSize> buffer = {0};

        auto totalSize = is.tellg();
        LOGD("The size of <{}> speech file is <{}> bytes", speechFile, totalSize);
        is.seekg(0);

        while (!is.eof()) {
            is.read(buffer.data(), kBufferSize);
            if (auto bytesRead = is.gcount(); bytesRead > 0) {
                const auto chunk = http::make_chunk(io::const_buffer(buffer.data(), bytesRead));
                LOGD("Writing <{}> bytes of speech data", bytesRead);
                std::size_t bytesTransferred = io::write(stream, chunk, error);
                if (error) {
                    LOGE("Writing has failed: {}", error.what());
                    return {false, "Writing speech data chunk has failed"};
                } else {
                    LOGD("The <{}> bytes has been written", bytesTransferred);
                }
            }
        }

        LOGD("Finalizing writing of speech fata");
        std::size_t bytesTransferred = io::write(stream, http::make_chunk_last(), error);
        if (error) {
            LOGE("Finalizing writing of speech data has failed: {}", error.what());
            return {false, "Finalizing writing of speech data has failed"};
        } else {
            LOGD("The <{}> bytes has been written", bytesTransferred);
        }
    }

    http::response<http::string_body> res;
    {
        beast::flat_buffer buffer;
        LOGD("Reading result of speech recognizing");
        std::size_t bytesTransferred = http::read(stream, buffer, res, error);
        if (error) {
            LOGE("Reading speech recognizing result has failed: {}", error.what());
            return {false, "Reading speech recognizing result has failed"};
        } else {
            LOGD("The <{}> bytes has been written", bytesTransferred);
        }
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
    sys::error_code error;

    tcp::resolver resolver{executor};
    LOGD("Resolving address of <{}> host", host);
    auto const results = resolver.resolve(host, port, error);
    if (error) {
        LOGE("Resolving address has failed: {}", error.what());
        return {false, "Resolving address has failed"};
    } else {
        LOGD("The <{}> endpoints has been resolved", results.size());
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
    sys::error_code error;

    tcp::resolver resolver{executor};
    LOGD("Resolving address of <{}> host", host);
    auto const results = resolver.resolve(host, port, error);
    if (error) {
        LOGE("Resolving address has failed: {}", error.what());
        return {false, "Resolving address has failed"};
    } else {
        LOGD("The <{}> endpoints has been resolved", results.size());
    }

    return recognizeSpeech(executor, results, speechFile);
}

} // namespace jar::clients