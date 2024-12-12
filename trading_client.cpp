#include "trading_client.hpp"
#include "helper.hpp"
#include <boost/asio/strand.hpp>
#include <fmt/base.h>

trading_client::trading_client(net::io_context &ioc, ssl::context &ctx,
                               std::string host, std::string port,
                               std::string target,
                               std::shared_ptr<shared_state> ss)
    : resolver(net::make_strand(ioc)), ws(net::make_strand(ioc), ctx),
      host(host), port(port), target(target), ss(ss) {}

void trading_client::establish_connection() {
    resolver.async_resolve(
        host, port,
        beast::bind_front_handler(&trading_client::on_resolve,
                                  shared_from_this()));
}

void trading_client::on_resolve(beast::error_code ec,
                                tcp::resolver::results_type results) {
    if (ec) {
        return fail(ec, "resolve");
    }
    print_console("TRADING_CLIENT: Resolved host");
    beast::get_lowest_layer(ws).expires_after(std::chrono::seconds(30));
    beast::get_lowest_layer(ws).async_connect(
        results, beast::bind_front_handler(&trading_client::on_connect,
                                           shared_from_this()));
}

void trading_client::on_connect(beast::error_code ec,
                                tcp::resolver::results_type::endpoint_type ep) {
    if (ec) {
        return fail(ec, "connect");
    }
    print_console("TRADING_CLIENT: Connected to test.deribit.com");
    beast::get_lowest_layer(ws).expires_after(std::chrono::seconds(30));
    host += ':' + std::to_string(ep.port());
    ws.next_layer().async_handshake(
        ssl::stream_base::client,
        beast::bind_front_handler(&trading_client::on_ssl_handshake,
                                  shared_from_this()));
}

void trading_client::on_ssl_handshake(beast::error_code ec) {
    if (ec) {
        return fail(ec, "ssl_handshake");
    }
    print_console("TRADING_CLIENT: SSL handshake complete");
    beast::get_lowest_layer(ws).expires_never();
    ws.set_option(
        websocket::stream_base::timeout::suggested(beast::role_type::client));
    ws.async_handshake(host, target,
                       beast::bind_front_handler(&trading_client::on_handshake,
                                                 shared_from_this()));
}

void trading_client::on_handshake(beast::error_code ec) {
    if (ec) {
        return fail(ec, "handshake");
    }
    print_console("TRADING_CLIENT: Handshake complete");
    print_console("TRADING_CLIENT: Websocket connection established!");
    do_read();
}

void trading_client::do_read() {
    ws.async_read(buffer, beast::bind_front_handler(&trading_client::on_read,
                                                    shared_from_this()));
}

void trading_client::on_read(beast::error_code ec,
                             std::size_t bytes_transferred) {
    boost::ignore_unused(bytes_transferred);
    if (ec) {
        return fail(ec, "read");
    }
    std::string buffer_message = std::string(
        net::buffer_cast<const char *>(buffer.data()), buffer.size());
    ss->enqueue(std::make_shared<std::string>(std::move(buffer_message)));
    buffer.consume(buffer.size());
    do_read();
}

void trading_client::do_write(std::shared_ptr<std::string const> const &msg) {
    outgoing.emplace(msg);
    if (outgoing.size() > 1) {
        return;
    }
    ws.async_write(net::buffer(*outgoing.front()),
                   beast::bind_front_handler(&trading_client::on_write,
                                             shared_from_this()));
}

void trading_client::on_write(beast::error_code ec, std::size_t) {
    if (ec) {
        return fail(ec, "write");
    }
    outgoing.pop();
    if (!outgoing.empty()) {
        ws.async_write(net::buffer(*outgoing.front()),
                       beast::bind_front_handler(&trading_client::on_write,
                                                 shared_from_this()));
    }
}

void trading_client::do_close() {
    ws.async_close(websocket::close_code::normal,
                   beast::bind_front_handler(&trading_client::on_close,
                                             shared_from_this()));
}

void trading_client::on_close(beast::error_code ec) {
    if (ec) {
        return fail(ec, "close");
    }
    fmt::print("\r\033[K");
    fmt::println("TRADING_CLIENT: Websocket connection closed");
}

void trading_client::fail(beast::error_code ec, std::string what) {
    if (ec == net::error::operation_aborted || ec == websocket::error::closed)
        return;
    fmt::println("{}: {}", what, ec.message());
}
