#pragma once

#include <iostream>
#include <optional>
#include <string>
#include <vector>
#include <termios.h>

struct ClickRegion {
    int col = 1;
    int row = 1;
    int width = 1;
    int height = 1;
    std::string action;
};

enum class EventType {
    None,
    Key,
    MouseClick,
};

struct InputEvent {
    EventType type = EventType::None;
    std::string key;
    int col = 0;
    int row = 0;
};

constexpr const char *CLEAR_SCREEN = "\x1b[2J\x1b[H";
constexpr const char *TITLE_COLOR = "\x1b[38;5;81m";
constexpr const char *TEXT_COLOR = "\x1b[38;5;252m";
constexpr const char *HIGHLIGHT_COLOR = "\x1b[38;5;114m";
constexpr const char *ERROR_COLOR = "\x1b[38;5;203m";
constexpr const char *RESET_COLOR = "\x1b[0m";

class TerminalSession {
public:
    TerminalSession();
    ~TerminalSession();

    TerminalSession(const TerminalSession &) = delete;
    TerminalSession &operator=(const TerminalSession &) = delete;

private:
    termios originalState{};
};

std::size_t visibleLength(const std::string &text);
std::string visibleTruncate(const std::string &text, std::size_t maxLength);
void drawLine(int width);
void drawBoxText(const std::string &text, int width);

InputEvent readInputEvent();
std::optional<std::string> actionAt(const std::vector<ClickRegion> &regions, int col, int row);