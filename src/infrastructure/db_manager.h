//
// Created by Roman Yakimkin on 24.05.2026.
//

#pragma once

#include <string>

namespace spdlog { class logger; }
namespace pqxx { class connection; }

namespace infra {
    class db_manager {
        std::string conn_string;
        spdlog::logger* logger;
    public:
        explicit db_manager(const std::string& _conn_string, spdlog::logger* _logger);

        pqxx::connection& get_connection();
        void reset_connection();
    };
} // infra