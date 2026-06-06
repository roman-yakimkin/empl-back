//
// Created by Roman Yakimkin on 01.06.2026.
//

#include <gtest/gtest.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/null_sink.h>
#include <nlohmann/json.hpp>

#include "api/handlers/position_ctrl.h"

#include "ctrl_test.h"
#include "api/helpers/msg.h"
#include "core/errors/data_access_exception.h"
#include "core/errors/validation_exception.h"
#include "helpers/http.h"
#include "mocks/mock_position_svc.h"

class PositionCtrlTest : public ctrl_test, public ::testing::Test {
protected:
    testing::StrictMock<mock_position_svc> mock_svc;
    std::unique_ptr<api::position_ctrl> ctrl;

    void SetUp() override {
        set_up();
        ctrl =std::make_unique<api::position_ctrl>(&mock_svc, test_logger.get());
    };

    void TearDown() override {
        ctrl.reset();
        tear_down();
    };
};

TEST_F(PositionCtrlTest, GetOne_Success) {
    const int dept_id = 42;
    core::position expected_dept{dept_id, "Программист"};

    EXPECT_CALL(mock_svc, get_one(dept_id))
        .Times(1)
        .WillOnce(::testing::Return(std::make_optional(expected_dept)));

    crow::request req = make_request("positions/42");
    crow::response resp;

    ctrl->get_one(req, resp, dept_id);

    EXPECT_EQ(resp.code, 200);

    auto json = parse_response(resp);

    EXPECT_EQ(json.at("id").get<int>(), dept_id);
    EXPECT_EQ(json.at("name").get<std::string>(), "Программист");
}

TEST_F(PositionCtrlTest, GetOne_NotFound) {
    const int dept_id = 42;

    EXPECT_CALL(mock_svc, get_one(dept_id))
        .Times(1)
        .WillOnce(::testing::Return(std::nullopt));

    crow::request req = make_request("positions/42");
    crow::response resp;

    ctrl->get_one(req, resp, dept_id);

    EXPECT_EQ(resp.code, 404);

    auto json = parse_response(resp);

    EXPECT_EQ(json.at("error").get<std::string>(), api::msg::ERR_POS_NOT_FOUND);
}

TEST_F(PositionCtrlTest, GetOne_ThrowDataAccessException) {
    const int dept_id = 42;

    EXPECT_CALL(mock_svc, get_one(dept_id))
        .Times(1)
        .WillOnce(::testing::Throw(core::data_access_exception("service exception")));

    crow::request req = make_request("positions/42");
    crow::response resp;

    ctrl->get_one(req, resp, dept_id);

    EXPECT_EQ(resp.code, 500);

    auto json = parse_response(resp);

    EXPECT_EQ(json.at("error").get<std::string>(), api::msg::ERR_POS_DATA_ERROR);
}

TEST_F(PositionCtrlTest, GetOne_ThrowCustomException) {
    const int dept_id = 42;

    EXPECT_CALL(mock_svc, get_one(dept_id))
        .Times(1)
        .WillOnce(::testing::Throw(std::runtime_error("service exception")));

    crow::request req = make_request("positions/42");
    crow::response resp;

    ctrl->get_one(req, resp, dept_id);

    EXPECT_EQ(resp.code, 500);

    auto json = parse_response(resp);

    EXPECT_EQ(json.at("error").get<std::string>(), api::msg::ERR_INTERNAL);
}

TEST_F(PositionCtrlTest, GetAll_Success) {
    const std::map<int, core::position> expected_deps{
            {1, core::position{1, "Программист"}},
            {2, core::position{2, "Инженер"}},
            {3, core::position{3, "Бухгалтер"}}
    };

    EXPECT_CALL(mock_svc, get_all())
        .Times(1)
        .WillOnce(::testing::Return(expected_deps));

    crow::request req = make_request("positions");
    crow::response resp;

    ctrl->get_all(req, resp);

    EXPECT_EQ(resp.code, 200);

    auto json = parse_response(resp);
    ASSERT_TRUE(json.at("items").is_array());
    EXPECT_EQ(json.at("items").size(), 3);

    // Проверяем первую запись для примера
    EXPECT_EQ(json.at("items")[0].at("id").get<int>(), 1);
    EXPECT_EQ(json.at("items")[0].at("name").get<std::string>(), "Программист");
}

