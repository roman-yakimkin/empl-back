//
// Created by Roman Yakimkin on 01.06.2026.
//

#include <gtest/gtest.h>
#include <pqxx/pqxx>
#include <memory>
#include <map>

#include "repo_test.h"
#include "repository/position_repo.h"

#include "core/errors/data_access_exception.h"

class PositionRepoTest : public repo_test, public ::testing::Test {
protected:
    std::unique_ptr<repo::position_repo> repo;

    void SetUp() override {
        set_up();
        repo = std::make_unique<repo::position_repo>(db_manager.get(), test_logger.get());
    };

    void TearDown() override {
        repo.reset();
        tear_down();
    }

    void prepare_data() override {
        pqxx::connection conn(get_connection());
        pqxx::work tx(conn);

        try {
            tx.exec("drop table if exists positions");
            tx.exec(R"(
                create table positions (
                    id serial primary key,
                    name varchar not null unique,
                    CHECK (name <> '')
                )
            )");
            tx.exec(R"(
                insert into positions (name) values
                    ('Директор'),
                    ('Бухгалтер'),
                    ('Экономист')
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
            tx.exec("drop table if exists positions");
            tx.commit();
            test_logger->info("Test data dropped successfully");
        }
        catch (const std::exception& e) {
            test_logger->critical("Failed to drop data: {}", e.what());
        }
    };
};

TEST_F(PositionRepoTest, GetOne_ExistingId_ReturnsCorrectData) {
    auto result = repo->get_one(1);

    ASSERT_TRUE(result.has_value()) << "Expected position id=1 to exist";
    EXPECT_EQ(result->id, 1);
    EXPECT_EQ(result->name, "Директор");
}

TEST_F(PositionRepoTest, GetOne_NonExistingId_ReturnsNullopt) {
    auto result = repo->get_one(999999);

    EXPECT_FALSE(result.has_value()) << "Expected nullopt for missing id";
}

TEST_F(PositionRepoTest, GetOne_IsolationCheck) {
    {
        pqxx::connection conn(repo_test::get_connection());
        pqxx::work tx(conn);

        tx.exec("delete from positions where id = 1");
        tx.commit();
    }

    auto result = repo->get_one(1);
    EXPECT_FALSE(result.has_value());
}

TEST_F(PositionRepoTest, GetAll_ReturnsCorrectData) {
    auto result = repo->get_all();

    std::map<int, core::position> expected{
        {1, {1, "Директор"}},
        {2, {2, "Бухгалтер"}},
        {3, {3, "Экономист"}},
    };

    EXPECT_EQ(result, expected);
}

TEST_F(PositionRepoTest, Create_Success_ReturningCorrectData) {
    core::position dep;
    dep.name = "Инженер";

    auto result = repo->create(dep);

    EXPECT_EQ(result.id, 4);
    EXPECT_EQ(result.name, "Инженер");
}

TEST_F(PositionRepoTest, Create_Failure_DuplicateOfName) {
    core::position dep;
    dep.name = "Директор";

    ASSERT_THROW(repo->create(dep), core::data_access_exception) << "Expected data_access_exception";
}

TEST_F(PositionRepoTest, Update_Success_ReturningCorrectData) {
    core::position dep;
    dep.name = "Руководитель";

    auto result = repo->update(1, dep);

    ASSERT_TRUE(result.has_value()) << "Expected position updated";
    EXPECT_EQ(result->id, 1);
    EXPECT_EQ(result->name, "Руководитель");
}

TEST_F(PositionRepoTest, Update_Failure_NoName) {
    ASSERT_THROW(repo->update(1, {}), core::data_access_exception) << "Expected data_access_exception";
}

TEST_F(PositionRepoTest, Update_Failure_DuplicateOfName) {
    core::position dep;
    dep.name = "Директор";

    ASSERT_THROW(repo->update(2, dep), core::data_access_exception) << "Expected data_access_exception";
}

TEST_F(PositionRepoTest, Del_Success_ReturningTrue) {
    EXPECT_EQ(repo->del(1), true) << "Expected departament deleted";
}

TEST_F(PositionRepoTest, Del_Failure_NoDeletingItem) {
    EXPECT_EQ(repo->del(99999), false) << "Expected no departament for deleting";
}