//
// Created by Roman Yakimkin on 01.06.2026.
//

#pragma once

#include <spdlog/logger.h>

#include "core/contracts/i_position_repo.h"
#include "core/contracts/i_position_svc.h"

namespace svc {
    class position_service : public core::i_position_svc {
        core::i_position_repo* repo;
        spdlog::logger* logger;
    public:
        explicit position_service(core::i_position_repo* _repo, spdlog::logger* _logger) : repo(_repo), logger(_logger) {
            if (_repo == nullptr) throw std::invalid_argument("position repo cannot be null");
            if (_logger == nullptr) throw std::invalid_argument("logger cannot be null");
        }

        [[nodiscard]] std::optional<core::position> get_one(int id) const override;
        [[nodiscard]] std::map<int, core::position> get_all() const override;
        core::position create(const core::position& dep) override;
        std::optional<core::position> update(int id, const core::position& dep) override;
        bool del(int id) override;
    };
} // svc