TEST_F(PositionCtrlTest, GetAll_EmptyList) {
    const std::map<int, core::position> empty_deps;

    EXPECT_CALL(mock_svc, get_all())
        .Times(1)
        .WillOnce(::testing::Return(empty_deps));

    crow::request req = make_request("positions");
    crow::response resp;

    ctrl->get_all(req, resp);

    EXPECT_EQ(resp.code, 200);

    auto json = parse_response(resp);
    ASSERT_TRUE(json.at("items").is_array());
    EXPECT_EQ(json.at("items").size(), 0);
}

TEST_F(PositionCtrlTest, GetAll_ThrowDataAccessException) {
    EXPECT_CALL(mock_svc, get_all())
        .Times(1)
        .WillOnce(::testing::Throw(core::data_access_exception("DB connection lost")));

    crow::request req = make_request("positions");
    crow::response resp;

    ctrl->get_all(req, resp);

    EXPECT_EQ(resp.code, 500);

    auto json = parse_response(resp);
    EXPECT_EQ(json.at("error").get<std::string>(), api::msg::ERR_POS_LIST_ERROR);
}


TEST_F(PositionCtrlTest, GetAll_ThrowStdException) {
    EXPECT_CALL(mock_svc, get_all())
        .Times(1)
        .WillOnce(::testing::Throw(std::runtime_error("Unexpected error")));

    crow::request req = make_request("positions");
    crow::response resp;

    ctrl->get_all(req, resp);

    EXPECT_EQ(resp.code, 500);

    auto json = parse_response(resp);
    EXPECT_EQ(json.at("error").get<std::string>(), api::msg::ERR_INTERNAL);
}

TEST_F(PositionCtrlTest, Add_Success) {
    const std::string req_body = R"({"name": "Новая должность"})";
    const core::position input_dept{0, "Новая должность"};      // id=0, т.к. создаётся
    const core::position created_dept{101, "Новая должность"};  // id=101 — вернул сервис

    EXPECT_CALL(mock_svc, create(::testing::Field(&core::position::name, "Новая должность")))
        .Times(1)
        .WillOnce(::testing::Return(created_dept));

    crow::request req = make_request("positions", req_body, crow::HTTPMethod::Post);
    crow::response resp;

    ctrl->add(req, resp);

    EXPECT_EQ(resp.code, 201); // Created

    auto json = parse_response(resp);
    EXPECT_EQ(json.at("id").get<int>(), 101);
    EXPECT_EQ(json.at("name").get<std::string>(), "Новая должность");
}

TEST_F(PositionCtrlTest, Add_InvalidJson) {
    const std::string invalid_body = R"({"name": "unclosed string)";

    crow::request req = make_request("positions", invalid_body, crow::HTTPMethod::Post);
    crow::response resp;

    ctrl->add(req, resp);

    EXPECT_EQ(resp.code, 400);

    auto json = parse_response(resp);
    EXPECT_EQ(json.at("error").get<std::string>(), api::msg::ERR_INVALID_JSON);

    // Сервис не должен быть вызван при невалидном входе
    ::testing::Mock::VerifyAndClearExpectations(&mock_svc);
}

TEST_F(PositionCtrlTest, Add_ValidationException) {
    const std::string req_body = R"({"name": ""})"; // Пустое имя — валидация не пройдёт

    EXPECT_CALL(mock_svc, create(::testing::_))
        .Times(1)
        .WillOnce(::testing::Throw(core::validation_exception("name", "Field cannot be empty")));

    crow::request req = make_request("positions", req_body, crow::HTTPMethod::Post);
    crow::response resp;

    ctrl->add(req, resp);

    EXPECT_EQ(resp.code, 400);

    auto json = parse_response(resp);
    EXPECT_EQ(json.at("error").get<std::string>(), "Field cannot be empty");
}

