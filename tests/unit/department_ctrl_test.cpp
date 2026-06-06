//
// Created by Roman Yakimkin on 31.05.2026.
//

#include <gtest/gtest.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/null_sink.h>
#include <nlohmann/json.hpp>

#include "api/handlers/department_ctrl.h"

#include "ctrl_test.h"
#include "api/helpers/msg.h"
#include "core/errors/data_access_exception.h"
#include "core/errors/validation_exception.h"
#include "helpers/http.h"
#include "mocks/mock_department_svc.h"

class DepartmentCtrlTest : public ctrl_test, public ::testing::Test {
protected:
    testing::StrictMock<mock_department_svc> mock_svc;
    std::unique_ptr<api::department_ctrl> ctrl;

    void SetUp() override {
        set_up();
        ctrl =std::make_unique<api::department_ctrl>(&mock_svc, test_logger.get());
    };

    void TearDown() override {
        ctrl.reset();
        tear_down();
    };
};

TEST_F(DepartmentCtrlTest, GetOne_Success) {
    const int dept_id = 42;
    core::department expected_dept{dept_id, "Отдел АСУ"};

    EXPECT_CALL(mock_svc, get_one(dept_id))
        .Times(1)
        .WillOnce(::testing::Return(std::make_optional(expected_dept)));

    crow::request req = make_request("departments/42");
    crow::response resp;

    ctrl->get_one(req, resp, dept_id);

    EXPECT_EQ(resp.code, 200);

    auto json = parse_response(resp);

    EXPECT_EQ(json.at("id").get<int>(), dept_id);
    EXPECT_EQ(json.at("name").get<std::string>(), "Отдел АСУ");
}

TEST_F(DepartmentCtrlTest, GetOne_NotFound) {
    const int dept_id = 42;

    EXPECT_CALL(mock_svc, get_one(dept_id))
        .Times(1)
        .WillOnce(::testing::Return(std::nullopt));

    crow::request req = make_request("departments/42");
    crow::response resp;

    ctrl->get_one(req, resp, dept_id);

    EXPECT_EQ(resp.code, 404);

    auto json = parse_response(resp);

    EXPECT_EQ(json.at("error").get<std::string>(), api::msg::ERR_DEP_NOT_FOUND);
}

TEST_F(DepartmentCtrlTest, GetOne_ThrowDataAccessException) {
    const int dept_id = 42;

    EXPECT_CALL(mock_svc, get_one(dept_id))
        .Times(1)
        .WillOnce(::testing::Throw(core::data_access_exception("service exception")));

    crow::request req = make_request("departments/42");
    crow::response resp;

    ctrl->get_one(req, resp, dept_id);

    EXPECT_EQ(resp.code, 500);

    auto json = parse_response(resp);

    EXPECT_EQ(json.at("error").get<std::string>(), api::msg::ERR_DEP_DATA_ERROR);
}

TEST_F(DepartmentCtrlTest, GetOne_ThrowCustomException) {
    const int dept_id = 42;

    EXPECT_CALL(mock_svc, get_one(dept_id))
        .Times(1)
        .WillOnce(::testing::Throw(std::runtime_error("service exception")));

    crow::request req = make_request("departments/42");
    crow::response resp;

    ctrl->get_one(req, resp, dept_id);

    EXPECT_EQ(resp.code, 500);

    auto json = parse_response(resp);

    EXPECT_EQ(json.at("error").get<std::string>(), api::msg::ERR_INTERNAL);
}

