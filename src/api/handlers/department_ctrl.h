//
// Created by Roman Yakimkin on 21.05.2026.
//

#pragma once

#include <spdlog/logger.h>

#include "crow.h"
#include "service/department_service.h"

namespace api {
    class department_ctrl {
        core::i_department_svc* dep_svc;
        spdlog::logger* logger;
    public:
        department_ctrl() = delete;
        explicit department_ctrl(core::i_department_svc* _dep_svc, spdlog::logger* _logger) :
            dep_svc(_dep_svc), logger(_logger) {
            if (_dep_svc == nullptr) throw std::invalid_argument("dep_svc cannot be null");
            if (_logger == nullptr) throw std::invalid_argument("logger cannot be null");
        }

        void get_all(const crow::request& req, crow::response& resp) const;
        void get_one(const crow::request& req, crow::response& resp, int id) const;
        void add(const crow::request& req, crow::response& resp) const;
        void edit(const crow::request& req, crow::response& resp, int id) const;
        void del(const crow::request& req, crow::response& resp, int id) const;
    };
}

