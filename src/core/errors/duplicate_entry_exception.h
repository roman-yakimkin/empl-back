//
// Created by Roman Yakimkin on 16.06.2026.
//

#pragma once
#include "data_access_exception.h"

namespace core {
    class duplicate_entry_exception : public data_access_exception {
    public:
        explicit duplicate_entry_exception(const std::string& msg, const std::exception_ptr &_cause = nullptr)
        : data_access_exception(msg, _cause) {}
    };
} // core