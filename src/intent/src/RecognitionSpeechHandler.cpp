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

#include "intent/RecognitionSpeechHandler.hpp"

#include "common/IRecognitionFactory.hpp"
#include "intent/Utils.hpp"

#include <jarvisto/core/Logger.hpp>
#include <jarvisto/network/Http.hpp>

#include <boost/asio/experimental/awaitable_operators.hpp>
#include <boost/assert.hpp>

using namespace boost::asio::experimental::awaitable_operators;

namespace jar {

RecognitionSpeechHandler::Ptr
RecognitionSpeechHandler::create(Stream& stream,
                                 Buffer& buffer,
                                 Parser& parser,
                                 std::shared_ptr<IRecognitionFactory> factory)
{
    return Ptr(new RecognitionSpeechHandler(stream, buffer, parser, std::move(factory)));
}

RecognitionSpeechHandler::RecognitionSpeechHandler(Stream& stream,
                                                   Buffer& buffer,
                                                   Parser& parser,
                                                   std::shared_ptr<IRecognitionFactory> factory)
    : RecognitionHandler{stream}
    , _buffer{buffer}
    , _parser{parser}
    , _factory{std::move(factory)}
{
    BOOST_ASSERT(_factory);
}

io::awaitable<RecognitionResult>
RecognitionSpeechHandler::handle()
{
    static const std::size_t kChannelCapacity = 1'000'000 /* 1Mb */;

    if (not canHandle()) {
        co_return co_await RecognitionHandler::handle();
    }

    auto executor = co_await io::this_coro::executor;
    auto channel = std::make_shared<IRecognitionFactory::DataChannel>(executor, kChannelCapacity);
    auto recognition = _factory->speech(executor, channel);
    BOOST_ASSERT(recognition);
    auto result = co_await (sendSpeechData(channel) && recognition->run());
    co_await sendResponse(result);
    co_return std::move(result);
}

bool
RecognitionSpeechHandler::canHandle() const
{
    return parser::isSpeechTarget(_parser.get().target());
}

io::awaitable<void>
RecognitionSpeechHandler::sendSpeechData(std::shared_ptr<Channel> channel)
{
    if (auto& request = _parser.get(); request[http::field::expect] != "100-continue") {
        LOGE("100-continue is expected: session");
        throw std::runtime_error{"Missing expect field in request header"};
    }

    http::response<http::empty_body> res{http::status::continue_, kHttpVersion11};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    co_await http::async_write(stream(), res, io::use_awaitable);

    std::string chunk;
    auto onHeader = [&](std::uint64_t size, std::string_view extensions, sys::error_code& ec) {
        chunk.reserve(size);
        chunk.clear();
    };
    auto onBody = [&](std::uint64_t remain, std::string_view body, sys::error_code& ec) {
        if (remain == body.size()) {
            ec = http::error::end_of_chunk;
        }
        chunk.append(body.data(), body.size());
        return body.size();
    };
    _parser.on_chunk_header(onHeader);
    _parser.on_chunk_body(onBody);

    sys::error_code ec;
    while (not _parser.is_done()) {
        co_await http::async_read(
            stream(), _buffer, _parser, io::redirect_error(io::use_awaitable, ec));
        if (not ec) {
            continue;
        } else {
            if (ec != http::error::end_of_chunk) {
                LOGE("Error receiving speech data: error<{}>", ec.message());
                break;
            } else {
                ec = {};
            }
        }
        co_await channel->send(io::buffer(chunk));
    }

    co_await channel->send(io::error::eof);
    channel->close();
}

} // namespace jar