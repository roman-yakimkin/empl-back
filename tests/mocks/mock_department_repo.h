//
// Created by Roman Yakimkin on 31.05.2026.
//

#pragma once
#include <gmock/gmock.h>
#include <map>

#include "core/contracts/i_department_repo.h"

class mock_department_repo : public core::i_department_repo {
public:
    MOCK_METHOD(std::optional<core::department>, get_one, (int), (const override));
    MOCK_METHOD((std::map<int, core::department>), get_all, (), (const override));
    MOCK_METHOD(core::department, create, (const core::department&), (override));
    MOCK_METHOD(std::optional<core::department>, update, (int, const core::department&), (override));
    MOCK_METHOD(bool, del, (int), (override));
};