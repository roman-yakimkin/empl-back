//
// Created by Roman Yakimkin on 21.05.2026.
//

#pragma once

#include <spdlog/logger.h>

#include "infrastructure/db_manager.h"
#include "core/contracts/i_department_repo.h"

namespace repo {
    class department_repo : public core::i_department_repo {
        infra::db_manager* db;
        spdlog::logger* logger;
    public:
        explicit department_repo(infra::db_manager* _db, spdlog::logger* _logger) :
            db(_db), logger(_logger) {
            if (_db == nullptr) throw std::invalid_argument("db cannot be null");
            if (_logger == nullptr) throw std::invalid_argument("logger cannot be null");
        };

        std::optional<core::department> get_one(int id) const override;
        std::map<int, core::department> get_all() const override;
        core::department create(const core::department &d) override;
        std::optional<core::department> update(int id, const core::department &d) override;
        bool del(int id) override;
    };
} // repo