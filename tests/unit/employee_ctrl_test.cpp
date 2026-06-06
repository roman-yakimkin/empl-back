//
// Created by Roman Yakimkin on 05.06.2026.
//

#include <gtest/gtest.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/null_sink.h>
#include <nlohmann/json.hpp>

#include "api/handlers/employee_ctrl.h"

#include "ctrl_test.h"
#include "api/helpers/msg.h"
#include "core/errors/data_access_exception.h"
#include "core/errors/validation_exception.h"
#include "helpers/http.h"
#include "mocks/mock_employee_svc.h"

class EmployeeCtrlTest : public ctrl_test, public ::testing::Test {
protected:
    testing::StrictMock<mock_employee_svc> mock_svc;
    std::unique_ptr<api::employee_ctrl> ctrl;

    void SetUp() override {
        set_up();
        ctrl = std::make_unique<api::employee_ctrl>(&mock_svc, test_logger.get());
    };

    void TearDown() override {
        ctrl.reset();
        tear_down();
    };
};

TEST_F(EmployeeCtrlTest, GetOne_Success) {
    const int item_id = 42;
    core::employee_detail expected{
        item_id,
        "Смирнов",
        core::department(10, "Управление"),
        core::position(20, "Директор")
    };

    EXPECT_CALL(mock_svc, get_one(item_id))
        .Times(1)
        .WillOnce(::testing::Return(std::make_optional(expected)));

    crow::request req = make_request("employees/42");
    crow::response resp;

    ctrl->get_one(req, resp, item_id);

    EXPECT_EQ(resp.code, 200);

    auto json = parse_response(resp);

    EXPECT_EQ(json.at("id").get<int>(), item_id);
    EXPECT_EQ(json.at("name").get<std::string>(), "Смирнов");
    EXPECT_EQ(json.at(nlohmann::json::json_pointer("/department/id")).get<int>(), 10);
    EXPECT_EQ(json.at(nlohmann::json::json_pointer("/department/name")).get<std::string>(), "Управление");
    EXPECT_EQ(json.at(nlohmann::json::json_pointer("/position/id")).get<int>(), 20);
    EXPECT_EQ(json.at(nlohmann::json::json_pointer("/position/name")).get<std::string>(), "Директор");
}

TEST_F(EmployeeCtrlTest, GetOne_NotFound) {
    const int item_id = 42;

    EXPECT_CALL(mock_svc, get_one(item_id))
        .Times(1)
        .WillOnce(::testing::Return(std::nullopt));

    crow::request req = make_request("employees/42");
    crow::response resp;

    ctrl->get_one(req, resp, item_id);

    EXPECT_EQ(resp.code, 404);

    auto json = parse_response(resp);

    EXPECT_EQ(json.at("error").get<std::string>(), api::msg::ERR_EMPL_NOT_FOUND);
}

TEST_F(EmployeeCtrlTest, GetOne_ThrowDataAccessException) {
    const int item_id = 42;

    EXPECT_CALL(mock_svc, get_one(item_id))
        .Times(1)
        .WillOnce(::testing::Throw(core::data_access_exception("service exception")));

    crow::request req = make_request("employees/42");
    crow::response resp;

    ctrl->get_one(req, resp, item_id);

    EXPECT_EQ(resp.code, 500);

    auto json = parse_response(resp);

    EXPECT_EQ(json.at("error").get<std::string>(), api::msg::ERR_EMPL_DATA_ERROR);
}

TEST_F(EmployeeCtrlTest, GetOne_ThrowCustomException) {
    const int item_id = 42;

    EXPECT_CALL(mock_svc, get_one(item_id))
        .Times(1)
        .WillOnce(::testing::Throw(std::runtime_error("service exception")));

    crow::request req = make_request("employees/42");
    crow::response resp;

    ctrl->get_one(req, resp, item_id);

    EXPECT_EQ(resp.code, 500);

    auto json = parse_response(resp);

    EXPECT_EQ(json.at("error").get<std::string>(), api::msg::ERR_INTERNAL);
}


