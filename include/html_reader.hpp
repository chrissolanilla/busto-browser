#pragma once
#include <string>
#include <vector>

struct LinkSpan { int start, end; std::string url; };

std::string html_to_text_basic(const std::string& html, std::vector<LinkSpan>& links);
void render_wrapped_with_links(const std::string& text, const std::vector<LinkSpan>& links);

