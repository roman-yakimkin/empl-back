//
// Created by Roman Yakimkin on 21.05.2026.
//

#pragma once

#include <spdlog/logger.h>

#include "core/contracts/i_department_repo.h"
#include "core/contracts/i_department_svc.h"

namespace svc {
    class department_service : public core::i_department_svc {
        core::i_department_repo* repo;
        spdlog::logger* logger;
    public:
        explicit department_service(core::i_department_repo* _repo, spdlog::logger* _logger) : repo(_repo), logger(_logger) {
            if (_repo == nullptr) throw std::invalid_argument("department repo cannot be null");
            if (_logger == nullptr) throw std::invalid_argument("logger cannot be null");
        }

        [[nodiscard]] std::optional<core::department> get_one(int id) const override;
        [[nodiscard]] std::map<int, core::department> get_all() const override;
        core::department create(const core::department& dep) override;
        std::optional<core::department> update(int id, const core::department& dep) override;
        bool del(int id) override;
    };
} // svc