#define NOMINMAX
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <utility>
#include <filesystem>

#ifdef _WIN32
    #include <conio.h>
    #include <windows.h>
    #pragma comment(lib, "user32.lib")
#else
    #include <termios.h>
    #include <unistd.h>
    #include <sys/ioctl.h>
#endif

namespace fs = std::filesystem;

// ANSI Escape Codes for terminal formatting
#define RESET     "\x1b[0m"
#define GREEN     "\x1b[32m"
#define RED       "\x1b[31m"
#define DIM       "\x1b[90m"
#define HIGHLIGHT "\x1b[44m"
#define CLR_SCR   "\x1b[2J\x1b[H"

enum KeyCode {
    KEY_ENTER     = 13,
    KEY_BACKSPACE = 8,
    KEY_CTRL_A    = 1,
    KEY_CTRL_C    = 3,
    KEY_CTRL_D    = 4,
    KEY_CTRL_Q    = 17,
    KEY_ESC       = 27,
    KEY_UP        = 1000,
    KEY_DOWN      = 1001,
    KEY_LEFT      = 1002,
    KEY_RIGHT     = 1003,
    KEY_PGUP      = 1004,
    KEY_PGDN      = 1005,
    KEY_HOME      = 1006,
    KEY_END       = 1007,
};

// ---------------------------------------------------------------------------
// Terminal / input helpers
// ---------------------------------------------------------------------------

void enableRawMode(bool legacy) {
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#else
    struct termios raw;
    tcgetattr(STDIN_FILENO, &raw);
    raw.c_lflag &= ~(ICANON | ECHO | ISIG);
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);
#endif
    if (!legacy) {
        std::cout << "\x1b[?1049h" << std::flush; // enter alternate screen buffer
    }
}

void disableRawMode(bool legacy) {
    if (!legacy) {
        std::cout << "\x1b[?1049l" << std::flush; // leave alternate screen buffer
    }
    std::cout << RESET;
#ifndef _WIN32
    struct termios cooked;
    tcgetattr(STDIN_FILENO, &cooked);
    cooked.c_lflag |= (ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &cooked);
#endif
}

int getTerminalRows() {
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
        return csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    }
    return 24;
#else
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0 && w.ws_row > 0) {
        return w.ws_row;
    }
    return 24;
#endif
}

int getTerminalColumns() {
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
        return csbi.srWindow.Right - csbi.srWindow.Left + 1;
    }
    return 80;
#else
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0 && w.ws_col > 0) {
        return w.ws_col;
    }
    return 80;
#endif
}

// Raw single byte read, used by the typing engine (mirrors ctyper's _getch/getchar).
int readByte() {
#ifdef _WIN32
    return _getch();
#else
    return getchar();
#endif
}

// Decoded key read (arrows / page keys), used by the picker.
int readKey() {
#ifdef _WIN32
    int ch = _getch();
    if (ch == 0 || ch == 224) {
        int c2 = _getch();
        switch (c2) {
            case 72: return KEY_UP;
            case 80: return KEY_DOWN;
            case 75: return KEY_LEFT;
            case 77: return KEY_RIGHT;
            case 73: return KEY_PGUP;
            case 81: return KEY_PGDN;
            case 71: return KEY_HOME;
            case 79: return KEY_END;
            default: return c2;
        }
    }
    return ch;
#else
    int ch = getchar();
    if (ch == KEY_ESC) {
        int c2 = getchar();
        if (c2 == '[') {
            int c3 = getchar();
            switch (c3) {
                case 'A': return KEY_UP;
                case 'B': return KEY_DOWN;
                case 'C': return KEY_RIGHT;
                case 'D': return KEY_LEFT;
                case 'H': return KEY_HOME;
                case 'F': return KEY_END;
                case '5': getchar(); return KEY_PGUP;
                case '6': getchar(); return KEY_PGDN;
                default: return KEY_ESC;
            }
        } else if (c2 == 'O') {
            int c3 = getchar();
            if (c3 == 'H') return KEY_HOME;
            if (c3 == 'F') return KEY_END;
            return KEY_ESC;
        }
        return KEY_ESC;
    }
    return ch;
#endif
}

// ---------------------------------------------------------------------------
// Lesson scanning
// ---------------------------------------------------------------------------

