//
// Created by Roman Yakimkin on 30.05.2026.
//

#pragma once

#include <string>
#include <memory>

#include "infrastructure/db_manager.h"
#include "repository/department_repo.h"
#include <spdlog/spdlog.h>

class repo_test {
protected:
    std::shared_ptr<spdlog::logger> test_logger;
    std::unique_ptr<infra::db_manager> db_manager;

    static std::string get_connection();
    void set_up();
    void tear_down();
    virtual void prepare_data() = 0;
    virtual void drop_data() = 0;
public:
    repo_test() = default;
    virtual ~repo_test() = default;
};
