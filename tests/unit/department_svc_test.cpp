//
// Created by Roman Yakimkin on 31.05.2026.
//

#include <gtest/gtest.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/null_sink.h>

#include "core/errors/validation_exception.h"
#include "mocks/mock_department_repo.h"
#include "service/department_service.h"

class DepartmentServiceTest : public ::testing::Test {
protected:
    testing::StrictMock<mock_department_repo> repo_mock;
    std::shared_ptr<spdlog::logger> test_logger;
    std::unique_ptr<svc::department_service> service;

    void SetUp() override {
        test_logger = spdlog::create<spdlog::sinks::null_sink_st>("test_logger");
        service = std::make_unique<svc::department_service>(&repo_mock, test_logger.get());
    };

    void TearDown() override {
        spdlog::drop("test_logger");
    };
};

TEST_F(DepartmentServiceTest, GetOne_Success) {
    const int dept_id = 42;
    const core::department expected(dept_id, "Отдел АСУ");

    EXPECT_CALL(repo_mock, get_one(dept_id))
        .Times(1)
        .WillOnce(testing::Return(expected));

    auto result = service->get_one(dept_id);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->id, dept_id);
    EXPECT_EQ(result->name, "Отдел АСУ");
}

TEST_F(DepartmentServiceTest, GetOne_NotFound_ReturnNullopt) {
    const int dept_id = 99999;

    EXPECT_CALL(repo_mock, get_one(dept_id))
        .Times(1)
        .WillOnce(testing::Return(std::nullopt));

    auto result = service->get_one(dept_id);

    EXPECT_FALSE(result.has_value());
}

TEST_F(DepartmentServiceTest, GetOne_RepoThrowsException) {
    const int dept_id = 1;

    EXPECT_CALL(repo_mock, get_one(dept_id))
        .Times(1)
        .WillOnce(testing::Throw(std::runtime_error("DB connection failed")));

    EXPECT_THROW(auto result = service->get_one(dept_id), std::runtime_error);
}

TEST_F(DepartmentServiceTest, Create_Success) {
    core::department input(0, "Отдел АСУ");
    core::department saved(15, "Отдел АСУ");

    EXPECT_CALL(repo_mock, create(testing::Field(&core::department::name, "Отдел АСУ")))
        .Times(1)
        .WillOnce(testing::Return(saved));

    auto result = service->create(input);

    EXPECT_EQ(result.id, 15);
    EXPECT_EQ(result.name, "Отдел АСУ");
}

TEST_F(DepartmentServiceTest, Create_Validation_EmptyName_Throws) {
    core::department invalid(0, "");  // пустое имя

    // Репозиторий НЕ должен быть вызван — валидация падает раньше
    EXPECT_CALL(repo_mock, create(testing::_)).Times(0);

    EXPECT_THROW(service->create(invalid), core::validation_exception);
}

TEST_F(DepartmentServiceTest, Create_Validation_EmptyName_CheckExceptionDetails) {
    core::department invalid(0, "");

    EXPECT_CALL(repo_mock, create(testing::_)).Times(0);

    try {
        service->create(invalid);
        FAIL() << "Expected validation_exception";
    } catch (const core::validation_exception& e) {
        EXPECT_EQ(e.field(), "name");
        EXPECT_THAT(e.what(), testing::HasSubstr("Название отдела"));
    }
}

TEST_F(DepartmentServiceTest, Update_Validation_EmptyName_Throws) {
    core::department invalid(10, "");  // пустое имя

    EXPECT_CALL(repo_mock, update(testing::_, testing::_)).Times(0);
    EXPECT_THROW(service->update(10, invalid), core::validation_exception);
}

TEST_F(DepartmentServiceTest, Update_Success_DelegatesToRepo) {
    core::department updated(10, "Новый отдел");

    EXPECT_CALL(repo_mock, update(10, testing::Field(&core::department::name, "Новый отдел")))
        .Times(1)
        .WillOnce(testing::Return(updated));

    auto result = service->update(10, updated);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->name, "Новый отдел");
}

TEST_F(DepartmentServiceTest, Del_Success_ReturnsTrue) {
    EXPECT_CALL(repo_mock, del(42))
        .Times(1)
        .WillOnce(testing::Return(true));

    EXPECT_TRUE(service->del(42));
}

TEST_F(DepartmentServiceTest, Del_NotFound_ReturnsFalse) {
    EXPECT_CALL(repo_mock, del(999))
        .Times(1)
        .WillOnce(testing::Return(false));

    EXPECT_FALSE(service->del(999));
}

TEST_F(DepartmentServiceTest, Del_ThrowsValidateException) {
    EXPECT_CALL(repo_mock, del(1))
        .Times(1)
        .WillOnce(testing::Throw(core::validation_exception("invalid field")));

    EXPECT_THROW(service->del(1), core::validation_exception);
}

TEST(DepartmentServiceConstructorTest, NullRepo_Throws) {
    auto logger = spdlog::create<spdlog::sinks::null_sink_st>("tmp_logger");

    EXPECT_THROW(
        svc::department_service(nullptr, logger.get()),
        std::invalid_argument
    );
}

TEST(DepartmentServiceConstructorTest, NullLogger_Throws) {
    mock_department_repo repo;

    EXPECT_THROW(
        svc::department_service(&repo, nullptr),
        std::invalid_argument
    );
}