struct LessonEntry {
    std::string rel_path;   // display path, e.g. "official/webdev/nextjs/api.md"
    std::string full_path;  // absolute-ish path for opening
};

std::string lowerAscii(const std::string& s) {
    std::string out = s;
    for (char& c : out) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return out;
}

bool matchesQuery(const std::string& rel, const std::string& query) {
    if (query.empty()) return true;
    return lowerAscii(rel).find(lowerAscii(query)) != std::string::npos;
}

std::vector<LessonEntry> scanLessons(const std::string& root,
                                     const std::string& lesson_ext) {
    std::vector<LessonEntry> out;
    std::error_code ec;
    fs::path rootp(root);
    if (!fs::exists(rootp, ec) || !fs::is_directory(rootp, ec)) return out;

    fs::recursive_directory_iterator it(
        rootp, fs::directory_options::skip_permission_denied, ec), end;
    for (; it != end; it.increment(ec)) {
        if (ec) break;
        if (!it->is_regular_file(ec)) continue;
        if (lesson_ext != "*") {
            std::string ext = lowerAscii(it->path().extension().string());
            if (ext != lesson_ext) continue;
        }
        std::string rel = fs::relative(it->path(), rootp, ec).generic_string();
        if (ec) continue;
        out.push_back({ rel, it->path().string() });
    }

    std::sort(out.begin(), out.end(), [](const LessonEntry& a, const LessonEntry& b) {
        return a.rel_path < b.rel_path;
    });
    return out;
}

// One item shown in a single folder of the picker.
struct DirEntry {
    std::string name;       // display name (folders end with '/', files omit their extension)
    std::string full_path;  // path for opening a file
    std::string rel_path;   // path relative to lessons root
    bool is_dir;
    bool pinned = false;    // '!' prefix: always sorts first, prefix hidden
};

// Lists the contents of one directory: folders first, then lessons, each sorted.
std::vector<DirEntry> listDirectory(const std::string& root,
                                    const std::string& rel,
                                    const std::string& lesson_ext) {
    std::vector<DirEntry> out;
    std::error_code ec;
    fs::path dir = rel.empty() ? fs::path(root) : fs::path(root) / fs::path(rel);
    if (!fs::exists(dir, ec) || !fs::is_directory(dir, ec)) return out;

    fs::directory_iterator it(dir, ec), end;
    for (; it != end; it.increment(ec)) {
        if (ec) break;
        std::string name = it->path().filename().string();
        if (it->is_directory(ec)) {
            bool pinned = false;
            if (!name.empty() && name[0] == '!') {
                pinned = true;
                name = name.substr(1);
            }
            std::string child_rel = rel.empty()
                                        ? it->path().filename().string()
                                        : rel + "/" + it->path().filename().string();
            out.push_back({ name + "/", it->path().string(), child_rel, true, pinned });
        } else if (it->is_regular_file(ec)) {
            if (lesson_ext != "*" &&
                lowerAscii(it->path().extension().string()) != lesson_ext) continue;
            std::string rel_path = fs::relative(it->path(), root, ec).generic_string();
            if (ec) continue;
            out.push_back({ it->path().stem().string(), it->path().string(), rel_path, false });
        }
    }

    std::sort(out.begin(), out.end(), [](const DirEntry& a, const DirEntry& b) {
        if (a.is_dir != b.is_dir) return a.is_dir;
        if (a.pinned != b.pinned) return a.pinned;
        return a.name < b.name;
    });
    return out;
}

// ---------------------------------------------------------------------------
// Lesson text loading / parsing helpers
// ---------------------------------------------------------------------------

struct LineInfo {
    size_t start_pos;
    size_t end_pos; // inclusive of newline if present
};

std::string loadNormalizedFile(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::binary);
    if (!file) return std::string();
    std::string raw((std::istreambuf_iterator<char>(file)),
                    std::istreambuf_iterator<char>());
    file.close();

    std::string target;
    target.reserve(raw.size());
    for (size_t i = 0; i < raw.size(); ++i) {
        if (raw[i] == '\r') {
            if (i + 1 < raw.size() && raw[i + 1] == '\n') continue;
            target += '\n';
        } else {
            target += raw[i];
        }
    }
    return target;
}

