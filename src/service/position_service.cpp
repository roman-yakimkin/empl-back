//
// Created by Roman Yakimkin on 01.06.2026.
//

#include "position_service.h"
#include "core/errors/validation_exception.h"

namespace svc {
    std::optional<core::position> position_service::get_one(const int id) const {
        return repo->get_one(id);
    }

    std::map<int, core::position> position_service::get_all() const {
        return repo->get_all();
    }

    core::position position_service::create(const core::position &dep) {
        if (dep.name.empty()) {
            logger->warn("Validation failed: position name is empty");
            throw core::validation_exception("name", "Название должности не может быть пустым");
        }

        return repo->create(dep);
    }

    std::optional<core::position> position_service::update(int id, const core::position &dep) {
        if (dep.name.empty()) {
            logger->warn("Validation failed: position name is empty");
            throw core::validation_exception("name", "Название должности не может быть пустым");
        }

        return repo->update(id, dep);
    }

    bool position_service::del(const int id) {
        try {
            return repo->del(id);
        }
        catch (const core::validation_exception& e) {
            throw core::validation_exception("position_id", "Попытка удалить должность, к которой привязаны сотрудники");
        }
    }

} // svc