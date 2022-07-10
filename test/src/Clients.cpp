#include "test/Clients.hpp"

#include "intent/Constants.hpp"
#include "intent/Utils.hpp"

#include <boost/json.hpp>

namespace json = boost::json;

namespace jar::clients {

static Result
getResult(const std::string& input)
{
    sys::error_code error;
    auto value = json::parse(input, error);
    if (error) {
        return std::make_tuple(false, "");
    }

    if (auto object = value.if_object(); object) {
        bool status{false};
        if (auto statusObject = object->if_contains("status"); statusObject) {
            if (auto statusIf = statusObject->if_bool(); statusIf) {
                status = *statusIf;
            }
        }
        std::string statusError;
        if (auto errorObject = object->if_contains("error"); errorObject) {
            if (auto statusErrorIf = errorObject->if_string(); statusErrorIf) {
                statusError.assign(statusErrorIf->begin(), statusErrorIf->end());
            }
        }
        return std::make_tuple(status, std::move(statusError));
    }

    return std::make_tuple(false, "");
}

Result
recognizeMessage(std::string_view host, std::string_view port, std::string_view message)
{
    net::io_context context;
    tcp::resolver resolver{context};
    auto const results = resolver.resolve(host, port);
    if (results.empty()) {
        return {};
    }

    beast::tcp_stream stream{context};
    stream.connect(results);

    const auto target = format::messageTarget(message);
    http::request<http::string_body> req{http::verb::get, target, kHttpVersion11};
    req.set(http::field::host, host);
    req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    http::write(stream, req);

    beast::flat_buffer buffer;
    http::response<http::string_body> res;
    http::read(stream, buffer, res);

    sys::error_code error;
    stream.socket().shutdown(tcp::socket::shutdown_both, error);

    return getResult(res.body());
}

} // namespace jar::clients