#ifndef _SHARED_STATE_H
#define _SHARED_STATE_H

#include "listener.hpp"
#include <memory>
#include <mutex>
#include <queue>
#include <string>

class shared_state {
    std::queue<std::shared_ptr<std::string>> incoming_messages;
    std::shared_ptr<listener> li;
    bool is_processing{true};

public:
    shared_state(std::shared_ptr<listener>);
    std::mutex mu;
    void process_incoming();
    void stop_processing();
    void enqueue(std::shared_ptr<std::string>);
};

#endif