TEST_F(DepartmentCtrlTest, GetAll_Success) {
    const std::map<int, core::department> expected_deps{
            {1, core::department{1, "Отдел АСУ"}},
            {2, core::department{2, "Отдел кадров"}},
            {3, core::department{3, "Бухгалтерия"}}
    };

    EXPECT_CALL(mock_svc, get_all())
        .Times(1)
        .WillOnce(::testing::Return(expected_deps));

    crow::request req = make_request("departments");
    crow::response resp;

    ctrl->get_all(req, resp);

    EXPECT_EQ(resp.code, 200);

    auto json = parse_response(resp);
    ASSERT_TRUE(json.at("items").is_array());
    EXPECT_EQ(json.at("items").size(), 3);

    // Проверяем первую запись для примера
    EXPECT_EQ(json.at("items")[0].at("id").get<int>(), 1);
    EXPECT_EQ(json.at("items")[0].at("name").get<std::string>(), "Отдел АСУ");
}

TEST_F(DepartmentCtrlTest, GetAll_EmptyList) {
    const std::map<int, core::department> empty_deps;

    EXPECT_CALL(mock_svc, get_all())
        .Times(1)
        .WillOnce(::testing::Return(empty_deps));

    crow::request req = make_request("departments");
    crow::response resp;

    ctrl->get_all(req, resp);

    EXPECT_EQ(resp.code, 200);

    auto json = parse_response(resp);
    ASSERT_TRUE(json.at("items").is_array());
    EXPECT_EQ(json.at("items").size(), 0);
}

TEST_F(DepartmentCtrlTest, GetAll_ThrowDataAccessException) {
    EXPECT_CALL(mock_svc, get_all())
        .Times(1)
        .WillOnce(::testing::Throw(core::data_access_exception("DB connection lost")));

    crow::request req = make_request("departments");
    crow::response resp;

    ctrl->get_all(req, resp);

    EXPECT_EQ(resp.code, 500);

    auto json = parse_response(resp);
    EXPECT_EQ(json.at("error").get<std::string>(), api::msg::ERR_DEP_LIST_ERROR);
}


TEST_F(DepartmentCtrlTest, GetAll_ThrowStdException) {
    EXPECT_CALL(mock_svc, get_all())
        .Times(1)
        .WillOnce(::testing::Throw(std::runtime_error("Unexpected error")));

    crow::request req = make_request("departments");
    crow::response resp;

    ctrl->get_all(req, resp);

    EXPECT_EQ(resp.code, 500);

    auto json = parse_response(resp);
    EXPECT_EQ(json.at("error").get<std::string>(), api::msg::ERR_INTERNAL);
}

TEST_F(DepartmentCtrlTest, Add_Success) {
    const std::string req_body = R"({"name": "Новый отдел"})";
    const core::department input_dept{0, "Новый отдел"};      // id=0, т.к. создаётся
    const core::department created_dept{101, "Новый отдел"};  // id=101 — вернул сервис

    EXPECT_CALL(mock_svc, create(::testing::Field(&core::department::name, "Новый отдел")))
        .Times(1)
        .WillOnce(::testing::Return(created_dept));

    crow::request req = make_request("departments", req_body, crow::HTTPMethod::Post);
    crow::response resp;

    ctrl->add(req, resp);

    EXPECT_EQ(resp.code, 201); // Created

    auto json = parse_response(resp);
    EXPECT_EQ(json.at("id").get<int>(), 101);
    EXPECT_EQ(json.at("name").get<std::string>(), "Новый отдел");
}

TEST_F(DepartmentCtrlTest, Add_InvalidJson) {
    const std::string invalid_body = R"({"name": "unclosed string)";

    crow::request req = make_request("departments", invalid_body, crow::HTTPMethod::Post);
    crow::response resp;

    ctrl->add(req, resp);

    EXPECT_EQ(resp.code, 400);

    auto json = parse_response(resp);
    EXPECT_EQ(json.at("error").get<std::string>(), api::msg::ERR_INVALID_JSON);

    // Сервис не должен быть вызван при невалидном входе
    ::testing::Mock::VerifyAndClearExpectations(&mock_svc);
}

