//
// Created by Roman Yakimkin on 28.05.2026.
//

#pragma once

#include "crow.h"
#include "nlohmann/json.hpp"

namespace api::helpers {

    // Header-only, inline, в namespace слоя API
    inline void send_json(crow::response& resp, int status, const nlohmann::json& j = {}) {
        resp.code = status;
        if (!j.empty()) resp.body = j.dump();
        resp.set_header("Content-Type", "application/json");
        resp.end();
    }

    // Удобные обёртки для типовых случаев
    inline void send_ok(crow::response& resp, const nlohmann::json& body) {
        send_json(resp, 200, body);
        resp.end();
    }

    inline void send_error(crow::response& resp, int status, std::string_view message) {
        send_json(resp, status, {{"error", message}});
        resp.end();
    }

    inline void send_no_content(crow::response& resp, int status) {
        send_json(resp, status);
        resp.end();
    }

} // namespace api::helpers