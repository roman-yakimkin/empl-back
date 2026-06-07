//
// Created by Roman Yakimkin on 01.06.2026.
//

#include <gtest/gtest.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/null_sink.h>

#include "core/errors/validation_exception.h"
#include "mocks/mock_position_repo.h"
#include "service/position_service.h"

class PositionServiceTest : public ::testing::Test {
protected:
    testing::StrictMock<mock_position_repo> repo_mock;
    std::shared_ptr<spdlog::logger> test_logger;
    std::unique_ptr<svc::position_service> service;

    void SetUp() override {
        test_logger = spdlog::create<spdlog::sinks::null_sink_st>("test_logger");
        service = std::make_unique<svc::position_service>(&repo_mock, test_logger.get());
    };

    void TearDown() override {
        spdlog::drop("test_logger");
    };
};

TEST_F(PositionServiceTest, GetOne_Success) {
    const int pos_id = 42;
    const core::position expected(pos_id, "Инженер");

    EXPECT_CALL(repo_mock, get_one(pos_id))
        .Times(1)
        .WillOnce(testing::Return(expected));

    auto result = service->get_one(pos_id);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->id, pos_id);
    EXPECT_EQ(result->name, "Инженер");
}

TEST_F(PositionServiceTest, GetOne_NotFound_ReturnNullopt) {
    const int pos_id = 99999;

    EXPECT_CALL(repo_mock, get_one(pos_id))
        .Times(1)
        .WillOnce(testing::Return(std::nullopt));

    auto result = service->get_one(pos_id);

    EXPECT_FALSE(result.has_value());
}

TEST_F(PositionServiceTest, GetOne_RepoThrowsException) {
    const int pos_id = 1;

    EXPECT_CALL(repo_mock, get_one(pos_id))
        .Times(1)
        .WillOnce(testing::Throw(std::runtime_error("DB connection failed")));

    EXPECT_THROW(auto result = service->get_one(pos_id), std::runtime_error);
}

TEST_F(PositionServiceTest, Create_Success) {
    core::position input(0, "Инженер");
    core::position saved(15, "Инженер");

    EXPECT_CALL(repo_mock, create(testing::Field(&core::position::name, "Инженер")))
        .Times(1)
        .WillOnce(testing::Return(saved));

    auto result = service->create(input);

    EXPECT_EQ(result.id, 15);
    EXPECT_EQ(result.name, "Инженер");
}

TEST_F(PositionServiceTest, Create_Validation_EmptyName_Throws) {
    core::position invalid(0, "");  // пустое имя

    // Репозиторий НЕ должен быть вызван — валидация падает раньше
    EXPECT_CALL(repo_mock, create(testing::_)).Times(0);

    EXPECT_THROW(service->create(invalid), core::validation_exception);
}

TEST_F(PositionServiceTest, Create_Validation_EmptyName_CheckExceptionDetails) {
    core::position invalid(0, "");

    EXPECT_CALL(repo_mock, create(testing::_)).Times(0);

    try {
        service->create(invalid);
        FAIL() << "Expected validation_exception";
    } catch (const core::validation_exception& e) {
        EXPECT_EQ(e.field(), "name");
        EXPECT_THAT(e.what(), testing::HasSubstr("Название должности"));
    }
}

TEST_F(PositionServiceTest, Update_Validation_EmptyName_Throws) {
    core::position invalid(10, "");  // пустое имя

    EXPECT_CALL(repo_mock, update(testing::_, testing::_)).Times(0);
    EXPECT_THROW(service->update(10, invalid), core::validation_exception);
}

TEST_F(PositionServiceTest, Update_Success_DelegatesToRepo) {
    core::position updated(10, "Программист");

    EXPECT_CALL(repo_mock, update(10, testing::Field(&core::position::name, "Программист")))
        .Times(1)
        .WillOnce(testing::Return(updated));

    auto result = service->update(10, updated);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->name, "Программист");
}

TEST_F(PositionServiceTest, Del_Success_ReturnsTrue) {
    EXPECT_CALL(repo_mock, del(42))
        .Times(1)
        .WillOnce(testing::Return(true));

    EXPECT_TRUE(service->del(42));
}

TEST_F(PositionServiceTest, Del_NotFound_ReturnsFalse) {
    EXPECT_CALL(repo_mock, del(999))
        .Times(1)
        .WillOnce(testing::Return(false));

    EXPECT_FALSE(service->del(999));
}

TEST_F(PositionServiceTest, Del_ThrowsValidateException) {
    EXPECT_CALL(repo_mock, del(1))
        .Times(1)
        .WillOnce(testing::Throw(core::validation_exception("invalid field")));

    EXPECT_THROW(service->del(1), core::validation_exception);
}


TEST(PositionServiceConstructorTest, NullRepo_Throws) {
    auto logger = spdlog::create<spdlog::sinks::null_sink_st>("tmp_logger_1");

    EXPECT_THROW(
        svc::position_service(nullptr, logger.get()),
        std::invalid_argument
    );
}

TEST(PositionServiceConstructorTest, NullLogger_Throws) {
    mock_position_repo repo;

    EXPECT_THROW(
        svc::position_service(&repo, nullptr),
        std::invalid_argument
    );
}

