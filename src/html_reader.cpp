#include "html_reader.hpp"
#include <algorithm>
#include <string.h>

static void erase_between(std::string& s, const char* open, const char* close) {
    for (;;) {
        auto a = strcasestr(s.c_str(), open);
        if (!a) break;
        auto b = strcasestr(a + strlen(open), close);
        if (!b) { s.erase(a - s.c_str()); break; }
        s.erase(a - s.c_str(), (b - a) + strlen(close));
    }
}

static void replace_all_ci(std::string& s, const char* from, const char* to) {
    for (;;) {
        auto p = strcasestr(s.c_str(), from);
        if (!p) break;
        size_t off = p - s.c_str();
        s.replace(off, strlen(from), to);
    }
}

std::string html_to_text_basic(const std::string& in, std::vector<LinkSpan>& links) {
    std::string s = in;

    //fuck script tags
    erase_between(s, "<script", "</script>");
    erase_between(s, "<style",  "</style>");
    //put new lines on tags
    const char* breaks[] = {"<br>", "<br/>", "<br />", "<p>", "</p>", "</div>", "</h1>", "</h2>", "</h3>", "</li>"};
    for (auto* t : breaks){
		replace_all_ci(s, t, "\n");
	}
	replace_all_ci(s, "<h1>", "\n# ");
	replace_all_ci(s, "<h2>", "\n## ");
	replace_all_ci(s, "<h3>", "\n### ");

    //bullet points for these tags
    replace_all_ci(s, "<ul>",  "\n");
    replace_all_ci(s, "<ol>",  "\n");
    replace_all_ci(s, "<li>",  "• ");
    //for anchor tags somehow do this later idk
    links.clear();
    //super-naive lmao i guess find <a ... href="...">text</a>
    for (;;) {
        auto a = strcasestr(s.c_str(), "<a ");
        if (!a) break;
        auto href = strcasestr(a, "href=");
        auto gt = strchr(a, '>');
        if (!href || !gt) break;
        char quote = (*(href+5) == '\"' || *(href+5) == '\'') ? *(href+5) : 0;
        const char* url_start = quote ? href+6 : href+5;
        const char* url_end   = quote ? strchr(url_start, quote) : strpbrk(url_start, " >");
        if (!url_end) break;
        const char* text_start = gt + 1;
        auto close = strcasestr(text_start, "</a>");
        if (!close) break;
        std::string url(url_start, url_end);
        std::string label(text_start, close);
        //replace the anchor tag with a label
        size_t off = a - s.c_str();
        size_t len = (close + 4) - a;
        std::string repl = "[" + label + "]";
        s.replace(off, len, repl);
        //record where that bracketed label is now
        links.push_back({(int)off, (int)(off + (int)repl.size()), url});
    }
    //fuck all the other tags
    {
        std::string out; out.reserve(s.size());
        bool intag = false;
        for (char c : s) {
            if (c == '<') { intag = true; continue; }
            if (c == '>') { intag = false; continue; }
            if (!intag) out.push_back(c);
        }
        s.swap(out);
    }

    //try my best to get rid of all these line braks
    std::string out; out.reserve(s.size());
    int nl_run = 0;
    for (char c : s) {
        if (c == '\r') continue;
        if (c == '\n') { nl_run++; if (nl_run <= 2) out.push_back('\n'); }
        else { nl_run = 0; out.push_back(c); }
    }
    return out;
}

#ifdef _WIN32
#include <shellapi.h>
static void open_url(const std::string& u){ ShellExecuteA(nullptr,"open",u.c_str(),nullptr,nullptr,SW_SHOWNORMAL); }
#elif __APPLE__
static void open_url(const std::string& u){ std::string cmd="open \""+u+"\""; system(cmd.c_str()); }
#else
static void open_url(const std::string& u){ std::string cmd="xdg-open \""+u+"\""; system(cmd.c_str()); }
#endif

#include "imgui.h"