TEST_F(PositionCtrlTest, Add_ThrowDataAccessException) {
    const std::string req_body = R"({"name": "Должность"})";

    EXPECT_CALL(mock_svc, create(::testing::_))
        .Times(1)
        .WillOnce(::testing::Throw(core::data_access_exception("FK constraint violated")));

    crow::request req = make_request("positions", req_body, crow::HTTPMethod::Post);
    crow::response resp;

    ctrl->add(req, resp);

    EXPECT_EQ(resp.code, 500);

    auto json = parse_response(resp);
    EXPECT_EQ(json.at("error").get<std::string>(), api::msg::ERR_POS_CREATE_FAILED);
}

TEST_F(PositionCtrlTest, Add_ThrowStdException) {
    const std::string req_body = R"({"name": "Должность"})";

    EXPECT_CALL(mock_svc, create(::testing::_))
        .Times(1)
        .WillOnce(::testing::Throw(std::logic_error("Unexpected state")));

    crow::request req = make_request("positions", req_body, crow::HTTPMethod::Post);
    crow::response resp;

    ctrl->add(req, resp);

    EXPECT_EQ(resp.code, 500);

    auto json = parse_response(resp);
    EXPECT_EQ(json.at("error").get<std::string>(), api::msg::ERR_INTERNAL);
}

TEST_F(PositionCtrlTest, Edit_Success) {
    const int pos_id = 42;
    const std::string req_body = R"({"name": "Обновлённая должность"})";
    const core::position updated_dept{pos_id, "Обновлённая должность"};

    EXPECT_CALL(mock_svc, update(pos_id, ::testing::Field(&core::position::name, "Обновлённая должность")))
        .Times(1)
        .WillOnce(::testing::Return(std::make_optional(updated_dept)));

    crow::request req = make_request("positions/42", req_body, crow::HTTPMethod::Put);
    crow::response resp;

    ctrl->edit(req, resp, pos_id);

    EXPECT_EQ(resp.code, 200);

    auto json = parse_response(resp);
    EXPECT_EQ(json.at("id").get<int>(), pos_id);
    EXPECT_EQ(json.at("name").get<std::string>(), "Обновлённая должность");
}