TEST_F(DepartmentCtrlTest, Add_ValidationException) {
    const std::string req_body = R"({"name": ""})"; // Пустое имя — валидация не пройдёт

    EXPECT_CALL(mock_svc, create(::testing::_))
        .Times(1)
        .WillOnce(::testing::Throw(core::validation_exception("name", "Field cannot be empty")));

    crow::request req = make_request("departments", req_body, crow::HTTPMethod::Post);
    crow::response resp;

    ctrl->add(req, resp);

    EXPECT_EQ(resp.code, 400);

    auto json = parse_response(resp);
    EXPECT_EQ(json.at("error").get<std::string>(), "Field cannot be empty");
}

TEST_F(DepartmentCtrlTest, Add_ThrowDataAccessException) {
    const std::string req_body = R"({"name": "Отдел"})";

    EXPECT_CALL(mock_svc, create(::testing::_))
        .Times(1)
        .WillOnce(::testing::Throw(core::data_access_exception("FK constraint violated")));

    crow::request req = make_request("departments", req_body, crow::HTTPMethod::Post);
    crow::response resp;

    ctrl->add(req, resp);

    EXPECT_EQ(resp.code, 500);

    auto json = parse_response(resp);
    EXPECT_EQ(json.at("error").get<std::string>(), api::msg::ERR_DEP_CREATE_FAILED);
}

