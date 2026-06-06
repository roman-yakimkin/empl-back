//
// Created by Roman Yakimkin on 30.05.2026.
//

#include "repo_test.h"
#include "helpers/envs.h"
#include <spdlog/sinks/stdout_color_sinks.h>

std::string repo_test::get_connection() {
    return "dbname=" + helpers::get_env("DB_NAME", "empldb_test") +
           " host=" + helpers::get_env("DB_HOST", "localhost") +
           " port=" + helpers::get_env("DB_PORT", "5432") +
           " user=" + helpers::get_env("DB_USER", "emplcpp") +
           " password=" + helpers::get_env("DB_PASSWORD", "emplcpp");
}

void repo_test::set_up() {
    test_logger = spdlog::stdout_color_mt("test_logger");
    test_logger->set_level(spdlog::level::debug);

    prepare_data();

    db_manager = std::make_unique<infra::db_manager>(get_connection(), test_logger.get());
}

void repo_test::tear_down() {
    drop_data();

    db_manager.reset();
    spdlog::drop("test_logger");
}


