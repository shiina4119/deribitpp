#include "broadcast_session.hpp"
#include "helper.hpp"
#include <boost/asio/dispatch.hpp>
#include <boost/asio/error.hpp>
#include <fmt/base.h>
#include <memory>

namespace net = boost::asio;

broadcast_session::broadcast_session(tcp::socket &&socket)
    : ws(std::move(socket)) {}

void broadcast_session::run() {
    net::dispatch(ws.get_executor(),
                  beast::bind_front_handler(&broadcast_session::on_run,
                                            shared_from_this()));
}

void broadcast_session::on_run() {
    ws.set_option(
        websocket::stream_base::timeout::suggested(beast::role_type::server));
    ws.async_accept(beast::bind_front_handler(&broadcast_session::on_accept,
                                              shared_from_this()));
}

void broadcast_session::on_accept(beast::error_code ec) {
    if (ec) {
        return fail(ec, "accept");
    }
    print_console("BROADCAST_SESSION: receiver connected");
}

void broadcast_session::do_write(std::shared_ptr<std::string> msg) {
    ws.async_write(net::buffer(*msg),
                   beast::bind_front_handler(&broadcast_session::on_write,
                                             shared_from_this()));
}

void broadcast_session::on_write(beast::error_code ec, std::size_t) {
    if (ec) {
        return fail(ec, "write");
    }
}

void broadcast_session::fail(beast::error_code ec, std::string what) {
    if (ec == net::error::operation_aborted || ec == websocket::error::closed ||
        ec == net::error::broken_pipe)
        return;
    fmt::println("{}: {}", what, ec.message());
}
