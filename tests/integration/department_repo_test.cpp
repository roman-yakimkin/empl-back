//
// Created by Roman Yakimkin on 30.05.2026.
//

#include <gtest/gtest.h>
#include <pqxx/pqxx>
#include <memory>
#include <map>

#include "repo_test.h"
#include "repository/department_repo.h"

#include "core/errors/data_access_exception.h"

class DepartmentRepoTest : public repo_test, public ::testing::Test {
protected:
    std::unique_ptr<repo::department_repo> repo;

    void SetUp() override {
        set_up();
        repo = std::make_unique<repo::department_repo>(db_manager.get(), test_logger.get());
    };

    void TearDown() override {
        repo.reset();
        tear_down();
    }

    void prepare_data() override {
        pqxx::connection conn(get_connection());
        pqxx::work tx(conn);

        try {
            tx.exec("drop table if exists departments");
            tx.exec(R"(
                create table departments (
                    id serial primary key,
                    name varchar not null unique,
                    CHECK (name <> '')
                )
            )");
            tx.exec(R"(
                insert into departments (name) values
                    ('Управление'),
                    ('Бухгалтерия'),
                    ('Плановый отдел')
            )");

            tx.commit();
            test_logger->info("Test data prepared successfully");
        }
        catch (const std::exception& e) {
            test_logger->critical("Failed to prepare database: {}", e.what());
            throw;
        }
    }

    void drop_data() override {
        pqxx::connection conn(get_connection());
        pqxx::work tx(conn);

        try {
            tx.exec("drop table if exists departments");
            tx.commit();
            test_logger->info("Test data dropped successfully");
        }
        catch (const std::exception& e) {
            test_logger->critical("Failed to drop data: {}", e.what());
        }
    };
};

TEST_F(DepartmentRepoTest, GetOne_ExistingId_ReturnsCorrectData) {
    auto result = repo->get_one(1);

    ASSERT_TRUE(result.has_value()) << "Expected department id=1 to exist";
    EXPECT_EQ(result->id, 1);
    EXPECT_EQ(result->name, "Управление");
}

TEST_F(DepartmentRepoTest, GetOne_NonExistingId_ReturnsNullopt) {
    auto result = repo->get_one(999999);

    EXPECT_FALSE(result.has_value()) << "Expected nullopt for missing id";
}

TEST_F(DepartmentRepoTest, GetOne_IsolationCheck) {
    {
        pqxx::connection conn(repo_test::get_connection());
        pqxx::work tx(conn);

        tx.exec("delete from departments where id = 1");
        tx.commit();
    }

    auto result = repo->get_one(1);
    EXPECT_FALSE(result.has_value());
}

TEST_F(DepartmentRepoTest, GetAll_ReturnsCorrectData) {
    auto result = repo->get_all();

    std::map<int, core::department> expected{
        {1, {1, "Управление"}},
        {2, {2, "Бухгалтерия"}},
        {3, {3, "Плановый отдел"}},
    };

    EXPECT_EQ(result, expected);
}

TEST_F(DepartmentRepoTest, Create_Success_ReturningCorrectData) {
    core::department dep;
    dep.name = "Охрана";

    auto result = repo->create(dep);

    EXPECT_EQ(result.id, 4);
    EXPECT_EQ(result.name, "Охрана");
}

TEST_F(DepartmentRepoTest, Create_Failure_DuplicateOfName) {
    core::department dep;
    dep.name = "Управление";

    ASSERT_THROW(repo->create(dep), core::data_access_exception) << "Expected data_access_exception";
}

TEST_F(DepartmentRepoTest, Update_Success_ReturningCorrectData) {
    core::department dep;
    dep.name = "Руководство";

    auto result = repo->update(1, dep);

    ASSERT_TRUE(result.has_value()) << "Expected department updated";
    EXPECT_EQ(result->id, 1);
    EXPECT_EQ(result->name, "Руководство");
}

TEST_F(DepartmentRepoTest, Update_Failure_NoName) {
    ASSERT_THROW(repo->update(1, {}), core::data_access_exception) << "Expected data_access_exception";
}

TEST_F(DepartmentRepoTest, Update_Failure_DuplicateOfName) {
    core::department dep;
    dep.name = "Управление";

    ASSERT_THROW(repo->update(2, dep), core::data_access_exception) << "Expected data_access_exception";
}

TEST_F(DepartmentRepoTest, Del_Success_ReturningTrue) {
    EXPECT_EQ(repo->del(1), true) << "Expected departament deleted";
}

TEST_F(DepartmentRepoTest, Del_Failure_NoDeletingItem) {
    EXPECT_EQ(repo->del(99999), false) << "Expected no departament for deleting";
}