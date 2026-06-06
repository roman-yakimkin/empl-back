//
// Created by Roman Yakimkin on 20.05.2026.
//

#pragma once

#include <string>
#include <nlohmann/json.hpp>

#include "department_dto.h"
#include "position_dto.h"
#include "core/models/employee.h"
#include "core/models/read/employee_detail.h"

namespace api {

    struct employee_request {
        std::string name;
        int department_id;
        int position_id;
    };

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(employee_request, name, department_id, position_id);

    struct employee_response {
        int id;
        std::string name;
        int department_id;
        int position_id;
        int64_t created_at;
        int64_t updated_at;

        explicit employee_response(const core::employee& item) :
            id(item.id), name(item.name),
            department_id(item.department_id), position_id(item.position_id),
            created_at(item.created_at), updated_at(item.updated_at) {}
    };

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(employee_response, id, name, department_id, position_id, created_at, updated_at);

    struct employee_detail_response {
        int id;
        std::string name;
        department_response department;
        position_response position;
        int64_t created_at;
        int64_t updated_at;

        explicit employee_detail_response(const core::employee_detail& item) :
            id(item.id), name(item.name), department(item.dep), position(item.pos),
            created_at(item.created_at), updated_at(item.updated_at) {}
    };

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(employee_detail_response, id, name, department, position, created_at, updated_at);

    struct employee_detail_response_list {
        std::vector<employee_detail_response> items;

        explicit employee_detail_response_list(const std::vector<core::employee_detail>& vec) {
            items.reserve(vec.size());
            for (const auto& item: vec) {
                items.emplace_back(item);
            }
        }
    };

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(employee_detail_response_list, items);

}

