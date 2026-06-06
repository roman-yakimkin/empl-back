//
// Created by Roman Yakimkin on 31.05.2026.
//

#pragma once

#include <pqxx/pqxx>
#include <spdlog/logger.h>

#include "infrastructure/db_manager.h"
#include "core/contracts/i_position_repo.h"

namespace repo {
    class position_repo : public core::i_position_repo{
        infra::db_manager* db;
        spdlog::logger* logger;
    public:
        explicit position_repo(infra::db_manager* _db, spdlog::logger* _logger) :
            db(_db), logger(_logger) {
            if (_db == nullptr) throw std::invalid_argument("db cannot be null");
            if (_logger == nullptr) throw std::invalid_argument("logger cannot be null");
        };

        std::optional<core::position> get_one(int id) const override;
        std::map<int, core::position> get_all() const override;
        core::position create(const core::position &p) override;
        std::optional<core::position> update(int id, const core::position &p) override;
        bool del(int id) override;

    };
} // repo