//
// Created by Roman Yakimkin on 30.05.2026.
//

#include "repo_test.h"

void repo_test::set_up() {
    db_test::set_up();
    db_manager = std::make_unique<infra::db_manager>(get_connection(), test_logger.get());
}

void repo_test::tear_down() {
    db_test::tear_down();
    db_manager.reset();
}


