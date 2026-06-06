//
// Created by Roman Yakimkin on 01.06.2026.
//

#pragma once

#include <optional>
#include <vector>

#include "core/filters/employee.h"
#include "core/models/employee.h"
#include "core/models/read/employee_detail.h"

namespace core {
    class i_employee_repo {
    public:
        virtual ~i_employee_repo() = default;

        virtual std::optional<employee_detail> get_one(int id) const = 0;
        virtual std::vector<employee_detail> get_all(const filters::employee& params) const = 0;
        virtual employee create(const employee& e) = 0;
        virtual std::optional<employee> update(int id, const employee& d) = 0;
        virtual bool del(int id) = 0;
    };
} // core