TEST_F(PositionCtrlTest, Edit_NotFound) {
    const int pos_id = 999;
    const std::string req_body = R"({"name": "Не существует"})";

    EXPECT_CALL(mock_svc, update(pos_id, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(std::nullopt));

    crow::request req = make_request("positions/999", req_body, crow::HTTPMethod::Put);
    crow::response resp;

    ctrl->edit(req, resp, pos_id);

    EXPECT_EQ(resp.code, 404);

    auto json = parse_response(resp);
    EXPECT_EQ(json.at("error").get<std::string>(), api::msg::ERR_POS_NOT_FOUND);
}

TEST_F(PositionCtrlTest, Edit_InvalidJson) {
    const int pos_id = 42;
    const std::string invalid_body = R"({"name": invalid})";

    crow::request req = make_request("positions/42", invalid_body, crow::HTTPMethod::Put);
    crow::response resp;

    ctrl->edit(req, resp, pos_id);

    EXPECT_EQ(resp.code, 400);

    auto json = parse_response(resp);
    EXPECT_EQ(json.at("error").get<std::string>(), api::msg::ERR_INVALID_JSON);

    ::testing::Mock::VerifyAndClearExpectations(&mock_svc);
}

TEST_F(PositionCtrlTest, Edit_ValidationException) {
    const int pos_id = 42;
    const std::string req_body = R"({"name": ""})";

    EXPECT_CALL(mock_svc, update(pos_id, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Throw(core::validation_exception("name", "Name is required")));

    crow::request req = make_request("positions/42", req_body, crow::HTTPMethod::Put);
    crow::response resp;

    ctrl->edit(req, resp, pos_id);

    EXPECT_EQ(resp.code, 400);

    auto json = parse_response(resp);
    EXPECT_EQ(json.at("error").get<std::string>(), "Name is required");
}

TEST_F(PositionCtrlTest, Edit_ThrowDataAccessException) {
    const int pos_id = 42;
    const std::string req_body = R"({"name": "Должность"})";

    EXPECT_CALL(mock_svc, update(pos_id, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Throw(core::data_access_exception("Unique constraint failed")));

    crow::request req = make_request("positions/42", req_body, crow::HTTPMethod::Put);
    crow::response resp;

    ctrl->edit(req, resp, pos_id);

    EXPECT_EQ(resp.code, 500);

    auto json = parse_response(resp);
    EXPECT_EQ(json.at("error").get<std::string>(), api::msg::ERR_POS_UPDATE_FAILED);
}

TEST_F(PositionCtrlTest, Edit_ThrowStdException) {
    const int pos_id = 42;
    const std::string req_body = R"({"name": "Должность"})";

    EXPECT_CALL(mock_svc, update(pos_id, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Throw(std::bad_alloc()));

    crow::request req = make_request("positions/42", req_body, crow::HTTPMethod::Put);
    crow::response resp;

    ctrl->edit(req, resp, pos_id);

    EXPECT_EQ(resp.code, 500);

    auto json = parse_response(resp);
    EXPECT_EQ(json.at("error").get<std::string>(), api::msg::ERR_INTERNAL);
}

TEST_F(PositionCtrlTest, Del_Success) {
    const int pos_id = 42;

    EXPECT_CALL(mock_svc, del(pos_id))
        .Times(1)
        .WillOnce(::testing::Return(true));

    crow::request req = make_request("positions/42", "", crow::HTTPMethod::Delete);
    crow::response resp;

    ctrl->del(req, resp, pos_id);

    EXPECT_EQ(resp.code, 204); // No Content
    EXPECT_TRUE(resp.body.empty()); // Тело ответа должно быть пустым
}

TEST_F(PositionCtrlTest, Del_NotFound) {
    const int pos_id = 999;

    EXPECT_CALL(mock_svc, del(pos_id))
        .Times(1)
        .WillOnce(::testing::Return(false));

    crow::request req = make_request("positions/999", "", crow::HTTPMethod::Delete);
    crow::response resp;

    ctrl->del(req, resp, pos_id);

    EXPECT_EQ(resp.code, 404);

    auto json = parse_response(resp);
    EXPECT_EQ(json.at("error").get<std::string>(), api::msg::ERR_POS_NOT_FOUND);
}

TEST_F(PositionCtrlTest, Del_ThrowDataAccessException) {
    const int pos_id = 42;

    EXPECT_CALL(mock_svc, del(pos_id))
        .Times(1)
        .WillOnce(::testing::Throw(core::data_access_exception("Foreign key constraint")));

    crow::request req = make_request("positions/42", "", crow::HTTPMethod::Delete);
    crow::response resp;

    ctrl->del(req, resp, pos_id);

    EXPECT_EQ(resp.code, 500);

    auto json = parse_response(resp);
    EXPECT_EQ(json.at("error").get<std::string>(), api::msg::ERR_POS_DELETE_FAILED);
}

TEST_F(PositionCtrlTest, Del_ThrowStdException) {
    const int pos_id = 42;

    EXPECT_CALL(mock_svc, del(pos_id))
        .Times(1)
        .WillOnce(::testing::Throw(std::exception()));

    crow::request req = make_request("positions/42", "", crow::HTTPMethod::Delete);
    crow::response resp;

    ctrl->del(req, resp, pos_id);

    EXPECT_EQ(resp.code, 500);

    auto json = parse_response(resp);
    EXPECT_EQ(json.at("error").get<std::string>(), api::msg::ERR_INTERNAL);
}
