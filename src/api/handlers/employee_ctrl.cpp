//
// Created by Roman Yakimkin on 03.06.2026.
//

#include "employee_ctrl.h"

#include <nlohmann/json.hpp>

#include "api/dto/employee_dto.h"
#include "api/helpers/msg.h"
#include "api/helpers/http_helpers.h"
#include "core/errors/data_access_exception.h"
#include "core/errors/validation_exception.h"

namespace api {
    void employee_ctrl::get_one(const crow::request &req, crow::response &resp, int id) const {
        try {
            auto empl = empl_svc->get_one(id);

            if (!empl) {
                helpers::send_error(resp, 404, msg::ERR_EMPL_NOT_FOUND);
                return;
            }

            employee_detail_response dto(*empl);

            nlohmann::json j = dto;
            helpers::send_json(resp, 200, j);
        }
        catch (const core::data_access_exception& e) {
            // Ошибка БД — внутренняя проблема
            logger->error("Database error during getting employee: {}", e.what());
            helpers::send_error(resp, 500, msg::ERR_EMPL_DATA_ERROR);
        }
        catch (const std::exception& e) {
            // Любое другое непредвиденное исключение
            logger->error("Unexpected error in get_one(): {}", e.what());
            helpers::send_error(resp, 500, msg::ERR_INTERNAL);
        }
    }

    void employee_ctrl::get_all(const crow::request &req, crow::response &resp) const {
        try {
            core::filters::employee params;

            if (auto val = req.url_params.get("department_id"); val) {
                int id = 0;
                if (auto [ptr, ec] = std::from_chars(val, val + strlen(val), id); ec != std::errc() || ptr != val + strlen(val)) {
                    helpers::send_error(resp, 400, msg::ERR_EMPL_INVALID_DEP_ID_FORMAT);
                    return;
                }
                if (id <= 0) {
                    helpers::send_error(resp, 400, msg::ERR_EMPL_DEP_ID_MUST_BE_POSITIVE);
                    return;
                }
                params.department_id = id;
             }

            if (auto val = req.url_params.get("position_id"); val) {
                int id = 0;
                if (auto [ptr, ec] = std::from_chars(val, val + strlen(val), id); ec != std::errc() || ptr != val + strlen(val)) {
                    helpers::send_error(resp, 400, msg::ERR_EMPL_INVALID_POS_ID_FORMAT);
                    return;
                }
                if (id <= 0) {
                    helpers::send_error(resp, 400, msg::ERR_EMPL_POS_ID_MUST_BE_POSITIVE);
                    return;
                }
                params.position_id = id;
            }

            if (auto val = req.url_params.get("name"); val)
                params.name_pattern = std::string(val);

            auto empl = empl_svc->get_all(params);

            employee_detail_response_list dto(empl);
            nlohmann::json j = dto;

            helpers::send_json(resp, 200, j);
        }
        catch (const core::data_access_exception& e) {
            // Ошибка БД — внутренняя проблема
            logger->error("Database error during getting empl list: {}", e.what());
            helpers::send_error(resp, 500, msg::ERR_EMPL_LIST_ERROR);
        }
        catch (const std::exception& e) {
            // Любое другое непредвиденное исключение
            logger->error("Unexpected error in get_all(): {}", e.what());
            helpers::send_error(resp, 500, msg::ERR_INTERNAL);
        }
    }

    void employee_ctrl::add(const crow::request &req, crow::response &resp) const {
        try {
            auto body_json = nlohmann::json::parse(req.body);
            auto req_dto = body_json.get<employee_request>();

            auto empl = empl_svc->create(core::employee{
                req_dto.name,
                req_dto.department_id,
                req_dto.position_id
            });

            employee_response resp_dto(empl);

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
            helpers::send_error(resp, 500, msg::ERR_EMPL_CREATE_FAILED);
        }
        catch (const std::exception& e) {
            // Любое другое непредвиденное исключение
            logger->error("Unexpected error in add(): {}", e.what());
            helpers::send_error(resp, 500, msg::ERR_INTERNAL);
        }
    }

    void employee_ctrl::edit(const crow::request &req, crow::response &resp, int id) const {
        try {
            auto body_json = nlohmann::json::parse(req.body);
            auto req_dto = body_json.get<employee_request>();

            auto empl = empl_svc->update(id, core::employee{
                req_dto.name,
                req_dto.department_id,
                req_dto.position_id
            });
            if (!empl) {
                helpers::send_error(resp, 404, msg::ERR_EMPL_NOT_FOUND);
                return;
            }

            employee_response resp_dto(*empl);

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
            helpers::send_error(resp, 500, msg::ERR_EMPL_UPDATE_FAILED);
        }
        catch (const std::exception& e) {
            // Любое другое непредвиденное исключение
            logger->error("Unexpected error in edit(): {}", e.what());
            helpers::send_error(resp, 500, msg::ERR_INTERNAL);
        }
    }

    void employee_ctrl::del(const crow::request &req, crow::response &resp, int id) const {
        try {
            auto result = empl_svc->del(id);
            if (result == false) {
                helpers::send_error(resp, 404, msg::ERR_EMPL_NOT_FOUND);
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
            helpers::send_error(resp, 500, msg::ERR_EMPL_DELETE_FAILED);
        }
        catch (const std::exception& e) {
            // Любое другое непредвиденное исключение
            logger->error("Unexpected error in del(): {}", e.what());
            helpers::send_error(resp, 500, msg::ERR_INTERNAL);
        }
    }
} // api