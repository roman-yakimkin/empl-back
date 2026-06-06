//
// Created by Roman Yakimkin on 31.05.2026.
//

#pragma once

#include <optional>
#include <map>
#include <gmock/gmock.h>

#include "core/contracts/i_department_svc.h"

class mock_department_svc : public core::i_department_svc {
public:
    MOCK_METHOD(std::optional<core::department>, get_one, (int id), (const, override));
    MOCK_METHOD((std::map<int, core::department>), get_all, (), (const, override));
    MOCK_METHOD(core::department, create, (const core::department& d), (override));
    MOCK_METHOD(std::optional<core::department>, update, (int id, const core::department& d), (override));
    MOCK_METHOD(bool, del, (int id), (override));
};
