#ifndef _LISTENER_H
#define _LISTENER_H

#include "broadcast_session.hpp"
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/websocket.hpp>
#include <cstdlib>
#include <memory>

namespace beast = boost::beast;
namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;

class listener : public std::enable_shared_from_this<listener> {
    net::io_context &ioc;
    tcp::acceptor acceptor;
    void do_accept();
    void on_accept(beast::error_code ec, tcp::socket socket);
    void fail(beast::error_code, std::string);
    std::shared_ptr<broadcast_session> session;

public:
    listener(net::io_context &ioc, tcp::endpoint endpoint);
    void run();
    void stop();
    std::shared_ptr<broadcast_session> get_session();
};

#endif
