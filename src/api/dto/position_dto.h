//
// Created by Roman Yakimkin on 20.05.2026.
//

#pragma once

#include <string>
#include <vector>
#include <map>
#include <nlohmann/json.hpp>

#include "core/models/position.h"

namespace api {

    struct position_request {
        std::string name;
    };

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(position_request, name);

    struct position_response {
        int id = 0;
        std::string name;

        explicit position_response(const core::position& p): id(p.id), name(p.name) {}
    };

    inline void to_json(nlohmann::json& j, const position_response& dto) {
        j = {
            {"id", dto.id},
            {"name", dto.name}
        };
    }

    inline void from_json(const nlohmann::json& j, position_response& dto) {
        dto.id = j.at("id").get<int>();
        dto.name = j.at("name").get<std::string>();
    }

    struct position_response_list {
        std::vector<position_response> items;

        explicit position_response_list(const std::map<int, core::position>& pl) {
            items.reserve(pl.size());

            for (const auto&[id, elem]: pl) items.emplace_back(elem);
        }
    };

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(position_response_list, items);

}

