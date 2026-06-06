//
// Created by Roman Yakimkin on 31.05.2026.
//

#pragma once

#include <map>
#include <optional>

#include <core/models/position.h>

namespace core {
    class i_position_svc {
    public:
        virtual ~i_position_svc() = default;

        [[nodiscard]] virtual std::optional<position> get_one(int id) const = 0;
        [[nodiscard]] virtual std::map<int, position> get_all() const = 0;
        virtual position create(const position& d) = 0;
        virtual std::optional<position> update(int id, const position& d) = 0;
        virtual bool del(int id) = 0;
    };
} // core