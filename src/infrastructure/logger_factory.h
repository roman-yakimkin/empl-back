//
// Created by Roman Yakimkin on 22.05.2026.
//

#pragma once

#include <spdlog/spdlog.h>
#include <memory>

namespace infra {
    std::shared_ptr<spdlog::logger> create_app_logger();
}
