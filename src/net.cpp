#include "net.hpp"
#include <curl/curl.h>
#include <string>

static size_t write_cb(char* ptr, size_t size, size_t nmemb, void* userdata) {
    auto* s = reinterpret_cast<std::string*>(userdata);
    s->append(ptr, size * nmemb);
    return size * nmemb;
}

HttpResult http_get(const std::string& url) {
    HttpResult res;
    CURL* c = curl_easy_init();
    if (!c) { res.err = "curl_easy_init failed"; return res; }

    curl_easy_setopt(c, CURLOPT_URL, url.c_str());
    curl_easy_setopt(c, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(c, CURLOPT_USERAGENT, "ImGuiVulkanBrowser/1.0");
    curl_easy_setopt(c, CURLOPT_TIMEOUT, 15L);
    curl_easy_setopt(c, CURLOPT_WRITEFUNCTION, write_cb);
    curl_easy_setopt(c, CURLOPT_WRITEDATA, &res.body);
    curl_easy_setopt(c, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(c, CURLOPT_SSL_VERIFYHOST, 2L);

    CURLcode code = curl_easy_perform(c);
    if (code != CURLE_OK) {
        res.err = curl_easy_strerror(code);
        curl_easy_cleanup(c);
        return res;
    }
    curl_easy_getinfo(c, CURLINFO_RESPONSE_CODE, &res.status);
    res.ok = (res.status >= 200 && res.status < 300);
    if (!res.ok && res.err.empty()) res.err = "HTTP " + std::to_string(res.status);
    curl_easy_cleanup(c);
    return res;
}