TEST_F(EmployeeCtrlTest, GetAll_Success) {
    const std::vector<core::employee_detail> expected{
            {1, "Иванов", {10, "Управление"}, {20, "Директор"}},
            {2, "Петров", {11, "Бухгалтерия"}, {21, "Бухгалтер"}},
            {3, "Сидоров", {12, "Транспортный отдел"}, {22, "Водитель"}}
    };

    EXPECT_CALL(mock_svc, get_all(testing::_))
        .Times(1)
        .WillOnce(::testing::Return(expected));

    crow::request req = make_request("employees");
    crow::response resp;

    ctrl->get_all(req, resp);

    EXPECT_EQ(resp.code, 200);

    auto json = parse_response(resp);
    ASSERT_TRUE(json.at("items").is_array());
    EXPECT_EQ(json.at("items").size(), 3);

    EXPECT_EQ(json.at("items")[0].at("id").get<int>(), 1);
    EXPECT_EQ(json.at("items")[0].at("name").get<std::string>(), "Иванов");
    EXPECT_EQ(json.at(nlohmann::json::json_pointer("/items/0/department/id")).get<int>(), 10);
    EXPECT_EQ(json.at(nlohmann::json::json_pointer("/items/0/department/name")).get<std::string>(), "Управление");
    EXPECT_EQ(json.at(nlohmann::json::json_pointer("/items/0/position/id")).get<int>(), 20);
    EXPECT_EQ(json.at(nlohmann::json::json_pointer("/items/0/position/name")).get<std::string>(), "Директор");

    EXPECT_EQ(json.at("items")[1].at("id").get<int>(), 2);
    EXPECT_EQ(json.at("items")[1].at("name").get<std::string>(), "Петров");
    EXPECT_EQ(json.at(nlohmann::json::json_pointer("/items/1/department/id")).get<int>(), 11);
    EXPECT_EQ(json.at(nlohmann::json::json_pointer("/items/1/department/name")).get<std::string>(), "Бухгалтерия");
    EXPECT_EQ(json.at(nlohmann::json::json_pointer("/items/1/position/id")).get<int>(), 21);
    EXPECT_EQ(json.at(nlohmann::json::json_pointer("/items/1/position/name")).get<std::string>(), "Бухгалтер");

    EXPECT_EQ(json.at("items")[2].at("id").get<int>(), 3);
    EXPECT_EQ(json.at("items")[2].at("name").get<std::string>(), "Сидоров");
    EXPECT_EQ(json.at(nlohmann::json::json_pointer("/items/2/department/id")).get<int>(), 12);
    EXPECT_EQ(json.at(nlohmann::json::json_pointer("/items/2/department/name")).get<std::string>(), "Транспортный отдел");
    EXPECT_EQ(json.at(nlohmann::json::json_pointer("/items/2/position/id")).get<int>(), 22);
    EXPECT_EQ(json.at(nlohmann::json::json_pointer("/items/2/position/name")).get<std::string>(), "Водитель");
}

TEST_F(EmployeeCtrlTest, GetAll_EmptyList) {
    const std::vector<core::employee_detail> empty_items;

    EXPECT_CALL(mock_svc, get_all(testing::_))
        .Times(1)
        .WillOnce(::testing::Return(empty_items));

    crow::request req = make_request("employees");
    crow::response resp;

    ctrl->get_all(req, resp);

    EXPECT_EQ(resp.code, 200);

    auto json = parse_response(resp);
    ASSERT_TRUE(json.at("items").is_array());
    EXPECT_EQ(json.at("items").size(), 0);
}

TEST_F(EmployeeCtrlTest, GetAll_InvalidParamFormat) {
    crow::request req = make_request("/employees?department_id=-10");
    crow::response resp;
    ctrl->get_all(req, resp);

    EXPECT_EQ(resp.code, 400);
    auto json = parse_response(resp);
    EXPECT_EQ(json.at("error").get<std::string>(), api::msg::ERR_EMPL_DEP_ID_MUST_BE_POSITIVE);

    req = make_request("/employees?department_id=abcd");
    ctrl->get_all(req, resp);

    EXPECT_EQ(resp.code, 400);
    json = parse_response(resp);
    EXPECT_EQ(json.at("error").get<std::string>(), api::msg::ERR_EMPL_INVALID_DEP_ID_FORMAT);

    req = make_request("/employees?position_id=-20");
    ctrl->get_all(req, resp);

    EXPECT_EQ(resp.code, 400);
    json = parse_response(resp);
    EXPECT_EQ(json.at("error").get<std::string>(), api::msg::ERR_EMPL_POS_ID_MUST_BE_POSITIVE);

    req = make_request("/employees?position_id=abcd");
    ctrl->get_all(req, resp);

    EXPECT_EQ(resp.code, 400);
    json = parse_response(resp);
    EXPECT_EQ(json.at("error").get<std::string>(), api::msg::ERR_EMPL_INVALID_POS_ID_FORMAT);
}


