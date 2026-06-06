//
// Created by Roman Yakimkin on 01.06.2026.
//

#pragma once

#include <optional>
#include <map>
#include <gmock/gmock.h>

#include "core/contracts/i_position_svc.h"

class mock_position_svc : public core::i_position_svc {
public:
    MOCK_METHOD(std::optional<core::position>, get_one, (int id), (const, override));
    MOCK_METHOD((std::map<int, core::position>), get_all, (), (const, override));
    MOCK_METHOD(core::position, create, (const core::position& p), (override));
    MOCK_METHOD(std::optional<core::position>, update, (int id, const core::position& p), (override));
    MOCK_METHOD(bool, del, (int id), (override));
};