std::vector<LineInfo> splitLines(const std::string& target) {
    std::vector<LineInfo> lines;
    size_t l_start = 0;
    for (size_t i = 0; i < target.size(); ++i) {
        if (target[i] == '\n') {
            lines.push_back({ l_start, i });
            l_start = i + 1;
        }
    }
    if (l_start < target.size()) {
        lines.push_back({ l_start, target.size() - 1 });
    }
    return lines;
}

// Lines whose (leading-whitespace-trimmed) content starts with '#' = section headers.
std::vector<size_t> findHeadingLines(const std::string& target,
                                     const std::vector<LineInfo>& lines) {
    std::vector<size_t> heading_lines;
    for (size_t l = 0; l < lines.size(); ++l) {
        size_t i = lines[l].start_pos;
        while (i <= lines[l].end_pos && (target[i] == ' ' || target[i] == '\t')) ++i;
        if (i <= lines[l].end_pos && target[i] == '#') heading_lines.push_back(l);
    }
    return heading_lines;
}

std::string sectionNameFor(const std::string& target,
                           const std::vector<LineInfo>& lines,
                           const std::vector<size_t>& heading_lines,
                           size_t pos) {
    for (size_t k = heading_lines.size(); k-- > 0; ) {
        size_t hl = heading_lines[k];
        if (lines[hl].start_pos <= pos) {
            std::string text = target.substr(lines[hl].start_pos,
                                             lines[hl].end_pos - lines[hl].start_pos + 1);
            size_t b = text.find_first_not_of("# \t");
            if (b == std::string::npos) return std::string();
            size_t e = text.find_last_not_of(" \t\r\n");
            if (e != std::string::npos && e >= b) return text.substr(b, e - b + 1);
            return std::string();
        }
    }
    return std::string();
}

std::string truncateTo(const std::string& s, size_t maxlen) {
    if (s.size() <= maxlen) return s;
    std::string out = s.substr(0, maxlen);
    while (!out.empty() && (static_cast<unsigned char>(out.back()) & 0xC0) == 0x80)
        out.pop_back();
    if (!out.empty() && (static_cast<unsigned char>(out.back()) & 0xC0) == 0xC0)
        out.pop_back();
    const std::string ell = "...";
    if (out.size() < ell.size()) return out;
    return out.substr(0, out.size() - ell.size()) + ell;
}

// ---------------------------------------------------------------------------
// Typing session (adapted from ctyper)
// ---------------------------------------------------------------------------

