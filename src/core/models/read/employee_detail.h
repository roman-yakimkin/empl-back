//
// Created by Roman Yakimkin on 01.06.2026.
//

#pragma once

#include "core/models/department.h"
#include "core/models/position.h"

namespace core {
    struct employee_detail {
        int id = 0;
        std::string name;

        department dep;
        position pos;

        int64_t created_at = 0;
        int64_t updated_at = 0;

        employee_detail() = default;
        employee_detail(int _id, std::string _name,
                        department _dep, position _pos,
                        int64_t _created_at = 0, int64_t _updated_at = 0) :
            id(_id), name(std::move(_name)),
            dep(std::move(_dep)), pos(std::move(_pos)),
            created_at(_created_at), updated_at(_updated_at) {}

        bool operator==(const employee_detail& another) const {
            return id == another.id &&
                   name == another.name &&
                   dep == another.dep &&
                   pos == another.pos;
        }

        friend std::ostream& operator<<(std::ostream& os, const employee_detail& e) {
            return os << "employee{id=" << e.id
                      << ", name=\"" << e.name
                      << "\", department=" << e.dep
                      << ", position=" << e.pos
                      << "}";
        }

    };
}
