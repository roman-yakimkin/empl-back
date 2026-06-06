//
// Created by Roman Yakimkin on 03.06.2026.
//

#include "employee_service.h"

#include "core/errors/validation_exception.h"

namespace svc {
    void employee_service::check_employee(const core::employee &e) const {
        if (e.name.empty()) {
            logger->warn("Validation failed: employee name is empty");
            throw core::validation_exception("name", "Имя сотрудника не может быть пустым");
        }

        if (!e.department_id) {
            logger->warn("Validation failed: department is empty");
            throw core::validation_exception("department_id", "Отдел сотрудника должен быть задан");
        }

        if (!e.position_id) {
            logger->warn("Validation failed: position is empty");
            throw core::validation_exception("position_id", "Должность сотрудника должна быть задана");
        }
    }

    std::optional<core::employee_detail> employee_service::get_one(int id) const {
        return repo->get_one(id);
    }

    std::vector<core::employee_detail> employee_service::get_all(const core::filters::employee &params) const {
        return repo->get_all(params);
    }

    core::employee employee_service::create(const core::employee &e) {
        check_employee(e);

        return repo->create(e);
    }

    std::optional<core::employee> employee_service::update(int id, const core::employee &e) {
        check_employee(e);

        return repo->update(id, e);
    }

    bool employee_service::del(int id) {
        return repo->del(id);
    }
} // svc