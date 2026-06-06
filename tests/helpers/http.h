//
// Created by Roman Yakimkin on 31.05.2026.
//

#pragma once

#include <string>
#include "crow.h"
#include "nlohmann/json.hpp"

crow::request make_request(const std::string& url = "/",
                           const std::string& body = "",
                           crow::HTTPMethod method = crow::HTTPMethod::Get
                           );

inline nlohmann::json parse_response(const crow::response& resp) {
    return nlohmann::json::parse(resp.body, nullptr, false);
}