TEST_F(EmployeeCtrlTest, GetAll_ThrowDataAccessException) {
    EXPECT_CALL(mock_svc, get_all(testing::_))
        .Times(1)
        .WillOnce(::testing::Throw(core::data_access_exception("DB connection lost")));

    crow::request req = make_request("employees");
    crow::response resp;

    ctrl->get_all(req, resp);

    EXPECT_EQ(resp.code, 500);

    auto json = parse_response(resp);
    EXPECT_EQ(json.at("error").get<std::string>(), api::msg::ERR_EMPL_LIST_ERROR);
}

TEST_F(EmployeeCtrlTest, GetAll_ThrowStdException) {
    EXPECT_CALL(mock_svc, get_all(testing::_))
        .Times(1)
        .WillOnce(::testing::Throw(std::runtime_error("Unexpected error")));

    crow::request req = make_request("employees");
    crow::response resp;

    ctrl->get_all(req, resp);

    EXPECT_EQ(resp.code, 500);

    auto json = parse_response(resp);
    EXPECT_EQ(json.at("error").get<std::string>(), api::msg::ERR_INTERNAL);
}

TEST_F(EmployeeCtrlTest, Add_Success) {
    const std::string req_body = R"({"name": "Смирнов", "department_id": 10, "position_id": 20})";
    const core::employee input_item(0, "Смирнов", 10, 20);      // id=0, т.к. создаётся
    const core::employee created_item{101, "Смирнов", 10, 20};  // id=101 — вернул сервис

    EXPECT_CALL(mock_svc, create(testing::AllOf(
        ::testing::Field(&core::employee::name, "Смирнов"),
        ::testing::Field(&core::employee::department_id, 10),
        ::testing::Field(&core::employee::position_id, 20)
        ))
    )
        .Times(1)
        .WillOnce(::testing::Return(created_item));

    crow::request req = make_request("employees", req_body, crow::HTTPMethod::Post);
    crow::response resp;

    ctrl->add(req, resp);

    EXPECT_EQ(resp.code, 201); // Created

    auto json = parse_response(resp);
    EXPECT_EQ(json.at("id").get<int>(), 101);
    EXPECT_EQ(json.at("name").get<std::string>(), "Смирнов");
    EXPECT_EQ(json.at("department_id").get<int>(), 10);
    EXPECT_EQ(json.at("position_id").get<int>(), 20);
}

TEST_F(EmployeeCtrlTest, Add_InvalidJson) {
    const std::string invalid_body = R"({"name": "unclosed string)";

    crow::request req = make_request("employees", invalid_body, crow::HTTPMethod::Post);
    crow::response resp;

    ctrl->add(req, resp);

    EXPECT_EQ(resp.code, 400);

    auto json = parse_response(resp);
    EXPECT_EQ(json.at("error").get<std::string>(), api::msg::ERR_INVALID_JSON);

    // Сервис не должен быть вызван при невалидном входе
    ::testing::Mock::VerifyAndClearExpectations(&mock_svc);
}

TEST_F(EmployeeCtrlTest, Add_ValidationException) {
    const std::string req_body = R"({"name": "", "department_id": 10, "position_id": 20})"; // Пустое имя — валидация не пройдёт

    EXPECT_CALL(mock_svc, create(::testing::_))
        .Times(1)
        .WillOnce(::testing::Throw(core::validation_exception("name", "Field cannot be empty")));

    crow::request req = make_request("employees", req_body, crow::HTTPMethod::Post);
    crow::response resp;

    ctrl->add(req, resp);

    EXPECT_EQ(resp.code, 400);

    auto json = parse_response(resp);
    EXPECT_EQ(json.at("error").get<std::string>(), "Field cannot be empty");
}

