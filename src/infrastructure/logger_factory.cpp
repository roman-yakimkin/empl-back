//
// Created by Roman Yakimkin on 22.05.2026.
//

#include "logger_factory.h"

#include <spdlog/async.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/cfg/env.h>

namespace infra {
    std::shared_ptr<spdlog::logger> create_app_logger() {
        spdlog::init_thread_pool(8192, 1);

        auto sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        auto logger = std::make_shared<spdlog::async_logger>(
            "ms", sink, spdlog::thread_pool(), spdlog::async_overflow_policy::overrun_oldest
        );
        logger->set_pattern("%Y-%m-%dT%H:%M:%S.%f [%^%l%$] [%n] %v");

        spdlog::register_logger(logger);
        spdlog::set_default_logger(logger);
        spdlog::cfg::load_env_levels();

        return logger;
    }
}
