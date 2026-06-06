//
// Created by Roman Yakimkin on 04.06.2026.
//

#include <gtest/gtest.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/null_sink.h>

#include "core/errors/validation_exception.h"
#include "mocks/mock_employee_repo.h"
#include "repository/department_repo.h"
#include "service/employee_service.h"

class EmployeeServiceTest : public ::testing::Test {
protected:
    testing::StrictMock<mock_employee_repo> repo_mock;
    std::shared_ptr<spdlog::logger> test_logger;
    std::unique_ptr<svc::employee_service> service;

    void SetUp() override {
        test_logger = spdlog::create<spdlog::sinks::null_sink_st>("test_logger");
        service = std::make_unique<svc::employee_service>(&repo_mock, test_logger.get());
    };

    void TearDown() override {
        spdlog::drop("test_logger");
    };
};

TEST_F(EmployeeServiceTest, GetOne_Success) {
    const int item_id = 42;
    const core::employee_detail expected(
        item_id,
        "Смирнов",
        core::department(10, "Транспортный цех"),
        core::position(20, "Водитель")
    );

    EXPECT_CALL(repo_mock, get_one(item_id))
        .Times(1)
        .WillOnce(testing::Return(expected));

    auto result = service->get_one(item_id);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->id, item_id);
    EXPECT_EQ(result->name, "Смирнов");
    EXPECT_EQ(result->dep, core::department(10, "Транспортный цех"));
    EXPECT_EQ(result->pos, core::position(20, "Водитель"));
}

TEST_F(EmployeeServiceTest, GetOne_NotFound_ReturnNullopt) {
    const int item_id = 99999;

    EXPECT_CALL(repo_mock, get_one(item_id))
        .Times(1)
        .WillOnce(testing::Return(std::nullopt));

    auto result = service->get_one(item_id);

    EXPECT_FALSE(result.has_value());
}

TEST_F(EmployeeServiceTest, GetOne_RepoThrowsException) {
    const int item_id = 1;

    EXPECT_CALL(repo_mock, get_one(item_id))
        .Times(1)
        .WillOnce(testing::Throw(std::runtime_error("DB connection failed")));

    EXPECT_THROW(auto result = service->get_one(item_id), std::runtime_error);
}

TEST_F(EmployeeServiceTest, GetAll_Success) {
    const std::vector<core::employee_detail> vec {
        {1, "Иванов", {1, "Управление"}, {10, "Директор"}},
        {2, "Петров", {2, "Бухгалтерия"}, {20, "Бухгалтер"}},
        {3, "Сидоров", {3, "Транспортный отдел"}, {30, "Водитель"}},
    };

    core::filters::employee params;

    EXPECT_CALL(repo_mock, get_all(testing::_))
        .Times(1)
        .WillOnce(testing::Return(vec));

    auto result = service->get_all(params);

    EXPECT_EQ(result, vec);
}

TEST_F(EmployeeServiceTest, Create_Success) {
    core::department dep(10, "Управление");
    core::position pos(20, "Директор");
    core::employee input( "Смирнов", 10, 20);
    core::employee saved( "Смирнов", 10, 20);
    saved.id = 15;

    EXPECT_CALL(repo_mock, create(
        testing::AllOf(
            testing::Field(&core::employee::name, "Смирнов"),
            testing::Field(&core::employee::department_id, 10),
            testing::Field(&core::employee::position_id, 20)
            )
        ))
        .Times(1)
        .WillOnce(testing::Return(saved));

    auto result = service->create(input);

    EXPECT_EQ(result.id, 15);
    EXPECT_EQ(result.name, "Смирнов");
    EXPECT_EQ(result.department_id, 10);
    EXPECT_EQ(result.position_id, 20);
}

TEST_F(EmployeeServiceTest, Create_Validation_EmptyName_Throws) {
    core::employee invalid("", 10, 20);  // пустое имя

    // Репозиторий НЕ должен быть вызван — валидация падает раньше
    EXPECT_CALL(repo_mock, create(testing::_)).Times(0);

    EXPECT_THROW(service->create(invalid), core::validation_exception);
}

TEST_F(EmployeeServiceTest, Update_Validation_EmptyName_Throws) {
    core::employee invalid(10, "", 10, 20);  // пустое имя

    EXPECT_CALL(repo_mock, update(testing::_, testing::_)).Times(0);
    EXPECT_THROW(service->update(10, invalid), core::validation_exception);
}

TEST_F(EmployeeServiceTest, Update_Success_DelegatesToRepo) {
    core::employee updated(10, "Смирнов", 10, 20);

    EXPECT_CALL(repo_mock, update(10, testing::AllOf(
        testing::Field(&core::employee::name, "Смирнов"),
        testing::Field(&core::employee::department_id, 10),
        testing::Field(&core::employee::position_id, 20)
        )
    ))
        .Times(1)
        .WillOnce(testing::Return(updated));

    auto result = service->update(10, updated);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->name, "Смирнов");
    EXPECT_EQ(result->department_id, 10);
    EXPECT_EQ(result->position_id, 20);
}

TEST_F(EmployeeServiceTest, Update_Failure_EmployeeNotFound) {
    core::employee updated(99999, "Смирнов", 10, 20);

    EXPECT_CALL(repo_mock, update(99999, testing::AllOf(
        testing::Field(&core::employee::name, "Смирнов"),
        testing::Field(&core::employee::department_id, 10),
        testing::Field(&core::employee::position_id, 20)
        )
    ))
        .Times(1)
        .WillOnce(testing::Return(std::nullopt));

    auto result = service->update(99999, updated);

    ASSERT_FALSE(result.has_value());
}

TEST_F(EmployeeServiceTest, Del_Success_ReturnsTrue) {
    EXPECT_CALL(repo_mock, del(42))
        .Times(1)
        .WillOnce(testing::Return(true));

    EXPECT_TRUE(service->del(42));
}

TEST_F(EmployeeServiceTest, Del_NotFound_ReturnsFalse) {
    EXPECT_CALL(repo_mock, del(999))
        .Times(1)
        .WillOnce(testing::Return(false));

    EXPECT_FALSE(service->del(999));
}





