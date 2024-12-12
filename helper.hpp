#include <memory>
#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;

void print_console(std::shared_ptr<std::string> message);
void print_console(std::string message);

void print_response(std::string &);
void print_error(json &msg, std::string);

void make_auth_json_message(std::string &, std::string &, json &);
void print_auth_response(std::string &);

void make_place_order_json_message(std::string &, std::string &, int &, json &);
void print_place_order_response(std::string &);

void make_cancel_order_json_message(std::string &, json &);
void print_cancel_order_response(std::string &);

void make_edit_order_json_message(std::string &, int &, int &, json &);
void print_edit_order_response(std::string &);

void make_subscribe_json_message(std::string &, std::vector<std::string> &,
                                 json &);
void print_subscribe_response(std::string &);

void make_unsubscribe_json_message(std::string &, std::vector<std::string> &,
                                   json &);
void print_unsubscribe_response(std::string &);

void make_unsubscribe_all_json_message(std::string &, json &);
void print_unsubscribe_all_response(std::string &);

void make_get_orderbook_json_message(json &);
void print_get_orderbook_response(std::string &);

void make_get_positions_json_message(std::string &, std::string &, json &);
void print_get_positions_response(std::string &);
