//
// Created by Roman Yakimkin on 03.06.2026.
//

#pragma once

#include "crow.h"
#include "service/employee_service.h"

namespace api {
    class employee_ctrl {
        core::i_employee_svc* empl_svc;
        spdlog::logger* logger;
    public:
        employee_ctrl() = delete;
        explicit employee_ctrl(core::i_employee_svc* _empl_svc, spdlog::logger* _logger) :
            empl_svc(_empl_svc), logger(_logger) {
            if (_empl_svc == nullptr) throw std::invalid_argument("empl_svc cannot be null");
            if (_logger == nullptr) throw std::invalid_argument("logger cannot be null");
        }

        void get_all(const crow::request& req, crow::response& resp) const;
        void get_one(const crow::request& req, crow::response& resp, int id) const;
        void add(const crow::request& req, crow::response& resp) const;
        void edit(const crow::request& req, crow::response& resp, int id) const;
        void del(const crow::request& req, crow::response& resp, int id) const;
    };
} // api