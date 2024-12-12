#include "listener.hpp"
#include "helper.hpp"
#include <boost/asio/error.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core/error.hpp>
#include <fmt/base.h>
#include <memory>

listener::listener(net::io_context &ioc, tcp::endpoint endpoint)
    : ioc(ioc), acceptor(ioc) {
    beast::error_code ec;
    auto rc = acceptor.open(endpoint.protocol(), ec);
    if (ec) {
        fail(ec, "open");
        return;
    }
    rc = acceptor.set_option(net::socket_base::reuse_address(true), ec);
    if (ec) {
        fail(ec, "set_option");
        return;
    }
    rc = acceptor.bind(endpoint, ec);
    if (ec) {
        fail(ec, "bind");
        return;
    }
    rc = acceptor.listen(net::socket_base::max_listen_connections, ec);
    if (ec) {
        fail(ec, "listen");
        return;
    }
    print_console("LISTENER: listener object created");
    print_console("LISTENER: ready to accept incoming connection");
}

void listener::run() { do_accept(); }

void listener::do_accept() {
    acceptor.async_accept(
        net::make_strand(ioc),
        beast::bind_front_handler(&listener::on_accept, shared_from_this()));
}

void listener::on_accept(beast::error_code ec, tcp::socket socket) {
    print_console("LISTENER: accepted connection");
    if (ec) {
        fail(ec, "accept");
    } else {
        print_console("LISTENER: broadcast session created");
        session.reset();
        session = std::make_shared<broadcast_session>(std::move(socket));
        session->run();
    }
    do_accept();
}

void listener::stop() {
    session.reset();
    fmt::print("\r\033[K");
    fmt::println("LISTENER: acceptor closed");
    ioc.stop();
}

std::shared_ptr<broadcast_session> listener::get_session() { return session; }

void listener::fail(beast::error_code ec, std::string what) {
    if (ec == net::error::bad_descriptor)
        return;
    fmt::println("{}: {}", what, ec.message());
}