TEST_F(DepartmentCtrlTest, Add_ThrowStdException) {
    const std::string req_body = R"({"name": "Отдел"})";

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

TEST_F(DepartmentCtrlTest, Edit_Success) {
    const int dept_id = 42;
    const std::string req_body = R"({"name": "Обновлённый отдел"})";
    const core::department updated_dept{dept_id, "Обновлённый отдел"};

    EXPECT_CALL(mock_svc, update(dept_id, ::testing::Field(&core::department::name, "Обновлённый отдел")))
        .Times(1)
        .WillOnce(::testing::Return(std::make_optional(updated_dept)));

    crow::request req = make_request("departments/42", req_body, crow::HTTPMethod::Put);
    crow::response resp;

    ctrl->edit(req, resp, dept_id);

    EXPECT_EQ(resp.code, 200);

    auto json = parse_response(resp);
    EXPECT_EQ(json.at("id").get<int>(), dept_id);
    EXPECT_EQ(json.at("name").get<std::string>(), "Обновлённый отдел");
}

TEST_F(DepartmentCtrlTest, Edit_NotFound) {
    const int dept_id = 999;
    const std::string req_body = R"({"name": "Не существует"})";

    EXPECT_CALL(mock_svc, update(dept_id, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(std::nullopt));

    crow::request req = make_request("departments/999", req_body, crow::HTTPMethod::Put);
    crow::response resp;

    ctrl->edit(req, resp, dept_id);

    EXPECT_EQ(resp.code, 404);

    auto json = parse_response(resp);
    EXPECT_EQ(json.at("error").get<std::string>(), api::msg::ERR_DEP_NOT_FOUND);
}

TEST_F(DepartmentCtrlTest, Edit_InvalidJson) {
    const int dept_id = 42;
    const std::string invalid_body = R"({"name": invalid})";

    crow::request req = make_request("departments/42", invalid_body, crow::HTTPMethod::Put);
    crow::response resp;

    ctrl->edit(req, resp, dept_id);

    EXPECT_EQ(resp.code, 400);

    auto json = parse_response(resp);
    EXPECT_EQ(json.at("error").get<std::string>(), api::msg::ERR_INVALID_JSON);

    ::testing::Mock::VerifyAndClearExpectations(&mock_svc);
}

TEST_F(DepartmentCtrlTest, Edit_ValidationException) {
    const int dept_id = 42;
    const std::string req_body = R"({"name": ""})";

    EXPECT_CALL(mock_svc, update(dept_id, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Throw(core::validation_exception("name", "Name is required")));

    crow::request req = make_request("departments/42", req_body, crow::HTTPMethod::Put);
    crow::response resp;

    ctrl->edit(req, resp, dept_id);

    EXPECT_EQ(resp.code, 400);

    auto json = parse_response(resp);
    EXPECT_EQ(json.at("error").get<std::string>(), "Name is required");
}

TEST_F(DepartmentCtrlTest, Edit_ThrowDataAccessException) {
    const int dept_id = 42;
    const std::string req_body = R"({"name": "Отдел"})";

    EXPECT_CALL(mock_svc, update(dept_id, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Throw(core::data_access_exception("Unique constraint failed")));

    crow::request req = make_request("departments/42", req_body, crow::HTTPMethod::Put);
    crow::response resp;

    ctrl->edit(req, resp, dept_id);

    EXPECT_EQ(resp.code, 500);

    auto json = parse_response(resp);
    EXPECT_EQ(json.at("error").get<std::string>(), api::msg::ERR_DEP_UPDATE_FAILED);
}

TEST_F(DepartmentCtrlTest, Edit_ThrowStdException) {
    const int dept_id = 42;
    const std::string req_body = R"({"name": "Отдел"})";

    EXPECT_CALL(mock_svc, update(dept_id, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Throw(std::bad_alloc()));

    crow::request req = make_request("departments/42", req_body, crow::HTTPMethod::Put);
    crow::response resp;

    ctrl->edit(req, resp, dept_id);

    EXPECT_EQ(resp.code, 500);

    auto json = parse_response(resp);
    EXPECT_EQ(json.at("error").get<std::string>(), api::msg::ERR_INTERNAL);
}

TEST_F(DepartmentCtrlTest, Del_Success) {
    const int dept_id = 42;

    EXPECT_CALL(mock_svc, del(dept_id))
        .Times(1)
        .WillOnce(::testing::Return(true));

    crow::request req = make_request("departments/42", "", crow::HTTPMethod::Delete);
    crow::response resp;

    ctrl->del(req, resp, dept_id);

    EXPECT_EQ(resp.code, 204); // No Content
    EXPECT_TRUE(resp.body.empty()); // Тело ответа должно быть пустым
}

TEST_F(DepartmentCtrlTest, Del_NotFound) {
    const int dept_id = 999;

    EXPECT_CALL(mock_svc, del(dept_id))
        .Times(1)
        .WillOnce(::testing::Return(false));

    crow::request req = make_request("departments/999", "", crow::HTTPMethod::Delete);
    crow::response resp;

    ctrl->del(req, resp, dept_id);

    EXPECT_EQ(resp.code, 404);

    auto json = parse_response(resp);
    EXPECT_EQ(json.at("error").get<std::string>(), api::msg::ERR_DEP_NOT_FOUND);
}

TEST_F(DepartmentCtrlTest, Del_ThrowDataAccessException) {
    const int dept_id = 42;

    EXPECT_CALL(mock_svc, del(dept_id))
        .Times(1)
        .WillOnce(::testing::Throw(core::data_access_exception("Foreign key constraint")));

    crow::request req = make_request("departments/42", "", crow::HTTPMethod::Delete);
    crow::response resp;

    ctrl->del(req, resp, dept_id);

    EXPECT_EQ(resp.code, 500);

    auto json = parse_response(resp);
    EXPECT_EQ(json.at("error").get<std::string>(), api::msg::ERR_DEP_DELETE_FAILED);
}

TEST_F(DepartmentCtrlTest, Del_ThrowStdException) {
    const int dept_id = 42;

    EXPECT_CALL(mock_svc, del(dept_id))
        .Times(1)
        .WillOnce(::testing::Throw(std::exception()));

    crow::request req = make_request("departments/42", "", crow::HTTPMethod::Delete);
    crow::response resp;

    ctrl->del(req, resp, dept_id);

    EXPECT_EQ(resp.code, 500);

    auto json = parse_response(resp);
    EXPECT_EQ(json.at("error").get<std::string>(), api::msg::ERR_INTERNAL);
}