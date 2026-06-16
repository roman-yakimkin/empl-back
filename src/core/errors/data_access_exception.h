//
// Created by Roman Yakimkin on 22.05.2026.
//

#pragma once

#include <stdexcept>
#include <string>

namespace core {
    class data_access_exception : public std::runtime_error {
    protected:
        std::exception_ptr _cause;
    public:
        data_access_exception(const std::string& msg, std::exception_ptr _cause = nullptr)
            : std::runtime_error(msg), _cause(std::move(_cause)) {}

        [[nodiscard]] std::exception_ptr cause() const { return _cause; }
    };
} // core