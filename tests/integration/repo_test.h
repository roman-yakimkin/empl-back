//
// Created by Roman Yakimkin on 30.05.2026.
//

#pragma once

#include <memory>

#include "infrastructure/db_manager.h"

#include "helpers/db_test.h"

class repo_test :public db_test{
protected:
    std::unique_ptr<infra::db_manager> db_manager;

    void set_up() override;
    void tear_down() override;
};
