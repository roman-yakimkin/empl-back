//
// Created by Roman Yakimkin on 21.05.2026.
//

#include "router.h"

void api::router::register_routes(crow::SimpleApp &app) const {
    CROW_ROUTE(app, "/health")([]() {
       return crow::response(200, "OK");
    });

    CROW_ROUTE(app, "/departments").methods(crow::HTTPMethod::GET)([this](const crow::request& req, crow::response& resp) {
        dep_ctrl->get_all(req, resp);
    });
    CROW_ROUTE(app, "/departments/<int>").methods(crow::HTTPMethod::GET)([this](const crow::request& req, crow::response& resp, int id) {
        dep_ctrl->get_one(req, resp, id);
    });
    CROW_ROUTE(app, "/departments").methods(crow::HTTPMethod::POST)([this](const crow::request& req, crow::response& resp) {
        dep_ctrl->add(req, resp);
    });
    CROW_ROUTE(app, "/departments/<int>").methods(crow::HTTPMethod::PUT)([this](const crow::request& req, crow::response& resp, int id) {
        dep_ctrl->edit(req, resp, id);
    });
    CROW_ROUTE(app, "/departments/<int>").methods(crow::HTTPMethod::DELETE)([this](const crow::request& req, crow::response& resp, int id) {
        dep_ctrl->del(req, resp, id);
    });

    CROW_ROUTE(app, "/positions").methods(crow::HTTPMethod::GET)([this](const crow::request& req, crow::response& resp) {
        pos_ctrl->get_all(req, resp);
    });
    CROW_ROUTE(app, "/positions/<int>").methods(crow::HTTPMethod::GET)([this](const crow::request& req, crow::response& resp, int id) {
        pos_ctrl->get_one(req, resp, id);
    });
    CROW_ROUTE(app, "/positions").methods(crow::HTTPMethod::POST)([this](const crow::request& req, crow::response& resp) {
        pos_ctrl->add(req, resp);
    });
    CROW_ROUTE(app, "/positions/<int>").methods(crow::HTTPMethod::PUT)([this](const crow::request& req, crow::response& resp, int id) {
        pos_ctrl->edit(req, resp, id);
    });
    CROW_ROUTE(app, "/positions/<int>").methods(crow::HTTPMethod::DELETE)([this](const crow::request& req, crow::response& resp, int id) {
        pos_ctrl->del(req, resp, id);
    });

    CROW_ROUTE(app, "/employees").methods(crow::HTTPMethod::GET)([this](const crow::request& req, crow::response& resp) {
        empl_ctrl->get_all(req, resp);
    });
    CROW_ROUTE(app, "/employees/<int>").methods(crow::HTTPMethod::GET)([this](const crow::request& req, crow::response& resp, int id) {
        empl_ctrl->get_one(req, resp, id);
    });
    CROW_ROUTE(app, "/employees").methods(crow::HTTPMethod::POST)([this](const crow::request& req, crow::response& resp) {
        empl_ctrl->add(req, resp);
    });
    CROW_ROUTE(app, "/employees/<int>").methods(crow::HTTPMethod::PUT)([this](const crow::request& req, crow::response& resp, int id) {
        empl_ctrl->edit(req, resp, id);
    });
    CROW_ROUTE(app, "/employees/<int>").methods(crow::HTTPMethod::DELETE)([this](const crow::request& req, crow::response& resp, int id) {
        empl_ctrl->del(req, resp, id);
    });
}
