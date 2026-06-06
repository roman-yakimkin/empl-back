//
// Created by Roman Yakimkin on 21.05.2026.
//

#include "department_service.h"
#include "core/errors/validation_exception.h"

namespace svc {
    std::optional<core::department> department_service::get_one(const int id) const {
        return repo->get_one(id);
    }

    std::map<int, core::department> department_service::get_all() const {
        return repo->get_all();
    }

    core::department department_service::create(const core::department &dep) {
        if (dep.name.empty()) {
            logger->warn("Validation failed: department name is empty");
            throw core::validation_exception("name", "Название отдела не может быть пустым");
        }

        return repo->create(dep);
    }

    std::optional<core::department> department_service::update(int id, const core::department &dep) {
        if (dep.name.empty()) {
            logger->warn("Validation failed: department name is empty");
            throw core::validation_exception("name", "Название отдела не может быть пустым");
        }

        return repo->update(id, dep);
    }

    bool department_service::del(const int id) {
        try {
            return repo->del(id);
        }
        catch (const core::validation_exception& e) {
            throw core::validation_exception("department_id", "Попытка удалить отдел, к которому привязаны сотрудники");
        }
    }
} // svc