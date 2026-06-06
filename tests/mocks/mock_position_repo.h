//
// Created by Roman Yakimkin on 01.06.2026.
//

#pragma once
#include <gmock/gmock.h>
#include <map>

#include "core/contracts/i_position_repo.h"

class mock_position_repo : public core::i_position_repo {
public:
    MOCK_METHOD(std::optional<core::position>, get_one, (int), (const override));
    MOCK_METHOD((std::map<int, core::position>), get_all, (), (const override));
    MOCK_METHOD(core::position, create, (const core::position&), (override));
    MOCK_METHOD(std::optional<core::position>, update, (int, const core::position&), (override));
    MOCK_METHOD(bool, del, (int), (override));
};