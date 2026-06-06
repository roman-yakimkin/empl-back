//
// Created by Roman Yakimkin on 21.05.2026.
//

#pragma once

#include "crow.h"
#include "department_ctrl.h"
#include "employee_ctrl.h"
#include "position_ctrl.h"

namespace api {
    class router {
        department_ctrl* dep_ctrl;
        position_ctrl* pos_ctrl;
        employee_ctrl* empl_ctrl;
    public:
        explicit router(department_ctrl* _dep_ctrl, position_ctrl* _pos_ctrl, employee_ctrl* _empl_ctrl) :
            dep_ctrl(_dep_ctrl), pos_ctrl(_pos_ctrl), empl_ctrl(_empl_ctrl) {
            if (_dep_ctrl == nullptr) throw std::invalid_argument("dep ctrl cannot be null");
            if (_pos_ctrl == nullptr) throw std::invalid_argument("pos ctrl cannot be null");
            if (_empl_ctrl == nullptr) throw std::invalid_argument("empl ctrl cannot be null");
        }

        void register_routes(crow::SimpleApp& app) const;
    };
}

