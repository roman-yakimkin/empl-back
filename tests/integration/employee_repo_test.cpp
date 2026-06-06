//
// Created by Roman Yakimkin on 04.06.2026.
//

#include <gtest/gtest.h>
#include <pqxx/pqxx>
#include <memory>

#include "repo_test.h"
#include "repository/employee_repo.h"

#include "core/errors/data_access_exception.h"
#include "core/errors/validation_exception.h"

class EmployeeRepoTest : public repo_test, public ::testing::Test {
protected:
    std::unique_ptr<repo::employee_repo> repo;

    void SetUp() override {
        set_up();
        repo = std::make_unique<repo::employee_repo>(db_manager.get(), test_logger.get());
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

            tx.exec("drop table if exists employees");
            tx.exec(R"(
                create table employees (
                    id serial primary key,
                    name varchar not null,
                    department_id int,
                    position_id int,
                    created_at timestamp default NOW(),
                    updated_at timestamp default NOW(),

                    constraint fk_department foreign key (department_id) references departments(id) on update cascade ,
                    constraint fk_position foreign key (position_id) references positions(id) on update cascade,
                    check (name <> '')
                );
            )");
            tx.exec(R"(
                insert into employees (name, department_id, position_id) values
                    ('Иванов', 1, 1),
                    ('Петров', 2, 2),
                    ('Сидоров', 3, 3)
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
            tx.exec("drop table if exists employees");
            tx.exec("drop table if exists positions");
            tx.exec("drop table if exists departments");
            tx.commit();
            test_logger->info("Test data dropped successfully");
        }
        catch (const std::exception& e) {
            test_logger->critical("Failed to drop data: {}", e.what());
        }
    };
};

TEST_F(EmployeeRepoTest, GetOne_ExistingId_ReturnsCorrectData) {
    auto result = repo->get_one(1);

    ASSERT_TRUE(result.has_value()) << "Expected department id=1 to exist";
    EXPECT_EQ(result->id, 1);
    EXPECT_EQ(result->name, "Иванов");
    EXPECT_EQ(result->dep.id, 1);
    EXPECT_EQ(result->dep.name, "Управление");
    EXPECT_EQ(result->pos.id, 1);
    EXPECT_EQ(result->pos.name, "Директор");
}

TEST_F(EmployeeRepoTest, GetOne_NonExistingId_ReturnsNullopt) {
    auto result = repo->get_one(999999);

    EXPECT_FALSE(result.has_value()) << "Expected nullopt for missing id";
}

TEST_F(EmployeeRepoTest, GetAll_EmptyFilters_ReturnsCorrectData) {
    core::filters::employee params;
    auto result = repo->get_all(params);

    EXPECT_EQ(result.size(), 3);

    EXPECT_EQ(result[0].id, 1);
    EXPECT_EQ(result[0].name, "Иванов");
    EXPECT_EQ(result[0].dep, core::department(1, "Управление"));
    EXPECT_EQ(result[0].pos, core::position(1, "Директор"));

    EXPECT_EQ(result[1].id, 2);
    EXPECT_EQ(result[1].name, "Петров");
    EXPECT_EQ(result[1].dep, core::department(2, "Бухгалтерия"));
    EXPECT_EQ(result[1].pos, core::position(2, "Бухгалтер"));

    EXPECT_EQ(result[2].id, 3);
    EXPECT_EQ(result[2].name, "Сидоров");
    EXPECT_EQ(result[2].dep, core::department(3, "Плановый отдел"));
    EXPECT_EQ(result[2].pos, core::position(3, "Экономист"));
}

TEST_F(EmployeeRepoTest, GetAll_FilterByDepartment_ReturnsCorrectData) {
    core::filters::employee params;
    params.department_id = std::make_optional(1);
    auto result = repo->get_all(params);

    EXPECT_EQ(result.size(), 1);

    EXPECT_EQ(result[0].id, 1);
    EXPECT_EQ(result[0].name, "Иванов");
    EXPECT_EQ(result[0].dep, core::department(1, "Управление"));
    EXPECT_EQ(result[0].pos, core::position(1, "Директор"));
}

TEST_F(EmployeeRepoTest, GetAll_FilterByPosition_ReturnsCorrectData) {
    core::filters::employee params;
    params.position_id = std::make_optional(2);
    auto result = repo->get_all(params);

    EXPECT_EQ(result.size(), 1);

    EXPECT_EQ(result[0].id, 2);
    EXPECT_EQ(result[0].name, "Петров");
    EXPECT_EQ(result[0].dep, core::department(2, "Бухгалтерия"));
    EXPECT_EQ(result[0].pos, core::position(2, "Бухгалтер"));
}

TEST_F(EmployeeRepoTest, GetAll_FilterByName_ReturnsCorrectData) {
    core::filters::employee params;
    params.name_pattern = std::make_optional("ров");
    auto result = repo->get_all(params);

    EXPECT_EQ(result.size(), 2);

    EXPECT_EQ(result[0].id, 2);
    EXPECT_EQ(result[0].name, "Петров");
    EXPECT_EQ(result[0].dep, core::department(2, "Бухгалтерия"));
    EXPECT_EQ(result[0].pos, core::position(2, "Бухгалтер"));

    EXPECT_EQ(result[1].id, 3);
    EXPECT_EQ(result[1].name, "Сидоров");
    EXPECT_EQ(result[1].dep, core::department(3, "Плановый отдел"));
    EXPECT_EQ(result[1].pos, core::position(3, "Экономист"));
}

TEST_F(EmployeeRepoTest, Create_Success_ReturningCorrectData) {
    core::employee item("Смирнов", 3, 3);

    auto result = repo->create(item);

    EXPECT_EQ(result.id, 4);
    EXPECT_EQ(result.name, "Смирнов");
    EXPECT_EQ(result.department_id, 3);
    EXPECT_EQ(result.position_id, 3);
}

TEST_F(EmployeeRepoTest, Create_Failure_NonExistingDepartment) {
    core::employee item("Смирнов", 99999, 3);

    ASSERT_THROW(repo->create(item), core::validation_exception) << "Expected data_access_exception";
}

TEST_F(EmployeeRepoTest, Update_Success_ReturningCorrectData) {
    core::employee item("Иванов Сергей", 1, 1);

    auto result = repo->update(1, item);

    ASSERT_TRUE(result.has_value()) << "Expected department updated";
    EXPECT_EQ(result->id, 1);
    EXPECT_EQ(result->name, "Иванов Сергей");
    EXPECT_EQ(result->department_id, 1);
    EXPECT_EQ(result->position_id, 1);
}

TEST_F(EmployeeRepoTest, Update_Failure_NoName) {
    core::employee item("", 1, 1);

    ASSERT_THROW(repo->update(1, item), core::data_access_exception) << "Expected data_access_exception";
}

TEST_F(EmployeeRepoTest, Update_Failure_NotExistingItem) {
    core::employee item("Смирнов", 1, 1);

    auto result = repo->update(99999, item);

    ASSERT_FALSE(result.has_value());
}

TEST_F(EmployeeRepoTest, Del_Success_ReturningTrue) {
    EXPECT_EQ(repo->del(1), true) << "Expected employee deleted";
}

TEST_F(EmployeeRepoTest, Del_Failure_NoDeletingItem) {
    EXPECT_EQ(repo->del(99999), false) << "Expected no employee for deleting";
}
