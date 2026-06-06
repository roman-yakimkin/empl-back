//
// Created by Roman Yakimkin on 29.05.2026.
//

#pragma once

#include <stdexcept>

namespace core {
    class validation_exception : public std::runtime_error {
        std::string _field;          // имя поля, которое не прошло валидацию
        std::exception_ptr _cause;   // оригинальное исключение, если валидация упала внутри
    public:
        // Конструктор только с сообщением (самый частый случай)
        explicit validation_exception(const std::string& msg)
            : std::runtime_error(msg), _field{}, _cause{} {}

        // Конструктор с указанием поля
        validation_exception(const std::string& field, const std::string& msg)
            : std::runtime_error(msg), _field(field), _cause{} {}

        // Конструктор с указанием всех полей
        validation_exception(const std::string& field, const std::string& msg, std::exception_ptr cause)
            : std::runtime_error(msg), _field(field), _cause(std::move(cause)) {}

        // Конструктор с вложенным исключением (если валидация делегирует и что-то упало)
        validation_exception(const std::string& msg, std::exception_ptr cause)
            : std::runtime_error(msg), _field{}, _cause(std::move(cause)) {}

        // Геттеры
        [[nodiscard]] const std::string& field() const noexcept { return _field; }
        [[nodiscard]] std::exception_ptr cause() const noexcept { return _cause; }

        // Удобный метод для ре-трасса вложенного исключения (если нужно)
        void rethrow_cause() const {
            if (_cause) std::rethrow_exception(_cause);
        }
    };
} // core