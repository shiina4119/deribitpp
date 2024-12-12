#ifndef _TRADING_CLIENT_H
#define _TRADING_CLIENT_H

#include "shared_state.hpp"
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/ssl/ssl_stream.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <boost/beast/websocket/stream.hpp>
#include <cstdlib>
#include <memory>
#include <queue>
#include <string>

namespace beast = boost::beast;
namespace websocket = beast::websocket;
namespace net = boost::asio;
namespace ssl = boost::asio::ssl;
using tcp = boost::asio::ip::tcp;

class trading_client : public std::enable_shared_from_this<trading_client> {
    tcp::resolver resolver;
    websocket::stream<beast::ssl_stream<beast::tcp_stream>> ws;
    beast::flat_buffer buffer;
    std::string host;
    std::string port;
    std::string target;
    std::queue<std::shared_ptr<std::string const>> outgoing;
    std::shared_ptr<shared_state> ss;

public:
    explicit trading_client(net::io_context &, ssl::context &, std::string,
                            std::string, std::string,
                            std::shared_ptr<shared_state>);

    void establish_connection();
    void on_resolve(beast::error_code, tcp::resolver::results_type);
    void on_connect(beast::error_code,
                    tcp::resolver::results_type::endpoint_type);
    void on_ssl_handshake(beast::error_code);
    void on_handshake(beast::error_code);
    void do_write(std::shared_ptr<std::string const> const &);
    void on_write(beast::error_code, std::size_t);
    void do_read();
    void on_read(beast::error_code, std::size_t);
    void do_close();
    void on_close(beast::error_code);
    void fail(beast::error_code, std::string);
};

#endif
