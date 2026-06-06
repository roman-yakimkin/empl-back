//
// Created by Roman Yakimkin on 02.06.2026.
//

#include "pqxx.h"
#include <spdlog/logger.h>
#include <string>

core::validation_exception helpers::get_validation_exception(const pqxx::foreign_key_violation &ex, spdlog::logger* logger) {
    if (logger) logger->warn("Foreign key violation: {}", ex.what());

    std::string_view err_msg = ex.what();
    std::string field_name = "unknown";

    if (err_msg.find("fk_department") != std::string_view::npos) {
        field_name = "department_id";
    } else if (err_msg.find("fk_position") != std::string_view::npos) {
        field_name = "position_id";
    }

    // Пробрасываем доменное исключение валидации
    return {
        field_name,
        "Referenced entity not found. Invalid field: " + field_name,
        std::current_exception()
    };
}

int64_t parse_pg_timestamp_to_seconds(const std::string& ts_str) {
    int year, month, day, hour, min, sec;

    // Парсим YYYY-MM-DD HH:MM:SS. Дробная часть (.93819) будет проигнорирована,
    // так как стандартный Unix timestamp измеряется в целых секундах.
    if (std::sscanf(ts_str.c_str(), "%d-%d-%d %d:%d:%d",
                    &year, &month, &day, &hour, &min, &sec) != 6) {
        throw std::runtime_error("Не удалось распарсить timestamp: " + ts_str);
                    }

    std::tm tm = {};
    tm.tm_year = year - 1900;
    tm.tm_mon = month - 1;
    tm.tm_mday = day;
    tm.tm_hour = hour;
    tm.tm_min = min;
    tm.tm_sec = sec;
    tm.tm_isdst = 0; // 0 означает, что мы не учитываем летнее время (интерпретируем как UTC)

    // Преобразуем в Unix timestamp.
    // timegm (Linux/macOS) или _mkgmtime (Windows) гарантируют интерпретацию как UTC.
#if defined(_WIN32)
    std::time_t time = _mkgmtime(&tm);
#else
    std::time_t time = timegm(&tm);
#endif

    if (time == -1) {
        throw std::runtime_error("Ошибка конвертации в time_t: " + ts_str);
    }

    return time;
}
