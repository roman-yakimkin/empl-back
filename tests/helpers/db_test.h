//
// Created by Roman Yakimkin on 07.06.2026.
//

#pragma once

#include <string>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

inline std::mutex db_mutex;

class db_test {
protected:
    std::shared_ptr<spdlog::logger> test_logger;

    static std::string get_connection();

    virtual void prepare_data() = 0;
    virtual void drop_data() = 0;

    virtual void set_up();
    virtual void tear_down();
public:
    virtual ~db_test() = default;
};
