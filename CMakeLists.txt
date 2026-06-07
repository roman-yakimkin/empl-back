cmake_minimum_required(VERSION 3.18.4)
project(empl_back LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

find_package(Threads REQUIRED)
find_package(PostgreSQL REQUIRED)

# Подключение Crow
FetchContent_Declare(
        crow
        GIT_REPOSITORY https://github.com/CrowCpp/Crow.git
        GIT_TAG v1.3.0
)
FetchContent_MakeAvailable(crow)

# Подключение libpqxx
FetchContent_Declare(
        libpqxx
        GIT_REPOSITORY https://github.com/jtv/libpqxx.git
        GIT_TAG        7.10.7
)
FetchContent_MakeAvailable(libpqxx)

# Подключение nlohmann/json
FetchContent_Declare(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.12.0  # актуальная стабильная версия
)
FetchContent_MakeAvailable(nlohmann_json)

# Подключение spdlog
FetchContent_Declare(
        spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG v1.17.0
)
FetchContent_MakeAvailable(spdlog)

add_executable(empl_back src/main.cpp
        src/api/handlers/department_ctrl.cpp
        src/api/handlers/router.cpp
        src/repository/department_repo.cpp
        src/service/department_service.cpp
        src/infrastructure/logger_factory.cpp
        src/infrastructure/config.cpp
        src/helpers/strings.cpp
        src/infrastructure/db_manager.cpp
        src/helpers/envs.cpp
        src/repository/position_repo.cpp
        src/service/position_service.cpp
        src/api/handlers/position_ctrl.cpp
        src/repository/employee_repo.cpp
        src/helpers/pqxx.cpp
        src/service/employee_service.cpp
        src/api/handlers/employee_ctrl.cpp
)

target_link_libraries(${PROJECT_NAME}
        PRIVATE
        Crow::Crow
        nlohmann_json::nlohmann_json
        libpqxx::pqxx
        Threads::Threads
        spdlog::spdlog
)

target_include_directories(empl_back PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Опция для включения тестов (по умолчанию OFF)
option(ENABLE_TESTS "Build unit tests" OFF)

if (ENABLE_TESTS)

    # === Google Test ===
    FetchContent_Declare(
            googletest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG v1.17.0
    )

    # Отключаем OpenSSL, так как для локальных E2E тестов нам нужен только обычный HTTP
    set(HTTPLIB_REQUIRE_OPENSSL OFF CACHE BOOL "" FORCE)
    # Включаем режим компиляции (быстрее и безопаснее, чем header-only)
    set(HTTPLIB_COMPILE ON CACHE BOOL "" FORCE)

    FetchContent_Declare(
            httplib
            GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
            GIT_TAG v0.46.1
    )
    FetchContent_MakeAvailable(httplib)

    # Для Windows:
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    # Включаем тестирование
    enable_testing()

    add_executable(empl_back_tests
            tests/unit/helpers_strings_test.cpp
            src/helpers/strings.cpp
            tests/integration/department_repo_test.cpp
            src/infrastructure/db_manager.cpp
            src/repository/department_repo.cpp
            src/repository/position_repo.cpp
            tests/integration/repo_test.cpp
            src/helpers/envs.cpp
            src/repository/position_repo.cpp
            tests/unit/department_svc_test.cpp
            src/service/department_service.cpp
            src/api/handlers/department_ctrl.cpp
            tests/unit/department_ctrl_test.cpp
            tests/helpers/http.cpp
            tests/unit/ctrl_test.cpp
            src/service/position_service.cpp
            src/api/handlers/position_ctrl.cpp
            tests/integration/position_repo_test.cpp
            tests/unit/position_svc_test.cpp
            tests/unit/position_ctrl_test.cpp
            src/repository/employee_repo.cpp
            src/helpers/pqxx.cpp
            src/service/employee_service.cpp
            src/api/handlers/employee_ctrl.cpp
            tests/integration/employee_repo_test.cpp
            tests/unit/employee_svc_test.cpp
            tests/unit/employee_ctrl_test.cpp
            tests/e2e/employee_test.cpp
            tests/helpers/db_test.cpp
    )

    target_link_libraries(empl_back_tests
            PRIVATE
            GTest::gtest_main
            GTest::gmock
            httplib::httplib
            Crow::Crow
            nlohmann_json::nlohmann_json
            libpqxx::pqxx
            Threads::Threads
            spdlog::spdlog
    )

    target_include_directories(empl_back_tests PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/src
            ${CMAKE_CURRENT_SOURCE_DIR}/tests
    )

    include(GoogleTest)
    gtest_discover_tests(empl_back_tests)

endif()