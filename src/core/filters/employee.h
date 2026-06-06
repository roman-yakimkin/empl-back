//
// Created by Roman Yakimkin on 01.06.2026.
//

#pragma once

#include <optional>
#include <string>

namespace core::filters {
    struct employee {
        std::optional<int> department_id;
        std::optional<int> position_id;
        std::optional<std::string> name_pattern;
    };
}
