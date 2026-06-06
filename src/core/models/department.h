//
// Created by Roman Yakimkin on 20.05.2026.
//

#pragma once

#include <string>
#include <ostream>
#include <utility>

namespace core {
    struct department {
        int id = 0;
        std::string name;

        department() = default;
        department(const int _id, std::string _name): id(_id), name(std::move(_name)) {};

        bool operator==(const department& another) const {
            return id == another.id && name == another.name;
        }

        friend std::ostream& operator<<(std::ostream& os, const department& d) {
            return os << "department{id=" << d.id << ", name=\"" << d.name << "\"}";
        }
    };
}



