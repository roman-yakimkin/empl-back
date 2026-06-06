//
// Created by Roman Yakimkin on 23.05.2026.
//

#pragma once

#include <algorithm>
#include <string>
#include <cstdint>
#include <iostream>
#include <optional>
#include <unordered_map>

namespace infra {
    class config {
    public:
        struct {
            std::string driver;
            std::string host;
            uint16_t port;
            std::string user;
            std::string password;
            std::string name;
        } db = {};

        struct {
            uint16_t port;
        } server = {};

        struct {
            bool is_production;
        } features = {};

        static std::optional<config> load(
             int argc = 0,
             char** argv = nullptr,
             const std::string& env_path = ".env"
         );

        bool validate(std::ostream& log = std::cerr);
        void log_summary(std::ostream& out = std::cout) const;
    private:
        config() = default;
        static std::unordered_map<std::string, std::string> parse_env_file(const std::string& path);
        static bool is_running_in_container();

        template<typename T>
        static T resolve(
            const std::string& env_key,
            const std::string& cli_value,
            const std::unordered_map<std::string, std::string>& file_env,
            const T& default_val
        );
    };

    template<typename T>
    T config::resolve(const std::string& env_key,
                      const std::string& cli_value,
                      const std::unordered_map<std::string, std::string>& file_env,
                      const T& default_val) {

        // 1. CLI аргументы (наивысший приоритет)
        if (!cli_value.empty()) {
            if constexpr (std::is_same_v<T, std::string>) return cli_value;
            if constexpr (std::is_same_v<T, int>)         { try { return std::stoi(cli_value); } catch (...) {} }
            if constexpr (std::is_same_v<T, double>)      { try { return std::stod(cli_value); } catch (...) {} }
            if constexpr (std::is_same_v<T, bool>)        {
                std::string lower = cli_value;
                std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
                return (lower == "true" || lower == "1" || lower == "yes");
            }
        }

        // 2. Переменные окружения процесса (Docker/K8s/systemd)
        if (const char* val = std::getenv(env_key.c_str())) {
            std::string s(val);
            if constexpr (std::is_same_v<T, std::string>) return s;
            if constexpr (std::is_same_v<T, int>)         { try { return std::stoi(s); } catch (...) {} }
            if constexpr (std::is_same_v<T, double>)      { try { return std::stod(s); } catch (...) {} }
            if constexpr (std::is_same_v<T, bool>)        {
                std::string lower = s;
                std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
                return (lower == "true" || lower == "1" || lower == "yes");
            }
        }

        // 3. Значение из .env-файла
        auto it = file_env.find(env_key);
        if (it != file_env.end()) {
            const std::string& s = it->second;
            if constexpr (std::is_same_v<T, std::string>) return s;
            if constexpr (std::is_same_v<T, int>)         { try { return std::stoi(s); } catch (...) {} }
            if constexpr (std::is_same_v<T, double>)      { try { return std::stod(s); } catch (...) {} }
            if constexpr (std::is_same_v<T, bool>)        {
                std::string lower = s;
                std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
                return (lower == "true" || lower == "1" || lower == "yes");
            }
        }

        // 4. Дефолтное значение
        return default_val;
    }
}
