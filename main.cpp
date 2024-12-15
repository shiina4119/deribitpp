#include "broadcast_session.hpp"
#include "helper.hpp"
#include "listener.hpp"
#include "root_cert.hpp"
#include "trading_client.hpp"
#include <boost/asio/ip/address.hpp>
#include <boost/asio/ip/basic_endpoint.hpp>
#include <boost/asio/ip/detail/endpoint.hpp>
#include <fmt/base.h>
#include <iostream>
#include <memory>
#include <nlohmann/json.hpp>
#include <thread>

using json = nlohmann::json;

int main() {
    std::string command;
    std::string msg_dump;
    json msg;

    net::io_context listener_ioc;
    auto li = std::make_shared<listener>(
        listener_ioc, tcp::endpoint(net::ip::make_address("0.0.0.0"), 8090));
    li->run();

    auto ss = std::make_shared<shared_state>(li);

    net::io_context client_ioc;
    ssl::context ctx{ssl::context::tlsv12_client};
    load_root_certificates(ctx);
    auto cl = std::make_shared<trading_client>(
        client_ioc, ctx, "test.deribit.com", "443", "/ws/api/v2", ss);
    cl->establish_connection();

    std::thread listener_thread([&listener_ioc]() { listener_ioc.run(); });
    std::thread client_ioc_thread([&client_ioc]() { client_ioc.run(); });
    std::thread processing_thread([&ss]() { ss->process_incoming(); });

    while (true) {
        std::getline(std::cin, command);
        if (command == "auth") {
            std::string client_id, client_secret;
            fmt::print("Client ID: ");
            std::getline(std::cin, client_id);
            fmt::print("Client secret: ");
            std::getline(std::cin, client_secret);
            make_auth_json_message(client_id, client_secret, msg);
            msg_dump = msg.dump();
        } else if (command == "place order") {
            std::string method, instrument_name, amount_str, price_str;
            fmt::print("Type? (buy/sell): ");
            std::getline(std::cin, method);
            fmt::print("Instrument Name: ");
            std::getline(std::cin, instrument_name);
            fmt::print("Amount: ", amount_str);
            std::getline(std::cin, amount_str);
            fmt::print("Price: ", price_str);
            std::getline(std::cin, price_str);
            int amount = (amount_str == "" ? -1 : stoi(amount_str));
            int price = (price_str == "" ? -1 : stoi(price_str));
            make_place_order_json_message(method, instrument_name, amount,
                                          price, msg);
            msg_dump = msg.dump();
        } else if (command == "cancel order") {
            std::string order_id;
            fmt::print("Order ID: ");
            std::getline(std::cin, order_id);
            make_cancel_order_json_message(order_id, msg);
            msg_dump = msg.dump();
        } else if (command == "edit order") {
            std::string order_id, amount_str, price_str;
            fmt::print("Order ID: ");
            std::getline(std::cin, order_id);
            fmt::print("Amount: ");
            std::getline(std::cin, amount_str);
            fmt::print("Price: ");
            std::getline(std::cin, price_str);
            int amount = (amount_str == "" ? -1 : stoi(amount_str));
            int price = (price_str == "" ? -1 : stoi(price_str));
            make_edit_order_json_message(order_id, amount, price, msg);
            msg_dump = msg.dump();
        } else if (command == "subscribe") {
            std::string type;
            std::vector<std::string> channels;
            std::string channel;
            fmt::print("Type? (public/private): ");
            std::getline(std::cin, type);
            fmt::print("Channel: ");
            std::getline(std::cin, channel);
            channels.push_back(channel);
            make_subscribe_json_message(type, channels, msg);
            msg_dump = msg.dump();
        } else if (command == "unsubscribe") {
            std::string type;
            std::vector<std::string> channels;
            std::string channel;
            fmt::print("Type? (public/private): ");
            std::getline(std::cin, type);
            fmt::print("Channel: ");
            std::getline(std::cin, channel);
            channels.push_back(channel);
            make_unsubscribe_json_message(type, channels, msg);
            msg_dump = msg.dump();
        } else if (command == "unsubscribe all") {
            std::string type;
            fmt::print("Type? (public/private): ");
            std::getline(std::cin, type);
            make_unsubscribe_all_json_message(type, msg);
            msg_dump = msg.dump();
        } else if (command == "get orderbook") {
            make_get_orderbook_json_message(msg);
            msg_dump = msg.dump();
        } else if (command == "get positions") {
            std::string currency, kind;
            fmt::print("Currency (leave blank for any): ");
            std::getline(std::cin, currency);
            fmt::print("Kind: ");
            std::getline(std::cin, kind);
            make_get_positions_json_message(currency, kind, msg);
            msg_dump = msg.dump();
        } else if (command == "quit") {
            cl->do_close();
            ss->stop_processing();
            li->stop();
            break;
        } else {
            print_console("Invalid command");
            continue;
        }
        cl->do_write(std::make_shared<std::string const>(std::move(msg_dump)));
    }
    if (client_ioc_thread.joinable())
        client_ioc_thread.join();
    if (processing_thread.joinable())
        processing_thread.join();
    if (listener_thread.joinable())
        listener_thread.join();
}
