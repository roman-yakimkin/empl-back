//
// Created by Roman Yakimkin on 21.05.2026.
//

#include "department_ctrl.h"

#include "api/dto/department_dto.h"
#include "api/helpers/http_helpers.h"
#include "api/helpers/msg.h"
#include "core/errors/data_access_exception.h"
#include "core/errors/validation_exception.h"

// Получить все записи
void api::department_ctrl::get_all(const crow::request &req, crow::response &resp) const {
    try {
        auto deps = dep_svc->get_all();

        department_response_list dto(deps);
        nlohmann::json j = dto;

        helpers::send_json(resp, 200, j);
    }
    catch (const core::data_access_exception& e) {
        // Ошибка БД — внутренняя проблема
        logger->error("Database error during getting dep list: {}", e.what());
        helpers::send_error(resp, 500, msg::ERR_DEP_LIST_ERROR);
    }
    catch (const std::exception& e) {
        // Любое другое непредвиденное исключение
        logger->error("Unexpected error in get_all(): {}", e.what());
        helpers::send_error(resp, 500, msg::ERR_INTERNAL);
    }
}

// Получить одну запись по id
void api::department_ctrl::get_one(const crow::request &req, crow::response &resp, int id) const {
    try {
        auto dep = dep_svc->get_one(id);

        if (!dep) {
            helpers::send_error(resp, 404, msg::ERR_DEP_NOT_FOUND);
            return;
        }

        department_response dto(*dep);

        nlohmann::json j = dto;
        helpers::send_json(resp, 200, j);
    }
    catch (const core::data_access_exception& e) {
        // Ошибка БД — внутренняя проблема
        logger->error("Database error during getting dep: {}", e.what());
        helpers::send_error(resp, 500, msg::ERR_DEP_DATA_ERROR);
    }
    catch (const std::exception& e) {
        // Любое другое непредвиденное исключение
        logger->error("Unexpected error in get_one(): {}", e.what());
        helpers::send_error(resp, 500, msg::ERR_INTERNAL);
    }
}

// Добавить запись
void api::department_ctrl::add(const crow::request &req, crow::response &resp) const {
    try {
        auto body_json = nlohmann::json::parse(req.body);
        auto req_dto = body_json.get<department_request>();

        auto dep = dep_svc->create(core::department{0, req_dto.name});

        department_response resp_dto(dep);

        nlohmann::json j = resp_dto;
        helpers::send_json(resp, 201, j);
    }
    catch (const nlohmann::json::exception& e) {
        logger->warn("Invalid JSON in request body: {}", e.what());
        helpers::send_error(resp, 400, msg::ERR_INVALID_JSON);
    }
    catch (const core::validation_exception& e) {
        logger->debug("Validation failed: field='{}', message='{}'", e.field(), e.what());
        helpers::send_error(resp, 400, e.what());
    }
    catch (const core::data_access_exception& e) {
        // Ошибка БД — внутренняя проблема
        logger->error("Database error during create: {}", e.what());
        helpers::send_error(resp, 500, msg::ERR_DEP_CREATE_FAILED);
    }
    catch (const std::exception& e) {
        // Любое другое непредвиденное исключение
        logger->error("Unexpected error in add(): {}", e.what());
        helpers::send_error(resp, 500, msg::ERR_INTERNAL);
    }
}

// Редактировать запись
void api::department_ctrl::edit(const crow::request &req, crow::response &resp, int id) const {
    try {
        auto body_json = nlohmann::json::parse(req.body);
        auto req_dto = body_json.get<department_request>();

        auto dep = dep_svc->update(id, core::department{0, req_dto.name});
        if (!dep) {
            helpers::send_error(resp, 404, msg::ERR_DEP_NOT_FOUND);
            return;
        }

        department_response resp_dto(*dep);

        nlohmann::json j = resp_dto;
        helpers::send_json(resp, 200, j);
    }
    catch (const nlohmann::json::exception& e) {
        logger->warn("Invalid JSON in request body: {}", e.what());
        helpers::send_error(resp, 400, msg::ERR_INVALID_JSON);
    }
    catch (const core::validation_exception& e) {
        logger->debug("Validation failed: field='{}', message='{}'", e.field(), e.what());
        helpers::send_error(resp, 400, e.what());
    }
    catch (const core::data_access_exception& e) {
        // Ошибка БД — внутренняя проблема
        logger->error("Database error during update: {}", e.what());
        helpers::send_error(resp, 500, msg::ERR_DEP_UPDATE_FAILED);
    }
    catch (const std::exception& e) {
        // Любое другое непредвиденное исключение
        logger->error("Unexpected error in edit(): {}", e.what());
        helpers::send_error(resp, 500, msg::ERR_INTERNAL);
    }
}

// Удалить запись
void api::department_ctrl::del(const crow::request &req, crow::response &resp, int id) const {
    try {
        auto result = dep_svc->del(id);
        if (result == false) {
            helpers::send_error(resp, 404, msg::ERR_DEP_NOT_FOUND);
            return;
        }

        helpers::send_no_content(resp, 204);
    }
    catch (const core::validation_exception& e) {
        logger->debug("Validation failed: field='{}', message='{}'", e.field(), e.what());
        helpers::send_error(resp, 400, e.what());
    }
    catch (const core::data_access_exception& e) {
        // Ошибка БД — внутренняя проблема
        logger->error("Database error during delete: {}", e.what());
        helpers::send_error(resp, 500, msg::ERR_DEP_DELETE_FAILED);
    }
    catch (const std::exception& e) {
        // Любое другое непредвиденное исключение
        logger->error("Unexpected error in del(): {}", e.what());
        helpers::send_error(resp, 500, msg::ERR_INTERNAL);
    }
}
