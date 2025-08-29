// Copyright 2025 Denys Asauliak
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "cli/Recognizer.hpp"

#include "wit/Utils.hpp"

#include <boost/json.hpp>
#include <jarvisto/core/Logger.hpp>
#include <jarvisto/network/Http.hpp>
#include <sndfile.hh>

namespace json = boost::json;

namespace jar {

Recognizer::Recognizer(io::any_io_executor executor)
    : _executor{std::move(executor)}
{
}

Recognizer::Result
Recognizer::recognizeMessage(std::string_view host, std::string_view port, std::string_view message)
{
    sys::error_code error;

    beast::tcp_stream stream{_executor};
    {
        LOGD("Resolve <{}> host address", host, port);
        const auto result = resolve(host, port);
        if (not result) {
            return {false, "Resolving address has failed"};
        }
        LOGD("Connect to <{}> host", host, port);
        const tcp::endpoint endpoint = stream.connect(result.value(), error);
        if (error) {
            LOGE("Connecting to <{}> has failed: {}", host, port, error.what());
            return {false, "Connecting has failed"};
        } else {
            LOGD("Connected to <{}> host address", endpoint.address().to_string());
        }
    }

    {
        const auto target = wit::messageTarget(message);
        http::request<http::empty_body> req{http::verb::get, target, kHttpVersion11};
        req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
        LOGD("Writing recognize message request");
        std::size_t bytesTransferred = http::write(stream, req, error);
        if (error) {
            LOGE("Writing has failed: {}", error.what());
            return {false, "Writing has failed"};
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
            return {false, "Reading has failed"};
        } else {
            LOGD("The <{}> bytes has been read", bytesTransferred);
        }
    }

    LOGD("Close connection");
    stream.close();

    return getResult(res.body());
}

Recognizer::Result
Recognizer::recognizeSpeech(std::string_view host,
                            std::string_view port,
                            const std::filesystem::path& audioFilePath)
{
    sys::error_code error;

    beast::tcp_stream stream{_executor};
    {
        LOGD("Resolve <{}> host address", host, port);
        const auto result = resolve(host, port);
        if (not result) {
            return {false, "Resolving address has failed"};
        }
        LOGD("Connect to <{}> host", host, port);
        const tcp::endpoint endpoint = stream.connect(result.value(), error);
        if (error) {
            LOGE("Connecting to <{}> has failed: {}", host, port, error.what());
            return {false, "Connecting has failed"};
        } else {
            LOGD("Connected to <{}> host address", endpoint.address().to_string());
        }
    }

    {
        const auto target = wit::speechTarget();
        http::request<http::empty_body> req{http::verb::post, target, kHttpVersion11};
        req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
        req.set(http::field::transfer_encoding, "chunked");
        req.set(http::field::expect, "100-continue");

        LOGD("Write recognize speech request header");
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
        SndfileHandle audioFile{audioFilePath};
        if (audioFile) {
            LOGD("Opened file: {}", audioFilePath);
            LOGD("...Sample rate: {}", audioFile.samplerate());
            LOGD("...Channels: {}", audioFile.channels());
            LOGD("...Frames: {}", audioFile.frames());
        } else {
            LOGE("Unable to open <{}> audio file", audioFilePath);
            return {false, "Unable to open file"};
        }

        sf_count_t bytesRead{};
        constexpr const std::size_t kBufferSize{1024};
        std::array<char, kBufferSize> buffer = {0};
        do {
            bytesRead = audioFile.readRaw(buffer.data(), kBufferSize);
            if (bytesRead > 0) {
                const auto chunk = http::make_chunk(io::const_buffer(buffer.data(), bytesRead));
                LOGD("Write <{}> bytes of speech data", bytesRead);
                std::size_t bytesTransferred = io::write(stream, chunk, error);
                if (error) {
                    LOGE("Writing has failed: {}", error.what());
                    return {false, "Writing speech data chunk has failed"};
                } else {
                    LOGD("Writing <{}> bytes was successful", bytesTransferred);
                }
            }
        }
        while (bytesRead == kBufferSize);

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

std::expected<tcp::resolver::results_type, sys::error_code>
Recognizer::resolve(std::string_view host, std::string_view port)
{
    tcp::resolver resolver{_executor};
    LOGD("Resolving address of <{}> host", host);
    sys::error_code error;
    auto result = resolver.resolve(host, port, error);
    if (error) {
        LOGE("Resolving address has failed: {}", error.what());
        return std::unexpected(error);
    } else {
        LOGD("The <{}> endpoints has been resolved", result.size());
        return result;
    }
}

Recognizer::Result
Recognizer::getResult(std::string_view input)
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

} // namespace jar