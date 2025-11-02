#pragma once
#include <string>
struct HttpResult {
    bool ok = false;
    long status = 0;
    std::string body;
    std::string err;
};
HttpResult http_get(const std::string& url);

