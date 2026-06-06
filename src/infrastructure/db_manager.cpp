//
// Created by Roman Yakimkin on 24.05.2026.
//

#include "db_manager.h"

#include <pqxx/pqxx>
#include <spdlog/logger.h>

namespace infra {
    db_manager::db_manager(const std::string &_conn_string, spdlog::logger* _logger) {
        if (_logger == nullptr) throw std::invalid_argument("logger cannot be null");

        conn_string = _conn_string;
        logger = _logger;
        try {
            pqxx::connection test_conn(conn_string);
            _logger->info("Database connection verified successfully.");
        }
        catch (const std::exception& e) {
            _logger->critical("Failed to verify DB connection: {}", e.what());
            throw;
        }
    }

    pqxx::connection& db_manager::get_connection() {
        thread_local std::unique_ptr<pqxx::connection> conn;

        if (!conn || !conn->is_open()) {
            try {
                conn = std::make_unique<pqxx::connection>(conn_string);
            } catch (const pqxx::broken_connection& e) {
                logger->error("Failed to create DB connection: {}", e.what());
                throw;
            }
        }

        return *conn;
    }

    void db_manager::reset_connection() {
        thread_local std::unique_ptr<pqxx::connection> conn;
        conn.reset();
        logger->debug("DB connection reset for thread");
    }
} // infra