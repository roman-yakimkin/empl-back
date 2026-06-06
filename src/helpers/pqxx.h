//
// Created by Roman Yakimkin on 02.06.2026.
//

#pragma once

#include <pqxx/pqxx>

#include <spdlog/spdlog.h>
#include "core/errors/validation_exception.h"

namespace helpers {
    core::validation_exception get_validation_exception(const pqxx::foreign_key_violation &ex, spdlog::logger* logger);
    int64_t parse_pg_timestamp_to_seconds(const std::string& ts_str);
}
