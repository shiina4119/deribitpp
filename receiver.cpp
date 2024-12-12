#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/core/make_printable.hpp>
#include <boost/beast/websocket.hpp>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>

namespace beast = boost::beast;
namespace websocket = beast::websocket;
namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;

void fail(beast::error_code ec, std::string what) {
    if (ec == net::error::operation_aborted || ec == websocket::error::closed)
        return;
    std::cerr << what << ": " << ec.message() << "\n";
}

class receiver_client : public std::enable_shared_from_this<receiver_client> {
    tcp::resolver resolver;
    websocket::stream<beast::tcp_stream> ws;
    beast::flat_buffer buffer;
    std::string host;

public:
    explicit receiver_client(net::io_context &ioc)
        : resolver(net::make_strand(ioc)), ws(net::make_strand(ioc)) {}
    void run(std::string host, std::string port) {
        receiver_client::host = host;
        resolver.async_resolve(
            host, port,
            beast::bind_front_handler(&receiver_client::on_resolve,
                                      shared_from_this()));
    }

    void on_resolve(beast::error_code ec, tcp::resolver::results_type results) {
        if (ec) {
            return fail(ec, "resolve");
        }
        beast::get_lowest_layer(ws).expires_after(std::chrono::seconds(30));
        beast::get_lowest_layer(ws).async_connect(
            results, beast::bind_front_handler(&receiver_client::on_connect,
                                               shared_from_this()));
    }

    void on_connect(beast::error_code ec,
                    tcp::resolver::results_type::endpoint_type ep) {
        if (ec) {
            return fail(ec, "connect");
        }
        beast::get_lowest_layer(ws).expires_never();
        ws.set_option(websocket::stream_base::timeout::suggested(
            beast::role_type::client));
        host += ':' + std::to_string(ep.port());
        ws.async_handshake(
            host, "/",
            beast::bind_front_handler(&receiver_client::on_handshake,
                                      shared_from_this()));
    }

    void on_handshake(beast::error_code ec) {
        if (ec) {
            return fail(ec, "handshake");
        }
        do_read();
    }

    void do_read() {
        ws.async_read(buffer,
                      beast::bind_front_handler(&receiver_client::on_read,
                                                shared_from_this()));
    }

    void on_read(beast::error_code ec, std::size_t bytes_transferred) {
        boost::ignore_unused(bytes_transferred);
        if (ec) {
            return fail(ec, "read");
        }
        std::cout << beast::make_printable(buffer.data()) << std::endl;
        buffer.consume(buffer.size());
        do_read();
    }

    void do_close() {
        ws.async_close(websocket::close_code::normal,
                       beast::bind_front_handler(&receiver_client::on_close,
                                                 shared_from_this()));
    }

    void on_close(beast::error_code ec) {
        if (ec) {
            return fail(ec, "close");
        }
        std::cout << "Receiver connection closed";
    }
};

int main() {
    net::io_context ioc;
    std::make_shared<receiver_client>(ioc)->run("0.0.0.0", "8090");
    ioc.run();
}
