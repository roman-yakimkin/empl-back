//
// Created by Roman Yakimkin on 29.05.2026.
//

#pragma once

namespace api::msg {
    constexpr auto ERR_DEP_NOT_FOUND = "Отдел не найден";
    constexpr auto ERR_DEP_LIST_ERROR = "Ошибка при получении списка отделов";
    constexpr auto ERR_DEP_DATA_ERROR = "Ошибка при получении данных отдела";
    constexpr auto ERR_DEP_CREATE_FAILED = "Невозможно добавить отдел";
    constexpr auto ERR_DEP_UPDATE_FAILED = "Невозможно изменить отдел";
    constexpr auto ERR_DEP_DELETE_FAILED = "Невозможно удалить отдел";

    constexpr auto ERR_POS_NOT_FOUND = "Должность не найдена";
    constexpr auto ERR_POS_LIST_ERROR = "Ошибка при получении списка должностей";
    constexpr auto ERR_POS_DATA_ERROR = "Ошибка при получении данных должности";
    constexpr auto ERR_POS_CREATE_FAILED = "Невозможно добавить должность";
    constexpr auto ERR_POS_UPDATE_FAILED = "Невозможно изменить должность";
    constexpr auto ERR_POS_DELETE_FAILED = "Невозможно удалить должность";

    constexpr auto ERR_EMPL_NOT_FOUND = "Сотрудник не найден";
    constexpr auto ERR_EMPL_LIST_ERROR = "Ошибка при получении списка сотрудников";
    constexpr auto ERR_EMPL_DATA_ERROR = "Ошибка при получении данных сотрудника";
    constexpr auto ERR_EMPL_CREATE_FAILED = "Невозможно добавить сотрудника";
    constexpr auto ERR_EMPL_UPDATE_FAILED = "Невозможно изменить сотрудника";
    constexpr auto ERR_EMPL_DELETE_FAILED = "Невозможно удалить сотрудника";
    constexpr auto ERR_EMPL_INVALID_DEP_ID_FORMAT = "Неверный формат department_id в запросе";
    constexpr auto ERR_EMPL_DEP_ID_MUST_BE_POSITIVE = "department_id в запросе должен быть положительным";
    constexpr auto ERR_EMPL_INVALID_POS_ID_FORMAT = "Неверный формат position_id в запросе";
    constexpr auto ERR_EMPL_POS_ID_MUST_BE_POSITIVE = "position_id в запросе должен быть положительным";

    constexpr auto ERR_INTERNAL = "Внутренняя ошибка сервера";
    constexpr auto ERR_INVALID_JSON = "Неверный формат JSON";
    constexpr auto ERR_VALIDATION = "Ошибка валидации";
}