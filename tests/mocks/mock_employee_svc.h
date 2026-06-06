//
// Created by Roman Yakimkin on 05.06.2026.
//

#pragma once

#include <optional>
#include <vector>
#include <gmock/gmock.h>

#include "core/contracts/i_employee_svc.h"
#include "core/filters/employee.h"
#include "core/models/employee.h"
#include "core/models/read/employee_detail.h"

class mock_employee_svc : public core::i_employee_svc {
public:
    MOCK_METHOD(std::optional<core::employee_detail>, get_one, (int id), (const, override));
    MOCK_METHOD(std::vector<core::employee_detail>, get_all, (const core::filters::employee&), (const, override));
    MOCK_METHOD(core::employee, create, (const core::employee& e), (override));
    MOCK_METHOD(std::optional<core::employee>, update, (int id, const core::employee& e), (override));
    MOCK_METHOD(bool, del, (int id), (override));
};
