//
// Created by Roman Yakimkin on 07.06.2026.
//

#include <httplib.h>
#include <gtest/gtest.h>
#include <nlohmann/json.hpp>
#include <pqxx/pqxx>
#include <mutex>

#include "helpers/db_test.h"
#include "helpers/envs.h"

class E2E_EmployeeTest : public db_test, public ::testing::Test {
protected:
    const std::string BASE_URL = "http://" + helpers::get_env("SERVER_HOST", "app-e2e-test") + ":" + helpers::get_env("SERVER_PORT", "8080");
    std::unique_ptr<httplib::Client> cli;

    void SetUp() override {
        set_up();

        cli = std::make_unique<httplib::Client>(BASE_URL);
        cli->set_connection_timeout(5);
        cli->set_read_timeout(5);

        auto health_res = cli->Get("/health");
        ASSERT_TRUE(health_res)
            << "Не удалось подключиться к микросервису по адресу " << BASE_URL
            << ". Убедитесь, что он запущен.";

        ASSERT_EQ(health_res->status, 200)
            << "Health check вернул неожиданный статус: " << health_res->status;

    };

    void TearDown() override {
        tear_down();
    };

    void prepare_data() override {
        pqxx::connection conn(get_connection());
        pqxx::work tx(conn);

        try {
            tx.exec(R"(
                create table departments (
                    id serial primary key,
                    name varchar not null unique,
                    CHECK (name <> '')
                )
            )");
            tx.exec(R"(
                create table positions (
                    id serial primary key,
                    name varchar not null unique,
                    CHECK (name <> '')
                )
            )");
            tx.exec(R"(
                create table employees (
                    id serial primary key,
                    name varchar not null,
                    department_id int,
                    position_id int,
                    created_at timestamp default NOW(),
                    updated_at timestamp default NOW(),

                    constraint fk_department foreign key (department_id) references departments(id) on update cascade,
                    constraint fk_position foreign key (position_id) references positions(id) on update cascade,
                    check (name <> '')
                )
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

TEST_F(E2E_EmployeeTest, CreateDepartmentPositionAndEmployee) {

    // Добавить новый отдел
    nlohmann::json dep_payload = {{"name", "Отдел разработки ПО"}};

    auto dep_res = cli->Post("/departments", dep_payload.dump(), "application/json");

    ASSERT_TRUE(dep_res) << "Ошибка сети при запросе к /departments";
    ASSERT_EQ(dep_res->status, 201)
        << "Ожидался статус 201 Created, но получено: " << dep_res->status
        << "\nТело ответа: " << dep_res->body;

    nlohmann::json dep_resp = nlohmann::json::parse(dep_res->body);
    ASSERT_TRUE(dep_resp.contains("id")) << "В ответе нет поля 'id'";
    int department_id = dep_resp["id"].get<int>();

    // Добавить новую должность
    nlohmann::json pos_payload = {{"name", "C++ разработчик"}};
    auto pos_res = cli->Post("/positions", pos_payload.dump(), "application/json");

    ASSERT_TRUE(pos_res) << "Ошибка сети при запросе к /positions";
    ASSERT_EQ(pos_res->status, 201)
        << "Ожидался статус 201 Created, но получено: " << pos_res->status
        << "\nТело ответа: " << pos_res->body;

    nlohmann::json pos_resp = nlohmann::json::parse(pos_res->body);
    ASSERT_TRUE(pos_resp.contains("id")) << "В ответе нет поля 'id'";
    int position_id = pos_resp["id"].get<int>();

    // Добавить нового сотрудника в новый отдел и с новой должностью
    nlohmann::json emp_payload = {
        {"name", "Смирнов Иван"},
        {"department_id", department_id},
        {"position_id", position_id}
    };

    auto emp_res = cli->Post("/employees", emp_payload.dump(), "application/json");

    ASSERT_TRUE(emp_res) << "Ошибка сети при запросе к /employees";
    ASSERT_EQ(emp_res->status, 201)
        << "Ожидался статус 201 Created, но получено: " << emp_res->status
        << "\nТело ответа: " << emp_res->body;

    nlohmann::json emp_resp = nlohmann::json::parse(emp_res->body);

    EXPECT_EQ(emp_resp["name"].get<std::string>(), "Смирнов Иван");
    EXPECT_EQ(emp_resp["department_id"].get<int>(), department_id);
    EXPECT_EQ(emp_resp["position_id"].get<int>(), position_id);

    // Проверить наличие служебных полей, которые обычно отдает бэкенд
    EXPECT_TRUE(emp_resp.contains("id"));
    EXPECT_TRUE(emp_resp.contains("created_at"));
    EXPECT_TRUE(emp_resp.contains("updated_at"));
}


