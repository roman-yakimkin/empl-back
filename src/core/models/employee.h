//
// Created by Roman Yakimkin on 20.05.2026.
//

#pragma once

#include <string>
#include <ostream>

namespace core {
    struct employee {
        int id = 0;
        std::string name;
        int department_id = 0;
        int position_id = 0;
        int64_t created_at = 0;
        int64_t updated_at = 0;

        employee() = default;

        employee(std::string _name, int _dep_id, int _pos_id, int64_t _created_at = 0, int64_t _updated_at = 0) :
            name(std::move(_name)), department_id(_dep_id), position_id(_pos_id),
            created_at(_created_at), updated_at(_updated_at) {}

        employee(int _id, std::string _name, int _dep_id, int _pos_id, int64_t _created_at = 0, int64_t _updated_at = 0) :
            employee(std::move(_name), _dep_id, _pos_id, _created_at, _updated_at) {
            id = _id;
        }

        bool operator==(const employee& another) const {
            return id == another.id &&
                   name == another.name &&
                   department_id == another.department_id &&
                   position_id == another.position_id;
        }

        friend std::ostream& operator<<(std::ostream& os, const employee& e) {
            return os << "employee{id=" << e.id
                      << ", name=\"" << e.name
                      << "\", department_id=" << e.department_id
                      << ", position_id=" << e.position_id
                      << "}";
        }
    };
}
