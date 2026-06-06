//
// Created by Roman Yakimkin on 31.05.2026.
//

#include "ctrl_test.h"

void ctrl_test::set_up() {
    log_sink = std::make_shared<spdlog::sinks::null_sink_mt>();
    test_logger = std::make_shared<spdlog::logger>("ctrl_test", log_sink);
    test_logger->set_level(spdlog::level::off);
}

void ctrl_test::tear_down() {
    test_logger.reset();
    log_sink.reset();
}
