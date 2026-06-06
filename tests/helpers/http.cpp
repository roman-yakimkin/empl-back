//
// Created by Roman Yakimkin on 31.05.2026.
//

#include "http.h"

crow::request make_request(const std::string& url, const std::string& body, crow::HTTPMethod method) {
    crow::request req;
    req.body = body;
    req.url = url;
    req.method = method;
    req.url_params = crow::query_string(url);

    return req;
}