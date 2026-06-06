//
// Created by Roman Yakimkin on 21.05.2026.
//

#pragma once

#include <optional>
#include <map>

#include "core/models/position.h"

namespace core {
    class i_position_repo {
    public:
        virtual ~i_position_repo() = default;

        virtual std::optional<position> get_one(int id) const = 0;
        virtual std::map<int, position> get_all() const = 0;
        virtual position create(const position& d) = 0;
        virtual std::optional<position> update(int id, const position& d) = 0;
        virtual bool del(int id) = 0;
    };
}

