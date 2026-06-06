//
// Created by Roman Yakimkin on 20.05.2026.
//

#pragma once

#include <string>
#include <vector>
#include <map>
#include <nlohmann/json.hpp>

#include "core/models/department.h"

namespace api {
    struct department_request {
        std::string name;
    };

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(department_request, name);

    struct department_response {
        int id;
        std::string name;

        explicit department_response(const core::department &d): id(d.id), name(d.name) {};
    };

    inline void to_json(nlohmann::json& j, const department_response& dto) {
        j = {
            {"id", dto.id},
            {"name", dto.name}
        };
    }

    inline void from_json(const nlohmann::json& j, department_response& dto) {
        dto.id = j.at("id").get<int>();
        dto.name = j.at("name").get<std::string>();
    }

    struct department_response_list {
        std::vector<department_response> items;

        explicit department_response_list(const std::map<int, core::department> &dl) {
            items.reserve(dl.size());

            for (const auto&[id, elem]: dl) items.emplace_back(elem);
        }
    };

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(department_response_list, items);

}

