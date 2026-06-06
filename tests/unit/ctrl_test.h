//
// Created by Roman Yakimkin on 31.05.2026.
//

#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/null_sink.h>

class ctrl_test {
protected:
    std::shared_ptr<spdlog::sinks::null_sink_mt> log_sink;
    std::shared_ptr<spdlog::logger> test_logger;

    void set_up();
    void tear_down();


public:
    ctrl_test() = default;
    virtual ~ctrl_test() = default;
};
