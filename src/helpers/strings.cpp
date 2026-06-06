//
// Created by Roman Yakimkin on 24.05.2026.
//

#include "strings.h"

namespace helpers {
    std::string trim(const std::string& s) {
        auto start = s.find_first_not_of(" \t\r\n");
        if (start == std::string::npos)
            return "";
        auto end = s.find_last_not_of(" \t\r\n");
        return s.substr(start, end - start + 1);
    }

    std::string escape_like_pattern(const std::string& s) {
        std::string result;
        result.reserve(s.size() + s.size() / 4);
        for (char c : s) {
            switch (c) {
                case '\\': result += "\\\\"; break;
                case '%':  result += "\\%";  break;
                case '_':  result += "\\_";  break;
                default:   result += c;      break;
            }
        }
        return result;
    }
}