TEST_F(EmployeeCtrlTest, Add_ThrowDataAccessException) {
    const std::string req_body = R"({"name": "Смирнов", "department_id": 10, "position_id": 20})";

    EXPECT_CALL(mock_svc, create(::testing::_))
        .Times(1)
        .WillOnce(::testing::Throw(core::data_access_exception("FK constraint violated")));

    crow::request req = make_request("departments", req_body, crow::HTTPMethod::Post);
    crow::response resp;

    ctrl->add(req, resp);

    EXPECT_EQ(resp.code, 500);

    auto json = parse_response(resp);
    EXPECT_EQ(json.at("error").get<std::string>(), api::msg::ERR_EMPL_CREATE_FAILED);
}

TEST_F(EmployeeCtrlTest, Add_ThrowStdException) {
    const std::string req_body = R"({"name": "Смирнов", "department_id": 10, "position_id": 20})";

    EXPECT_CALL(mock_svc, create(::testing::_))
        .Times(1)
        .WillOnce(::testing::Throw(std::logic_error("Unexpected state")));

    crow::request req = make_request("departments", req_body, crow::HTTPMethod::Post);
    crow::response resp;

    ctrl->add(req, resp);

    EXPECT_EQ(resp.code, 500);

    auto json = parse_response(resp);
    EXPECT_EQ(json.at("error").get<std::string>(), api::msg::ERR_INTERNAL);
}

TEST_F(EmployeeCtrlTest, Edit_Success) {
    const int item_id = 42;
    const std::string req_body = R"({"name": "Смирнов", "department_id": 10, "position_id": 20})";
    const core::employee updated(item_id, "Смирнов", 10, 20);

    EXPECT_CALL(mock_svc, update(item_id, ::testing::AllOf(
        ::testing::Field(&core::employee::name, "Смирнов"),
        ::testing::Field(&core::employee::department_id, 10),
        ::testing::Field(&core::employee::position_id, 20)
        )
    ))
        .Times(1)
        .WillOnce(::testing::Return(std::make_optional(updated)));

    crow::request req = make_request("employees/42", req_body, crow::HTTPMethod::Put);
    crow::response resp;

    ctrl->edit(req, resp, item_id);

    EXPECT_EQ(resp.code, 200);

    auto json = parse_response(resp);
    EXPECT_EQ(json.at("id").get<int>(), item_id);
    EXPECT_EQ(json.at("name").get<std::string>(), "Смирнов");
    EXPECT_EQ(json.at("department_id").get<int>(), 10);
    EXPECT_EQ(json.at("position_id").get<int>(), 20);
}

