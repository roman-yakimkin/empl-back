//
// Created by Roman Yakimkin on 30.05.2026.
//

#include "envs.h"
#include <cstdlib>

namespace helpers {
    std::string get_env(const char* key, const char* default_val) {
        const char* val = std::getenv(key);
        return val ? val : default_val;
    }
}