void render_wrapped_with_links(const std::string& text, const std::vector<LinkSpan>& links) {
    ImGui::PushTextWrapPos(0.0f);

    // simple line-wise scan so we can style headings
    int pos = 0;
    while (pos < (int)text.size()) {
        int line_end = (int)text.find('\n', pos);
        if (line_end < 0) line_end = (int)text.size();
        std::string line = text.substr(pos, line_end - pos);

        auto draw_segment = [&](int start, int end, bool link, const std::string& url){
            std::string chunk = line.substr(start, end - start);
            if (link) {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 0.6f, 1.0f, 1.0f));
                if (ImGui::Selectable(chunk.c_str(), false, ImGuiSelectableFlags_AllowDoubleClick)) {
                    // open link
#ifdef _WIN32
                    ShellExecuteA(nullptr,"open",url.c_str(),nullptr,nullptr,SW_SHOWNORMAL);
#elif __APPLE__
                    std::string cmd="open \""+url+"\""; system(cmd.c_str());
#else
                    std::string cmd="xdg-open \""+url+"\""; system(cmd.c_str());
#endif
                }
                ImGui::PopStyleColor();
            } else {
                ImGui::TextUnformatted(chunk.c_str());
            }
        };

        // heading styling: lines starting with 1–3 hashes
        float scale = 1.0f;
        if      (line.rfind("### ", 0) == 0) { scale = 1.05f; line.erase(0, 4); }
        else if (line.rfind("## ",  0) == 0) { scale = 1.15f; line.erase(0, 3); }
        else if (line.rfind("# ",   0) == 0) { scale = 1.30f; line.erase(0, 2); }

        if (scale > 1.0f) ImGui::SetWindowFontScale(scale);

        // render line with link awareness (span coordinates are in full text; map to line)
        int global_line_start = pos;
        int i = 0;
        while (i < (int)line.size()) {
            const LinkSpan* hit = nullptr;
            int hit_start = 0, hit_end = 0;

            for (auto& L : links) {
                int Ls = L.start - global_line_start;
                int Le = L.end   - global_line_start;
                if (Le <= 0 || Ls >= (int)line.size()) continue;
                int s = std::max(0, Ls);
                int e = std::min((int)line.size(), Le);
                if (i >= s && i < e) { hit = &L; hit_start = s; hit_end = e; break; }
            }

            if (hit) {
                if (i < hit_start) {
                    std::string chunk = line.substr(i, hit_start - i);
                    ImGui::TextUnformatted(chunk.c_str());
                    i = hit_start;
                }
                std::string lchunk = line.substr(hit_start, hit_end - hit_start);
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 0.6f, 1.0f, 1.0f));
                if (ImGui::Selectable(lchunk.c_str(), false, ImGuiSelectableFlags_AllowDoubleClick)) {
#ifndef _WIN32
#ifdef __APPLE__
                    std::string cmd="open \""+hit->url+"\""; system(cmd.c_str());
#else
                    std::string cmd="xdg-open \""+hit->url+"\""; system(cmd.c_str());
#endif
#else
                    ShellExecuteA(nullptr,"open",hit->url.c_str(),nullptr,nullptr,SW_SHOWNORMAL);
#endif
                }
                ImGui::PopStyleColor();
                i = hit_end;
            } else {
                std::string chunk = line.substr(i);
                ImGui::TextUnformatted(chunk.c_str());
                break;
            }
        }

        if (scale > 1.0f) ImGui::SetWindowFontScale(1.0f);
        pos = line_end + 1; // next line
    }

    ImGui::PopTextWrapPos();
}

// void render_wrapped_with_links(const std::string& text, const std::vector<LinkSpan>& links) {
//     ImGui::PushTextWrapPos(0.0f);
//
//     int i = 0;
//     for (int pos = 0; pos < (int)text.size();) {
//         // See if we are inside any link span
//         const LinkSpan* hit = nullptr;
//         for (auto& L : links) {
//             if (pos >= L.start && pos < L.end) { hit = &L; break; }
//         }
//         int nextBreak = (int)text.size();
//         if (hit) nextBreak = std::min(nextBreak, hit->end);
//
//         std::string chunk = text.substr(pos, nextBreak - pos);
//         if (hit) {
//             // draw link-styled text
//             ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 0.6f, 1.0f, 1.0f));
//             ImGui::PushStyleColor(ImGuiCol_TextDisabled, ImVec4(0.3f, 0.6f, 1.0f, 0.6f));
//             if (ImGui::Selectable(chunk.c_str(), false, ImGuiSelectableFlags_AllowDoubleClick)) {
//                 open_url(hit->url);
//             }
//             ImGui::PopStyleColor(2);
//         } else {
//             ImGui::TextUnformatted(chunk.c_str());
//         }
//         pos = nextBreak;
//     }
//
//     ImGui::PopTextWrapPos();
// }
//
