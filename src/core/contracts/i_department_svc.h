//
// Created by Roman Yakimkin on 26.05.2026.
//

#pragma once

#include "core/models/department.h"

namespace core {
    class i_department_svc {
    public:
        virtual ~i_department_svc() = default;

        [[nodiscard]] virtual std::optional<department> get_one(int id) const = 0;
        [[nodiscard]] virtual std::map<int, department> get_all() const = 0;
        virtual department create(const department& d) = 0;
        virtual std::optional<department> update(int id, const department& d) = 0;
        virtual bool del(int id) = 0;
    };
} // core