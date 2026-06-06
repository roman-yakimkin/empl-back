//
// Created by Roman Yakimkin on 03.06.2026.
//

#pragma once

#include <vector>

#include "core/models/employee.h"
#include "core/models/read/employee_detail.h"
#include "core/filters/employee.h"

namespace core {
    class i_employee_svc {
    public:
        virtual ~i_employee_svc() = default;

        [[nodiscard]] virtual std::optional<employee_detail> get_one(int id) const = 0;
        [[nodiscard]] virtual std::vector<employee_detail> get_all(const filters::employee& params) const = 0;
        virtual employee create(const employee& e) = 0;
        virtual std::optional<employee> update(int id, const employee& e) = 0;
        virtual bool del(int id) = 0;
    };
} // core