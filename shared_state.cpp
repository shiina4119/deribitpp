#include "shared_state.hpp"
#include "helper.hpp"
#include <fmt/base.h>
#include <memory>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

shared_state::shared_state(std::shared_ptr<listener> li) : li(li) {}

void shared_state::process_incoming() {
    while (true) {
        std::unique_lock<std::mutex> lock(mu);
        if ((is_processing || incoming_messages.size()) == false) {
            lock.unlock();
            break;
        }
        if (incoming_messages.empty()) {
            lock.unlock();
            continue;
        }
        auto message = incoming_messages.front();
        incoming_messages.pop();
        lock.unlock();
        json des = json::parse(*message);
        std::string method = "";
        try {
            method = des.at("method");
        } catch (json::exception) {
            ;
        }
        if (method == "subscription") {
            if (li->get_session() != nullptr) {
                li->get_session()->do_write(message);
            }
        } else {
            print_console(*message);
        }
    }
}

void shared_state::stop_processing() { is_processing = false; }

void shared_state::enqueue(std::shared_ptr<std::string> message) {
    std::lock_guard<std::mutex> lock(mu);
    incoming_messages.emplace(message);
}
