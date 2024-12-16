#include "helper.hpp"
#include <fmt/base.h>
#include <fmt/format.h>
#include <iostream>

void print_console(std::shared_ptr<std::string> message) {
    fmt::print("\r\033[K");
    fmt::println("{}", *message);
    fmt::print("> ");
    std::cout.flush();
}

void print_console(std::string message) {
    fmt::print("\r\033[K");
    fmt::println("{}", message);
    fmt::print("> ");
    std::cout.flush();
}

void print_response(std::string &des) {
    json msg = json::parse(des);
    if (msg["id"] == 1) {
        print_auth_response(des);
    } else if (msg["id"] == 2) {
        print_place_order_response(des);
    } else if (msg["id"] == 3) {
        print_cancel_order_response(des);
    } else if (msg["id"] == 4) {
        print_edit_order_response(des);
    } else if (msg["id"] == 5) {
        print_subscribe_response(des);
    } else if (msg["id"] == 6) {
        print_unsubscribe_response(des);
    } else if (msg["id"] == 7) {
        print_unsubscribe_all_response(des);
    } else {
        print_console(des);
    }
}

void print_error(json &msg, std::string what) {
    print_console(fmt::format(
        "{}: {}", what, msg["error"]["message"].template get<std::string>()));
}

void make_auth_json_message(std::string &id, std::string &secret, json &msg) {
    json ser = {{"jsonrpc", "2.0"},
                {"id", 1},
                {"method", "public/auth"},
                {"params",
                 {{"grant_type", "client_credentials"},
                  {"client_id", id},
                  {"client_secret", secret}}}};
    msg = ser;
}

void print_auth_response(std::string &des) {
    json msg = json::parse(des);
    if (msg.contains("error")) {
        print_error(msg, "Auth error");
    } else {
        print_console("Successfully authenticated");
    }
}

void make_place_order_json_message(std::string &method,
                                   std::string &instrument_name, int &amount,
                                   int &price, json &msg) {
    json ser = {
        {"jsonrpc", "2.0"},
        {"id", 2},
        {"method", "private/" + method},
        {"params", {{"instrument_name", instrument_name}, {"type", "market"}}}};
    if (amount != -1) {
        ser["params"]["amount"] = amount;
    }
    if (price != -1) {
        ser["params"]["price"] = price;
    }
    msg = ser;
}

void print_place_order_response(std::string &des) {
    json msg = json::parse(des);
    if (msg.contains("error")) {
        print_error(msg, "Error in placing order");
    } else {
        print_console(fmt::format(
            "Successfully placed order. Order ID: {}",
            msg["result"]["order"]["order_id"].template get<std::string>()));
    }
}

void make_cancel_order_json_message(std::string &order_id, json &msg) {
    json ser = {{"jsonrpc", "2.0"},
                {"id", 3},
                {"method", "private/cancel"},
                {"params", {{"order_id", order_id}}}};
    msg = ser;
}

void print_cancel_order_response(std::string &des) {
    json msg = json::parse(des);
    if (msg.contains("error")) {
        print_error(msg, "Error in canceling order");
    } else {
        print_console("Successfully canceled order.");
    }
}

void make_edit_order_json_message(std::string &order_id, int &amount,
                                  int &price, json &msg) {
    json ser = {{"jsonrpc", "2.0"},
                {"id", 4},
                {"method", "private/edit"},
                {"params", {{"order_id", order_id}, {"advanced", "implv"}}}};
    if (amount != -1) {
        ser["params"]["amount"] = amount;
    }
    if (price != -1) {
        ser["params"]["price"] = price;
    }
    msg = ser;
}

void print_edit_order_response(std::string &des) {
    json msg = json::parse(des);
    if (msg.contains("error")) {
        print_error(msg, "Error in editing order");
    } else {
        print_console("Successfully edited order.");
    }
}

void make_subscribe_json_message(std::string &type,
                                 std::vector<std::string> &channels,
                                 json &msg) {
    json ser = {{"jsonrpc", "2.0"},
                {"id", 5},
                {"method", type + "/subscribe"},
                {"params", {{"channels", channels}}}};
    msg = ser;
}

void print_subscribe_response(std::string &des) {
    json msg = json::parse(des);
    if (msg.contains("error")) {
        print_error(msg, "Subscription failed");
    } else {
        if (msg["result"].size() > 0) {
            print_console(
                fmt::format("Subscribed to {}",
                            msg["result"][0].template get<std::string>()));
        } else {
            print_console("Invalid channel.");
        }
    }
}

void make_unsubscribe_json_message(std::string &type,
                                   std::vector<std::string> &channels,
                                   json &msg) {
    json ser = {{"jsonrpc", "2.0"},
                {"id", 6},
                {"method", type + "/unsubscribe"},
                {"params", {{"channels", channels}}}};
    msg = ser;
}

void print_unsubscribe_response(std::string &des) {
    json msg = json::parse(des);
    if (msg.contains("error")) {
        print_error(msg, "Failed to unsubscribe");
    } else {
        if (msg["result"].size() > 0) {
            print_console(
                fmt::format("Unsubscribed from {}",
                            msg["result"][0].template get<std::string>()));
        } else {
            print_console("Invalid channel.");
        }
    }
}

void make_unsubscribe_all_json_message(std::string &type, json &msg) {
    json ser = {{"jsonrpc", "2.0"},
                {"id", 7},
                {"method", type + "/unsubscribe_all"},
                {"params", {}}};
    msg = ser;
}

void print_unsubscribe_all_response(std::string &des) {
    json msg = json::parse(des);
    if (msg.contains("error")) {
        print_error(msg, "Failed to unsubscribe all");
    } else {
        print_console("Unsubscribed from all.");
    }
}

void make_get_orderbook_json_message(json &msg) {
    json ser = {
        {"jsonrpc", "2.0"},
        {"id", 8},
        {"method", "public/get_order_book"},
        {"params", {{"instrument_name", "BTC-PERPETUAL"}, {"depth", 5}}}};
    msg = ser;
}

void make_get_positions_json_message(std::string &currency, std::string &kind,
                                     json &msg) {
    if (currency == "") {
        currency = "any";
    }
    json ser = {{"jsonrpc", "2.0"},
                {"id", 8},
                {"method", "private/get_positions"},
                {"params", {{"currency", currency}}}};
    if (kind != "") {
        ser["params"]["kind"] = kind;
    }
    msg = ser;
}

void make_hello_json_message(json &msg) {
    json ser = {
        {"jsonrpc", "2.0"},
        {"id", 2841},
        {"method", "public/hello"},
        {"params", {{"client_name", "Harshad"}, {"client_version", "1.0.2"}}}};
    msg = ser;
}
