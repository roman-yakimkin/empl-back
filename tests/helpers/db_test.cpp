//
// Created by Roman Yakimkin on 07.06.2026.
//

#include "db_test.h"
#include "helpers/envs.h"

std::string db_test::get_connection() {
    return "dbname=" + helpers::get_env("DB_NAME", "empldb_test") +
           " host=" + helpers::get_env("DB_HOST", "localhost") +
           " port=" + helpers::get_env("DB_PORT", "5432") +
           " user=" + helpers::get_env("DB_USER", "emplcpp") +
           " password=" + helpers::get_env("DB_PASSWORD", "emplcpp");
};

void db_test::set_up() {
    test_logger = spdlog::stdout_color_mt("test_logger");
    test_logger->set_level(spdlog::level::debug);

    prepare_data();
}

void db_test::tear_down() {
    drop_data();

    spdlog::drop("test_logger");
}

