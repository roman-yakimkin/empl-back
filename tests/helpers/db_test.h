//
// Created by Roman Yakimkin on 07.06.2026.
//

#pragma once

#include <string>
#include <mutex>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

inline std::mutex& get_db_mutex() {
    static std::mutex instance;
    return instance;
}

class db_test {
protected:
    std::unique_lock<std::mutex> db_lock;

    std::shared_ptr<spdlog::logger> test_logger;

    static std::string get_connection();

    virtual void prepare_data() = 0;
    virtual void drop_data() = 0;

    virtual void set_up();
    virtual void tear_down();
public:
    virtual ~db_test() = default;
};
