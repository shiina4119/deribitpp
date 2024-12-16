#ifndef _BROADCAST_SESSION_H
#define _BROADCAST_SESSION_H

#include <boost/beast/core/error.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/websocket/stream.hpp>
#include <cstddef>
#include <memory>

namespace beast = boost::beast;
namespace websocket = beast::websocket;
using tcp = boost::asio::ip::tcp;

/*
    A broadcast_session object denotes a websocket connection
    by a subscription receiver client.
    It will relay subscription messages to the receiver.
*/
class broadcast_session
    : public std::enable_shared_from_this<broadcast_session> {
    websocket::stream<beast::tcp_stream> ws;
    beast::flat_buffer buffer;

public:
    explicit broadcast_session(tcp::socket &&);
    void run();
    void on_run();
    void on_accept(beast::error_code);
    void do_write(std::shared_ptr<std::string>);
    void on_write(beast::error_code, std::size_t);
    void fail(beast::error_code, std::string);
};

#endif
