//
// Created by Roman Yakimkin on 20.05.2026.
//
#pragma once

#include <string>
#include <ostream>

namespace core {
    struct position {
        int id = 0;
        std::string name;

        position() = default;
        position(const int _id, std::string _name) : id(_id), name(std::move(_name)) {}

        bool operator==(const position& another) const {
            return id == another.id && name == another.name;
        }

        friend std::ostream& operator<<(std::ostream& os, const position& d) {
            return os << "position{id=" << d.id << ", name=\"" << d.name << "\"}";
        }
    };
}
