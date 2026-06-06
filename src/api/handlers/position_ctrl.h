//
// Created by Roman Yakimkin on 01.06.2026.
//

#pragma once

#include <spdlog/logger.h>
#include "crow.h"

#include "core/contracts/i_position_svc.h"

namespace api {
    class position_ctrl {
        core::i_position_svc* pos_svc;
        spdlog::logger* logger;
    public:
        position_ctrl() = delete;
        explicit position_ctrl(core::i_position_svc* _pos_svc, spdlog::logger* _logger) :
            pos_svc(_pos_svc), logger(_logger) {
            if (_pos_svc == nullptr) throw std::invalid_argument("pos_svc cannot be null");
            if (_logger == nullptr) throw std::invalid_argument("logger cannot be null");
        }

        void get_all(const crow::request& req, crow::response& resp) const;
        void get_one(const crow::request& req, crow::response& resp, int id) const;
        void add(const crow::request& req, crow::response& resp) const;
        void edit(const crow::request& req, crow::response& resp, int id) const;
        void del(const crow::request& req, crow::response& resp, int id) const;
    };
} // api