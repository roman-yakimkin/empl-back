//
// Created by Roman Yakimkin on 04.06.2026.
//

#pragma once

#include <gmock/gmock.h>
#include <vector>

#include "core/contracts/i_employee_repo.h"

class mock_employee_repo : public core::i_employee_repo {
public:
    MOCK_METHOD(std::optional<core::employee_detail>, get_one, (int), (const override));
    MOCK_METHOD(std::vector<core::employee_detail>, get_all, (const core::filters::employee&), (const override));
    MOCK_METHOD(core::employee, create, (const core::employee&), (override));
    MOCK_METHOD(std::optional<core::employee>, update, (int, const core::employee&), (override));
    MOCK_METHOD(bool, del, (int), (override));
};