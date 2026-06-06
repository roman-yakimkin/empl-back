#include <iostream>

#include "api/handlers/employee_ctrl.h"
#include "api/handlers/router.h"
#include "crow/app.h"
#include "infrastructure/config.h"
#include "infrastructure/db_manager.h"
#include "infrastructure/logger_factory.h"
#include "repository/department_repo.h"
#include "repository/position_repo.h"
#include "repository/employee_repo.h"
#include "service/department_service.h"
#include "service/position_service.h"
#include "service/employee_service.h"

int main(int argc, char** argv) {
    std::cout << "Service is starting" << std::endl;

    // Загружаем конфиг
    const auto cfg = infra::config::load(argc, argv);
    if (!cfg) {
        std::cerr << "configuration validation failed. Exiting ...\n";

        return 1;
    }

    cfg->log_summary();

    // Подключаем логгер
    std::shared_ptr<spdlog::logger> logger;

    try {
        logger = infra::create_app_logger();
    }
    catch (std::exception& e) {
        std::cerr << "unable to create logger. " << e.what() << " Exiting ...\n";

        return 1;
    }

    try {
        // Инициализируем соединение с БД
        const std::string conn_string =
            " host=" + cfg->db.host +
            " port=" + std::to_string(cfg->db.port) +
            " user=" + cfg->db.user +
            " password=" + cfg->db.password+
            " dbname=" + cfg->db.name +
            " sslmode=disable";
        auto db_manager = std::make_shared<infra::db_manager>(infra::db_manager(conn_string, logger.get()));

        // Инициализируем репозитории
        auto dep_repo = std::make_shared<repo::department_repo>(repo::department_repo(db_manager.get(), logger.get()));
        auto pos_repo = std::make_shared<repo::position_repo>(repo::position_repo(db_manager.get(), logger.get()));
        auto empl_repo = std::make_shared<repo::employee_repo>(repo::employee_repo(db_manager.get(), logger.get()));

        // Инициализируем сервисы
        auto dep_service = std::make_shared<svc::department_service>(svc::department_service(dep_repo.get(), logger.get()));
        auto pos_service = std::make_shared<svc::position_service>(svc::position_service(pos_repo.get(), logger.get()));
        auto empl_service = std::make_shared<svc::employee_service>(svc::employee_service(empl_repo.get(), logger.get()));

        // Инициализируем контроллеры
        auto dep_ctrl = std::make_shared<api::department_ctrl>(api::department_ctrl(dep_service.get(), logger.get()));
        auto pos_ctrl = std::make_shared<api::position_ctrl>(api::position_ctrl(pos_service.get(), logger.get()));
        auto empl_ctrl = std::make_shared<api::employee_ctrl>(api::employee_ctrl(empl_service.get(), logger.get()));

        crow::SimpleApp app;

        auto router = api::router(
            dep_ctrl.get(),
            pos_ctrl.get(),
            empl_ctrl.get()
        );
        router.register_routes(app);

        logger->info("Server starting on port {}", cfg->server.port);
        app.port(cfg->server.port).multithreaded().run();
        logger->info("Server stopped gracefully");
    }
    catch (std::invalid_argument& e) {
        logger->critical("Application initialization error: {}. Exiting ...", e.what());
    }
    catch (std::exception& e) {
        logger->critical("Server terminated with error: {}", e.what());
        return 1;
    }

    spdlog::shutdown();
    return 0;
}