int runTypingSession(const std::string& filepath,
                     const std::string& rel_path,
                     int max_lines_setting,
                     bool audio_enabled,
                     bool legacy_render) {
    std::string target = loadNormalizedFile(filepath);
    if (target.empty()) return 0;

    std::vector<LineInfo> lines = splitLines(target);
    std::vector<size_t> heading_lines = findHeadingLines(target, lines);

    int term_rows = getTerminalRows() - 3; // reserve footer rows
    if (term_rows < 1) term_rows = 1;
    int max_lines_per_page = std::min(max_lines_setting, term_rows);

    std::vector<char> typed_status(target.size(), 0); // 0 = unread, 1 = correct, 2 = wrong

    size_t current_line_idx = 0;

    auto renderScreen = [&](size_t cursor_pos, size_t window_start_line) {
        size_t window_end_line = std::min(lines.size(),
                                          window_start_line + static_cast<size_t>(max_lines_per_page));

        // Legacy mode clears the whole screen each frame; default mode redraws
        // in place (no clear) so the previous frame is overwritten without flicker.
        if (legacy_render) std::cout << CLR_SCR;
        else std::cout << "\x1b[H";

        for (size_t l = window_start_line; l < window_end_line; ++l) {
            for (size_t i = lines[l].start_pos; i <= lines[l].end_pos; ++i) {
                if (target[i] == '\n') continue;
                if (i < cursor_pos) {
                    if (typed_status[i] == 1) std::cout << GREEN << target[i];
                    else if (typed_status[i] == 2) std::cout << RED << target[i];
                } else {
                    std::cout << RESET << target[i];
                }
            }
            if (!legacy_render) std::cout << "\x1b[K";
            std::cout << "\n";
        }

        // Footer: file, current section, progress, and key hints
        int cols = getTerminalColumns();
        if (cols < 1) cols = 80;
        std::string section = sectionNameFor(target, lines, heading_lines, cursor_pos);
        std::string pct = std::to_string(static_cast<int>(100.0 * cursor_pos / target.size()));
        std::string left = rel_path;
        if (!section.empty()) left += " \xc2\xb7 section: " + section;
        std::cout << DIM << "  " << truncateTo(left, static_cast<size_t>(cols - 4))
                  << "  " << pct << "%";
        if (!legacy_render) std::cout << "\x1b[K";
        std::cout << "\n"
                  << "  enter=skip line \xc2\xb7 shift+enter=reset line \xc2\xb7 ctrl+d=next section"
                  << " \xc2\xb7 ctrl+a=prev section \xc2\xb7 ctrl+c=back to menu";
        if (!legacy_render) std::cout << "\x1b[K";
        std::cout << "\n" << RESET;
        if (!legacy_render) std::cout << "\x1b[J";

        // Reposition terminal cursor back to cursor_pos relative to window_start_line
        std::cout << "\x1b[H";
        for (size_t l = window_start_line; l < window_end_line; ++l) {
            bool is_last_line = (l + 1 == lines.size());
            bool on_this_line = false;
            if (is_last_line) {
                on_this_line = (cursor_pos >= lines[l].start_pos && cursor_pos <= lines[l].end_pos + 1);
            } else {
                on_this_line = (cursor_pos >= lines[l].start_pos && cursor_pos <= lines[l].end_pos);
            }

            if (on_this_line) {
                for (size_t i = lines[l].start_pos; i < cursor_pos && i <= lines[l].end_pos; ) {
                    if (target[i] == '\n') break;
                    if (target[i] == '\t') {
                        std::cout << "\t";
                        i++;
                    } else {
                        std::cout << "\x1b[1C";
                        i++;
                        while (i < cursor_pos && i <= lines[l].end_pos &&
                               (static_cast<unsigned char>(target[i]) & 0xC0) == 0x80) {
                            i++;
                        }
                    }
                }
                break;
            } else {
                std::cout << "\x1b[1B";
            }
        }
        std::cout << std::flush;
    };

    size_t pos = 0;
    size_t window_start_line = 0;

    // End of the leading whitespace run on a line, plus whether it has a tab.
    auto indentRunEnd = [&](size_t line_idx) -> std::pair<size_t, bool> {
        size_t e = lines[line_idx].start_pos;
        bool has_tab = false;
        while (e <= lines[line_idx].end_pos &&
               (target[e] == ' ' || target[e] == '\t')) {
            if (target[e] == '\t') has_tab = true;
            e++;
        }
        return { e, has_tab };
    };

    auto autoSkipIndent = [&]() -> bool {
        if (pos >= target.size()) return false;
        if (pos != lines[current_line_idx].start_pos) return false;
        auto [run_end, has_tab] = indentRunEnd(current_line_idx);
        size_t run_len = run_end - pos;
        if (run_len < 2 && !has_tab) return false;
        for (size_t i = pos; i < run_end; ++i) typed_status[i] = 1;
        pos = run_end;
        if (current_line_idx != window_start_line) window_start_line = current_line_idx;
        return true;
    };

    auto locateLine = [&]() {
        while (current_line_idx < lines.size() && pos > lines[current_line_idx].end_pos) {
            current_line_idx++;
        }
        if (current_line_idx >= lines.size() && !lines.empty()) {
            current_line_idx = lines.size() - 1;
        }
        while (current_line_idx > 0 && pos < lines[current_line_idx].start_pos) {
            current_line_idx--;
        }
        if (current_line_idx != window_start_line) window_start_line = current_line_idx;
    };

    autoSkipIndent();
    renderScreen(pos, window_start_line);

    while (pos < target.size()) {
        locateLine();

        int ch = readByte();

        // Ctrl+C / Ctrl+Q: leave the typing session and return to the picker
        if (ch == KEY_CTRL_C || ch == KEY_CTRL_Q) {
            std::cout << CLR_SCR << RESET;
            return 1;
        }

        // Ctrl+A: jump back to the previous heading line
        if (ch == KEY_CTRL_A) {
            size_t prev_heading = lines.size();
            for (size_t k = heading_lines.size(); k-- > 0; ) {
                if (heading_lines[k] < current_line_idx) { prev_heading = heading_lines[k]; break; }
            }
            if (prev_heading != lines.size()) {
                size_t new_pos = lines[prev_heading].start_pos;
                if (new_pos < pos) {
                    pos = new_pos;
                    locateLine();
                    autoSkipIndent();
                    renderScreen(pos, window_start_line);
                }
            }
            continue;
        }

        // Ctrl+D: skip the current section, jump to the next heading line
        if (ch == KEY_CTRL_D) {
            size_t next_heading = lines.size();
            for (size_t h : heading_lines) {
                if (h > current_line_idx) { next_heading = h; break; }
            }
            size_t skip_to = (next_heading < lines.size())
                                 ? lines[next_heading].start_pos
                                 : target.size();
            if (skip_to <= pos) continue;
            for (size_t i = pos; i < skip_to; ++i) typed_status[i] = 2;
            pos = skip_to;
            locateLine();
            autoSkipIndent();
            renderScreen(pos, window_start_line);
            continue;
        }

        // Backspace handling
        if (ch == KEY_BACKSPACE || ch == 127) {
            if (pos > 0) {
                size_t run_start = lines[current_line_idx].start_pos;
                auto [run_end, has_tab] = indentRunEnd(current_line_idx);
                size_t run_len = run_end - run_start;

                // One backspace at the edge of (or inside) an auto-skipped
                // indent clears the whole run back to the line start.
                if ((run_len >= 2 || has_tab) && pos > run_start && pos <= run_end) {
                    for (size_t i = run_start; i < pos; ++i) typed_status[i] = 0;
                    pos = run_start;
                    if (current_line_idx != window_start_line) window_start_line = current_line_idx;
                    renderScreen(pos, window_start_line);
                } else {
                    do {
                        pos--;
                        typed_status[pos] = 0;
                    } while (pos > 0 && (static_cast<unsigned char>(target[pos]) & 0xC0) == 0x80);

                    if (current_line_idx > 0 && pos < lines[current_line_idx].start_pos) {
                        current_line_idx--;
                    }
                    if (current_line_idx != window_start_line) window_start_line = current_line_idx;
                    renderScreen(pos, window_start_line);
                }
            }
            continue;
        }

        // Shift+Enter: reset the current line — clear green/red markings and return to its first character
        if (ch == '\r' || ch == '\n') {
            bool shift_held = false;
#ifdef _WIN32
            shift_held = (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0;
#endif
            if (shift_held) {
                size_t ls = lines[current_line_idx].start_pos;
                size_t le = lines[current_line_idx].end_pos;
                for (size_t i = ls; i <= le && i < target.size(); ++i) {
                    typed_status[i] = 0;
                }
                pos = ls;
                locateLine();
                autoSkipIndent();
                renderScreen(pos, window_start_line);
                continue;
            }
            if (target[pos] != '\n') {
                for (size_t i = pos; i <= lines[current_line_idx].end_pos; ++i) {
                    typed_status[i] = 2;
                }
                pos = lines[current_line_idx].end_pos;
                if (pos < target.size() && target[pos] == '\n') {
                    typed_status[pos] = 1;
                    pos += 1;
                } else {
                    pos = target.size();
                }
            } else {
                typed_status[pos] = 1;
                pos += 1;
            }
            locateLine();
            autoSkipIndent();
            renderScreen(pos, window_start_line);
            continue;
        }

        // Determine expected UTF-8 char length
        unsigned char head = static_cast<unsigned char>(target[pos]);
        size_t char_len = 1;
        if ((head & 0xE0) == 0xC0) char_len = 2;
        else if ((head & 0xF0) == 0xE0) char_len = 3;
        else if ((head & 0xF8) == 0xF0) char_len = 4;

        if (pos + char_len > target.size()) {
            char_len = target.size() - pos;
        }

        // Read multi-byte character if needed
        std::string inputSeq;
        char firstChar = (ch == '\r') ? '\n' : static_cast<char>(ch);
        inputSeq += firstChar;

        for (size_t b = 1; b < char_len; ++b) {
            inputSeq += static_cast<char>(readByte());
        }

        std::string expectedSeq = target.substr(pos, char_len);

        bool match = (inputSeq == expectedSeq);
        for (size_t b = 0; b < char_len; ++b) {
            typed_status[pos + b] = match ? 1 : 2;
        }
        if (!match && audio_enabled) {
            std::cout << "\a"; // Audible alert on error
        }

        pos += char_len;
        locateLine();
        autoSkipIndent();
        renderScreen(pos, window_start_line);
    }

    std::cout << CLR_SCR << RESET;
    return 0;
}

// ---------------------------------------------------------------------------
// File picker TUI
// ---------------------------------------------------------------------------

enum PickResult { PICK_CONTINUE, PICK_QUIT };

PickResult runFilePicker(const std::string& root,
                         int max_lines_setting,
                         bool audio_enabled,
                         bool legacy_render,
                         const std::string& lesson_ext) {
    std::string query;
    std::string current_rel; // "" = lessons root
    size_t selected = 0;
    std::vector<DirEntry> entries;
    std::vector<size_t> filtered;

    auto refresh = [&]() {
        entries = listDirectory(root, current_rel, lesson_ext);
        if (!current_rel.empty()) {
            DirEntry up;
            up.name = "..";
            up.is_dir = true;
            entries.insert(entries.begin(), up);
        }
        filtered.clear();
        for (size_t i = 0; i < entries.size(); ++i) {
            if (matchesQuery(entries[i].name, query)) filtered.push_back(i);
        }
        if (selected >= filtered.size()) selected = filtered.empty() ? 0 : filtered.size() - 1;
    };

    auto ascend = [&]() {
        if (current_rel.empty()) return;
        size_t slash = current_rel.find_last_of('/');
        current_rel = (slash == std::string::npos) ? std::string()
                                                   : current_rel.substr(0, slash);
        query.clear();
        selected = 0;
        refresh();
    };

    auto breadcrumb = [&]() {
        std::string crumbs = "lessons";
        std::string rest = current_rel;
        size_t start = 0;
        while (true) {
            size_t slash = rest.find('/', start);
            std::string part = rest.substr(start, slash == std::string::npos
                                                    ? std::string::npos : slash - start);
            if (!part.empty() && part[0] == '!') part = part.substr(1);
            crumbs += " / " + part;
            if (slash == std::string::npos) break;
            start = slash + 1;
        }
        return crumbs;
    };

    refresh();

    while (true) {
        int rows = getTerminalRows();
        int cols = getTerminalColumns();
        if (rows < 1) rows = 24;
        if (cols < 1) cols = 80;
        int list_rows = rows - 7;
        if (list_rows < 1) list_rows = 1;

        std::cout << CLR_SCR;
        std::cout << HIGHLIGHT << "  typelab \xc2\xb7 lesson picker  " << RESET
                  << "  " << truncateTo(breadcrumb(), static_cast<size_t>(cols - 24)) << "\n\n";
        std::cout << "  \x1b[1m?\x1b[0m " << query << HIGHLIGHT << " \x1b[0m\n\n";

        if (filtered.empty()) {
            std::cout << DIM << "  (nothing here)" << RESET << "\n";
        } else {
            size_t scroll = 0;
            if (selected >= static_cast<size_t>(list_rows)) {
                scroll = selected - static_cast<size_t>(list_rows) + 1;
            }
            for (size_t i = scroll;
                 i < filtered.size() && i < scroll + static_cast<size_t>(list_rows);
                 ++i) {
                const DirEntry& e = entries[filtered[i]];
                std::string line = truncateTo(e.name, static_cast<size_t>(cols - 4));
                if (i == selected) std::cout << HIGHLIGHT << " > " << line << RESET << "\n";
                else if (e.is_dir) std::cout << DIM << "   " << line << RESET << "\n";
                else std::cout << "   " << line << "\n";
            }
        }
        std::cout << "\n" << DIM
                  << "  type to search \xc2\xb7 \x1b[1m\xe2\x86\x91\xe2\x86\x93\x1b[0m"
                  << DIM << " move \xc2\xb7 enter open \xc2\xb7 esc clear \xc2\xb7 ctrl+c quit\n"
                  << RESET;
        std::cout << std::flush;

        int key = readKey();

        if (key == KEY_CTRL_C || key == KEY_CTRL_Q) return PICK_QUIT;

        if (key == KEY_ENTER) {
            if (filtered.empty()) continue;
            const DirEntry& e = entries[filtered[selected]];
            if (e.name == "..") {
                ascend();
            } else if (e.is_dir) {
                current_rel = e.rel_path;
                query.clear();
                selected = 0;
                refresh();
            } else {
                runTypingSession(e.full_path, e.rel_path,
                                 max_lines_setting, audio_enabled, legacy_render);
                refresh();
            }
            continue;
        }

        if (key == KEY_BACKSPACE || key == 127 || key == KEY_LEFT) {
            if (!query.empty()) { query.pop_back(); refresh(); }
            else ascend();
            continue;
        }

        if (key == KEY_ESC) {
            if (!query.empty()) { query.clear(); refresh(); }
            continue;
        }

        if (key == KEY_UP) {
            if (!filtered.empty()) selected = (selected == 0) ? filtered.size() - 1 : selected - 1;
            continue;
        }
        if (key == KEY_DOWN) {
            if (!filtered.empty()) selected = (selected + 1) % filtered.size();
            continue;
        }
        if (key == KEY_PGUP) {
            if (!filtered.empty()) selected = selected > static_cast<size_t>(list_rows)
                                                  ? selected - static_cast<size_t>(list_rows) : 0;
            continue;
        }
        if (key == KEY_PGDN) {
            if (!filtered.empty()) {
                selected = std::min(filtered.size() - 1,
                                    selected + static_cast<size_t>(list_rows));
            }
            continue;
        }
        if (key == KEY_HOME) { selected = 0; continue; }
        if (key == KEY_END) { if (!filtered.empty()) selected = filtered.size() - 1; continue; }

        if (key >= 32 && key <= 126) {
            query += static_cast<char>(key);
            refresh();
            continue;
        }
    }
    return PICK_CONTINUE;
}

// ---------------------------------------------------------------------------
// Entry point
// ---------------------------------------------------------------------------

int main(int argc, char* argv[]) {
    int max_lines_setting = 50;
    bool audio_enabled = false;
    bool legacy_render = false;
    std::string lessons_root = "lessons";
    std::string lesson_ext = ".md";

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-n" || arg == "--lines") {
            if (i + 1 < argc) {
                max_lines_setting = std::atoi(argv[++i]);
                if (max_lines_setting <= 0) max_lines_setting = 50;
            } else {
                std::cerr << "Error: " << arg << " requires a numeric argument\n";
                return 1;
            }
        } else if (arg == "-a" || arg == "--audio") {
            audio_enabled = true;
        } else if (arg == "--legacy") {
            legacy_render = true;
        } else if (arg == "-l" || arg == "--lessons") {
            if (i + 1 < argc) {
                lessons_root = argv[++i];
            } else {
                std::cerr << "Error: " << arg << " requires a directory argument\n";
                return 1;
            }
        } else if (arg == "-e" || arg == "--ext") {
            if (i + 1 < argc) {
                std::string ext = argv[++i];
                if (ext != "*" && !ext.empty() && ext[0] != '.') ext = "." + ext;
                lesson_ext = lowerAscii(ext);
            } else {
                std::cerr << "Error: " << arg << " requires an extension argument\n";
                return 1;
            }
        } else if (arg == "-h" || arg == "--help") {
            std::cerr << "Usage: typelab [-n/--lines <num>] [-a/--audio] [--legacy] [-l/--lessons <dir>] [-e/--ext <ext|*>]\n";
            return 0;
        } else if (arg[0] == '-') {
            std::cerr << "Unknown option: " << arg << "\n";
            std::cerr << "Usage: typelab [-n/--lines <num>] [-a/--audio] [--legacy] [-l/--lessons <dir>] [-e/--ext <ext|*>]\n";
            return 1;
        }
    }

    enableRawMode(legacy_render);

    PickResult result = PICK_CONTINUE;
    while (result == PICK_CONTINUE) {
        std::vector<LessonEntry> lessons = scanLessons(lessons_root, lesson_ext);
        if (lessons.empty()) {
            std::string ext_label = (lesson_ext == "*") ? "" : (lesson_ext + " ");
            std::cout << CLR_SCR
                      << DIM << "  no " << ext_label << "lessons found under '" << lessons_root << "'\n"
                      << "  drop " << ext_label << "files into the lessons/ tree (official/ or ext/) and press enter\n"
                      << RESET << std::flush;
            int k = readKey();
            if (k == KEY_CTRL_C || k == KEY_CTRL_Q) break;
            continue;
        }
        result = runFilePicker(lessons_root, max_lines_setting, audio_enabled, legacy_render, lesson_ext);
    }

    // In default mode the app runs in the alternate screen buffer: clear it
    // first, then leave the buffer so the pre-app screen is restored intact.
    std::cout << CLR_SCR << RESET;
    disableRawMode(legacy_render);
    return 0;
}