TEST_F(EmployeeCtrlTest, Edit_NotFound) {
    const int item_id = 999;
    const std::string req_body = R"({"name": "Никто", "department_id": 10, "position_id": 20})";

    EXPECT_CALL(mock_svc, update(item_id, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(std::nullopt));

    crow::request req = make_request("employees/999", req_body, crow::HTTPMethod::Put);
    crow::response resp;

    ctrl->edit(req, resp, item_id);

    EXPECT_EQ(resp.code, 404);

    auto json = parse_response(resp);
    EXPECT_EQ(json.at("error").get<std::string>(), api::msg::ERR_EMPL_NOT_FOUND);
}

TEST_F(EmployeeCtrlTest, Edit_InvalidJson) {
    const int item_id = 42;
    const std::string invalid_body = R"({"name": invalid})";

    crow::request req = make_request("employees/42", invalid_body, crow::HTTPMethod::Put);
    crow::response resp;

    ctrl->edit(req, resp, item_id);

    EXPECT_EQ(resp.code, 400);

    auto json = parse_response(resp);
    EXPECT_EQ(json.at("error").get<std::string>(), api::msg::ERR_INVALID_JSON);

    ::testing::Mock::VerifyAndClearExpectations(&mock_svc);
}

TEST_F(EmployeeCtrlTest, Edit_ValidationException) {
    const int item_id = 42;
    const std::string req_body = R"({"name": "", "department_id": 10, "position_id": 20})";

    EXPECT_CALL(mock_svc, update(item_id, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Throw(core::validation_exception("name", "Name is required")));

    crow::request req = make_request("employees/42", req_body, crow::HTTPMethod::Put);
    crow::response resp;

    ctrl->edit(req, resp, item_id);

    EXPECT_EQ(resp.code, 400);

    auto json = parse_response(resp);
    EXPECT_EQ(json.at("error").get<std::string>(), "Name is required");
}

TEST_F(EmployeeCtrlTest, Edit_ThrowDataAccessException) {
    const int item_id = 42;
    const std::string req_body = R"({"name": "Смирнов", "department_id": 10, "position_id": 20})";

    EXPECT_CALL(mock_svc, update(item_id, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Throw(core::data_access_exception("Unique constraint failed")));

    crow::request req = make_request("employees/42", req_body, crow::HTTPMethod::Put);
    crow::response resp;

    ctrl->edit(req, resp, item_id);

    EXPECT_EQ(resp.code, 500);

    auto json = parse_response(resp);
    EXPECT_EQ(json.at("error").get<std::string>(), api::msg::ERR_EMPL_UPDATE_FAILED);
}

TEST_F(EmployeeCtrlTest, Edit_ThrowStdException) {
    const int item_id = 42;
    const std::string req_body = R"({"name": "Смирнов", "department_id": 10, "position_id": 20})";

    EXPECT_CALL(mock_svc, update(item_id, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Throw(std::bad_alloc()));

    crow::request req = make_request("employees/42", req_body, crow::HTTPMethod::Put);
    crow::response resp;

    ctrl->edit(req, resp, item_id);

    EXPECT_EQ(resp.code, 500);

    auto json = parse_response(resp);
    EXPECT_EQ(json.at("error").get<std::string>(), api::msg::ERR_INTERNAL);
}

TEST_F(EmployeeCtrlTest, Del_Success) {
    const int item_id = 42;

    EXPECT_CALL(mock_svc, del(item_id))
        .Times(1)
        .WillOnce(::testing::Return(true));

    crow::request req = make_request("employees/42", "", crow::HTTPMethod::Delete);
    crow::response resp;

    ctrl->del(req, resp, item_id);

    EXPECT_EQ(resp.code, 204); // No Content
    EXPECT_TRUE(resp.body.empty()); // Тело ответа должно быть пустым
}

TEST_F(EmployeeCtrlTest, Del_NotFound) {
    const int item_id = 999999;

    EXPECT_CALL(mock_svc, del(item_id))
        .Times(1)
        .WillOnce(::testing::Return(false));

    crow::request req = make_request("employees/999999", "", crow::HTTPMethod::Delete);
    crow::response resp;

    ctrl->del(req, resp, item_id);

    EXPECT_EQ(resp.code, 404);

    auto json = parse_response(resp);
    EXPECT_EQ(json.at("error").get<std::string>(), api::msg::ERR_EMPL_NOT_FOUND);
}

TEST_F(EmployeeCtrlTest, Del_ThrowDataAccessException) {
    const int item_id = 42;

    EXPECT_CALL(mock_svc, del(item_id))
        .Times(1)
        .WillOnce(::testing::Throw(core::data_access_exception("Foreign key constraint")));

    crow::request req = make_request("employees/42", "", crow::HTTPMethod::Delete);
    crow::response resp;

    ctrl->del(req, resp, item_id);

    EXPECT_EQ(resp.code, 500);

    auto json = parse_response(resp);
    EXPECT_EQ(json.at("error").get<std::string>(), api::msg::ERR_EMPL_DELETE_FAILED);
}

TEST_F(EmployeeCtrlTest, Del_ThrowStdException) {
    const int item_id = 42;

    EXPECT_CALL(mock_svc, del(item_id))
        .Times(1)
        .WillOnce(::testing::Throw(std::exception()));

    crow::request req = make_request("employees/42", "", crow::HTTPMethod::Delete);
    crow::response resp;

    ctrl->del(req, resp, item_id);

    EXPECT_EQ(resp.code, 500);

    auto json = parse_response(resp);
    EXPECT_EQ(json.at("error").get<std::string>(), api::msg::ERR_INTERNAL);
}
