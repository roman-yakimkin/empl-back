//
// Created by Roman Yakimkin on 03.06.2026.
//

#pragma once

#include <spdlog/logger.h>
#include <vector>
#include <optional>

#include "core/contracts/i_employee_svc.h"
#include "core/contracts/i_employee_repo.h"
#include "core/models/read/employee_detail.h"

namespace svc {
    class employee_service : public core::i_employee_svc{
        core::i_employee_repo* repo;
        spdlog::logger* logger;

        void check_employee(const core::employee& e) const;
    public:
        explicit employee_service(core::i_employee_repo* _repo, spdlog::logger* _logger) : repo(_repo), logger(_logger) {
            if (_repo == nullptr) throw std::invalid_argument("employee repo cannot be null");
            if (_logger == nullptr) throw std::invalid_argument("logger cannot be null");
        }

        [[nodiscard]] std::optional<core::employee_detail> get_one(int id) const override;
        [[nodiscard]] std::vector<core::employee_detail> get_all(const core::filters::employee& params) const override;
        core::employee create(const core::employee& e) override;
        std::optional<core::employee> update(int id, const core::employee& e) override;
        bool del(int id) override;
    };
} // svc