//
// Created by Roman Yakimkin on 21.05.2026.
//

#pragma once

#include <optional>
#include <map>

#include "core/models/department.h"

namespace core {
    class i_department_repo {
    public:
        virtual ~i_department_repo() = default;

        virtual std::optional<department> get_one(int id) const = 0;
        virtual std::map<int, department> get_all() const = 0;
        virtual department create(const department& d) = 0;
        virtual std::optional<department> update(int id, const department& d) = 0;
        virtual bool del(int id) = 0;
    };
}

