//
// Created by Roman Yakimkin on 23.05.2026.
//

#include "config.h"
#include <fstream>
#include <filesystem>
#include <cstdlib>
#include <helpers/strings.h>

namespace infra {

    // Проверка запуска в контейнере (Docker/K8s)
    bool config::is_running_in_container() {
        return std::filesystem::exists("/.dockerenv") ||
               (std::getenv("KUBERNETES_SERVICE_HOST") != nullptr);
    }

    // Парсер .env-файла
    std::unordered_map<std::string, std::string> config::parse_env_file(const std::string& path) {
        std::unordered_map<std::string, std::string> env;
        std::ifstream file(path);
        if (!file) return env; // Файл не найден — не ошибка

        std::string line;
        while (std::getline(file, line)) {
            line = helpers::trim(line);
            if (line.empty() || line[0] == '#') continue;

            auto eq = line.find('=');
            if (eq == std::string::npos) continue;

            std::string key = helpers::trim(line.substr(0, eq));
            std::string value = helpers::trim(line.substr(eq + 1));

            // Убираем обрамляющие кавычки
            if (value.size() >= 2 &&
               ((value.front() == '"' && value.back() == '"') ||
                (value.front() == '\'' && value.back() == '\''))) {
                value = value.substr(1, value.size() - 2);
            }

            // Игнорируем inline-комментарии (неэкранированные #)
            if (auto comment = value.find('#'); comment != std::string::npos) {
                value = helpers::trim(value.substr(0, comment));
            }

            if (!key.empty()) {
                env[key] = value;
            }
        }
        return env;
    }

    // === Основной метод загрузки ===
    std::optional<config> config::load(int argc, char** argv, const std::string& env_path) {
        config cfg;

        // 1. Парсим CLI: --key=value или --key value
        std::unordered_map<std::string, std::string> cli_args;
        for (int i = 1; i < argc; ++i) {
            std::string arg(argv[i]);
            if (arg.rfind("--", 0) == 0) {
                arg = arg.substr(2);
                auto eq = arg.find('=');
                if (eq != std::string::npos) {
                    cli_args[arg.substr(0, eq)] = arg.substr(eq + 1);
                } else if (i + 1 < argc) {
                    cli_args[arg] = argv[++i];
                } else {
                    cli_args[arg] = "true"; // --flag без значения = true
                }
            }
        }

        // 2. Загружаем .env ТОЛЬКО если НЕ в контейнере
        std::unordered_map<std::string, std::string> file_env;
        bool in_container = is_running_in_container();

        if (!in_container) {
            file_env = parse_env_file(env_path);

            // Опционально: .env.local имеет приоритет над .env
            auto local_env = parse_env_file(".env.local");
            for (const auto& [k, v] : local_env) {
                file_env[k] = v;
            }
        }

        // Хелпер для вызова resolve без дублирования кода
        auto get = [&](const std::string& env_key, const std::string& cli_key, const auto& default_val) {
            std::string cli_val;
            if (auto it = cli_args.find(cli_key); it != cli_args.end()) cli_val = it->second;
            return cfg.resolve(env_key, cli_val, file_env, default_val);
        };

        // 3. Заполняем поля по цепочке приоритетов
        cfg.db.driver   = get("DB_DRIVER",   "db-driver",   std::string("postgres"));
        cfg.db.host     = get("DB_HOST",     "db-host",     std::string("localhost"));
        cfg.db.port     = get("DB_PORT",     "db-port",     5432);
        cfg.db.user     = get("DB_USER",     "db-user",     std::string("postgres"));
        cfg.db.password = get("DB_PASSWORD", "db-password", std::string(""));
        cfg.db.name     = get("DB_NAME",     "db-name",     std::string("mydb"));

        cfg.server.port       = get("SERVER_PORT",   "server-port", 8080);

        cfg.features.is_production = get("IS_PRODUCTION", "prod", false);

        // 4. Валидация перед стартом
        if (!cfg.validate()) {
            return std::nullopt;
        }

        return cfg;
    }

//  Валидация
    bool config::validate(std::ostream& log) {
        bool ok = true;

        if (db.host.empty()) {
            log << "ERROR: DB_HOST cannot be empty\n";
            ok = false;
        }
        if (db.port < 1 || db.port > 65535) {
            log << "ERROR: DB_PORT out of valid range (1-65535)\n";
            ok = false;
        }
        if (db.driver != "postgres" && db.driver != "mysql" && db.driver != "sqlite") {
            log << "WARNING: Unknown DB_DRIVER: '" << db.driver << "'\n";
        }

        if (features.is_production) {
            if (db.password.empty()) {
                log << "ERROR: DB_PASSWORD is required in production mode\n";
                ok = false;
            }
        }

        if (server.port < 1 || server.port > 65535) {
            log << "ERROR: SERVER_PORT out of valid range\n";
            ok = false;
        }

        return ok;
    }

// Безопасное логирование
    void config::log_summary(std::ostream& out) const {
        out << "=== Configuration Summary ===\n";
        out << "DB: " << db.driver << "://" << db.user << "@"
            << db.host << ":" << db.port << "/" << db.name << "\n";
        out << "  Password: ***\n";  // ⚠️ Секрет маскируется

        out << "Server: " << server.port << "\n";

        out << "Mode: " << (features.is_production ? "PRODUCTION" : "DEVELOPMENT") << "\n";
        out << "=============================\n";
    }

}

