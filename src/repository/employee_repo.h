//
// Created by Roman Yakimkin on 02.06.2026.
//

#pragma once

#include <stdexcept>
#include <spdlog/logger.h>
#include <pqxx/pqxx>

#include "infrastructure/db_manager.h"
#include "core/contracts/i_employee_repo.h"

namespace repo {
    class employee_repo : public core::i_employee_repo {
        infra::db_manager* db;
        spdlog::logger* logger;

        std::string build_raw_query() const;
        void build_filtered_query(const core::filters::employee& filter, std::string& query, pqxx::params& params) const;
        core::employee_detail detail_from_db_row(const pqxx::row& row) const;
        core::employee from_db_row(const pqxx::row& row) const;

    public:
        explicit employee_repo(infra::db_manager* _db, spdlog::logger* _logger) :
            db(_db), logger(_logger) {
            if (_db == nullptr) throw std::invalid_argument("db cannot be null");
            if (_logger == nullptr) throw std::invalid_argument("logger cannot be null");
        };

        std::optional<core::employee_detail> get_one(int id) const override;
        std::vector<core::employee_detail> get_all(const core::filters::employee& filter) const override;
        core::employee create(const core::employee &e) override;
        std::optional<core::employee> update(int id, const core::employee &e) override;
        bool del(int id) override;
    };
} // repo