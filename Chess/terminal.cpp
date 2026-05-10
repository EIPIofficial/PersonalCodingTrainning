#include "terminal.h"

#include <iostream>
#include <optional>
#include <string>
#include <vector>
#include <termios.h>
#include <unistd.h>
#include <utility>

namespace {
    constexpr const char *ENABLE_MOUSE = "\x1b[?1000h\x1b[?1006h";
    constexpr const char *DISABLE_MOUSE = "\x1b[?1000l\x1b[?1006l";
    constexpr const char *HIDE_CURSOR = "\x1b[?25l";
    constexpr const char *SHOW_CURSOR = "\x1b[?25h";
    constexpr const char *ENTER_ALTERNATE_SCREEN = "\x1b[?1049h";
    constexpr const char *EXIT_ALTERNATE_SCREEN = "\x1b[?1049l";

    bool readByte(char &value) {
        return read(STDIN_FILENO, &value, 1) == 1;
    }

    std::optional<InputEvent> parseMouseEvent() {
        std::string payload = "[<";
        char c = '\0';

        while (readByte(c)) {
            payload.push_back(c);
            if (c == 'm' || c == 'M') {
                break;
            }
        }

        if (payload.empty() || payload.back() != 'M') {
            return std::nullopt;
        }

        if (payload.rfind("[<", 0) != 0) {
            return std::nullopt;
        }

        const std::size_t firstPos = payload.find(';');
        const std::size_t secondPos = payload.find(';', firstPos + 1);
        if (firstPos == std::string::npos || secondPos == std::string::npos) {
            return std::nullopt;
        }

        int botton = 0, col = 0, row = 0;

        try {
            botton = std::stoi(payload.substr(2, firstPos - 2));
            col = std::stoi(payload.substr(firstPos + 1, secondPos - firstPos - 1));
            row = std::stoi(payload.substr(secondPos + 1, payload.size() - secondPos - 2));
        }catch (...) {
            return std::nullopt;
        }

        if (botton != 0) {
            return std::nullopt;
        }

        return InputEvent{EventType::MouseClick, "", col, row};
    }
}

TerminalSession::TerminalSession() {
    tcgetattr(STDIN_FILENO, &originalState);
    termios raw = originalState;
    raw.c_lflag &= static_cast<unsigned long>(~(ICANON | ECHO));
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);

    std::cout << ENTER_ALTERNATE_SCREEN << ENABLE_MOUSE << HIDE_CURSOR << CLEAR_SCREEN << std::flush;
}

TerminalSession::~TerminalSession() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &originalState);
    std::cout << RESET_COLOR << DISABLE_MOUSE << SHOW_CURSOR << EXIT_ALTERNATE_SCREEN << std::flush;
}

std::size_t visibleLength(const std::string &text) {
    std::size_t length = 0;

    for (std::size_t i = 0; i < text.size(); ++i) {
        if (text[i] == '\x1b' && i+1 < text.size() && text[i+1] == '[') {
            i += 2;
            while (i < text.size() && (text[i] < '@' || text[i] > '~')) {
                i += 1;
            }
            continue;
        }

        ++length;
    }

    return length;
}

std::string visibleTruncate(const std::string &text, const std::size_t maxLength) {
    std::string truncated;
    std::size_t visible = 0;

    for (std::size_t i = 0; i < text.size() && visible < maxLength; ++i) {
        if (text[i] == '\x1b' && i + 1 < text.size() && text[i + 1] == '[') {
            const std::size_t sequenceStart = i;
            i += 2;
            while (i < text.size() && (text[i] < '@' || text[i] > '~')) {
                ++i;
            }
            if (i < text.size()) {
                truncated.append(text, sequenceStart, i - sequenceStart + 1);
            }
            continue;
        }

        truncated.push_back(text[i]);
        ++visible;
    }

    return truncated;
}

void drawLine(const int width) {
    std::cout << TITLE_COLOR << '+' << std::string(width-2, '-') << '+' << RESET_COLOR << '\n';
}


void drawBoxText(const std::string &text, const int width) {
    std::string line = text;
    const std::size_t maxWidth = static_cast<std::size_t>(width-4);

    if (visibleLength(line) > maxWidth) {
        line = visibleTruncate(line, maxWidth);
    }

    std::cout << TITLE_COLOR << "|" << RESET_COLOR << ' '
              << line
              << std::string(static_cast<std::size_t>(width-3) - visibleLength(line), ' ')
              << TITLE_COLOR << "|" << RESET_COLOR << '\n';
}

InputEvent readInputEvent() {
    char ch = '\0';
    if (!readByte(ch)) {
        return {};
    }

    switch (ch) {
        case '\r':
        case '\n':
            return InputEvent{EventType::Key, "enter"};
        case '\x1b': {
            char next = '\0';
            if (!readByte(next) || next != '[') {
                return {};
            }

            char code = '\0';
            if (!readByte(code)) {
                return {};
            }

            switch (code) {
                case '<':
                    return parseMouseEvent().value_or(InputEvent{});
                case 'A':
                    return InputEvent{EventType::Key, "up"};
                case 'B':
                    return InputEvent{EventType::Key, "down"};
                default:
                    return {};
            }
        }
        default:
            return InputEvent{EventType::Key, std::string(1, ch)};
    }
}

std::optional<std::string> actionAt(const std::vector<ClickRegion> &regions, const int col, const int row) {
    for (const auto &region: regions) {
        if (col >= region.col && col < region.col + region.width &&
            row >= region.row && row < region.row + region.height) {
            return region.action;
            }
    }
    return std::nullopt;
}
