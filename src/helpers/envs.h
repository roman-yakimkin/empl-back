//
// Created by Roman Yakimkin on 30.05.2026.
//

#pragma once

#include <string>

namespace helpers {
    std::string get_env(const char* key, const char* default_val);
}