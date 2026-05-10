#include "terminal.h"
#include "gobang.h"

#include <iostream>
#include <optional>
#include <string>
#include <vector>

#include "gobang.h"

namespace {

    struct MenuOption {
        std::string key;
        std::string title;
        std::string description;
    };
    
    constexpr int MENU_WIDTH = 78;

    std::vector<ClickRegion> renderMenu(const std::vector<MenuOption> &options, const std::string status) {
        std::vector<ClickRegion> regions;

        std::cout << CLEAR_SCREEN;
        drawLine(MENU_WIDTH);
        drawBoxText(std::string(TITLE_COLOR) + " Codex Chess " + RESET_COLOR, MENU_WIDTH);
        drawBoxText(std::string(TEXT_COLOR) + "Click a mode or press its key to choose it." + RESET_COLOR, MENU_WIDTH);
        drawLine(MENU_WIDTH);
        drawBoxText("", MENU_WIDTH);

        int row = 6;
        for (const auto &option: options) {
            drawBoxText(
              std::string(HIGHLIGHT_COLOR) + "[" + option.key + "] " + RESET_COLOR + option.title,
              MENU_WIDTH
            );
            drawBoxText("    " + option.description, MENU_WIDTH);
            drawBoxText("", MENU_WIDTH);

            regions.push_back(ClickRegion{2, row, MENU_WIDTH - 4, 3, option.key});
            row += 3;
        }

        drawBoxText(std::string(HIGHLIGHT_COLOR) + "[q] " + RESET_COLOR + "Quit", MENU_WIDTH);
        regions.push_back(ClickRegion{2, row, MENU_WIDTH - 4, 3, "q"});
        ++row;

        drawLine(MENU_WIDTH);
        drawBoxText("", MENU_WIDTH);
        drawBoxText(std::string(TEXT_COLOR) + "Keyboard: 1-4, q. Mouse: left click on a menu item." + RESET_COLOR, MENU_WIDTH);
        drawBoxText(status.empty() ? "" : std::string(ERROR_COLOR) + status + RESET_COLOR, MENU_WIDTH);
        drawLine(MENU_WIDTH);
        std::cout << std::flush;

        return regions;
    }

    const MenuOption *findOptionByAction(const std::vector<MenuOption> &options, const std::string &action) {
        for (const auto &option: options) {
            if (option.key == action) {
                return &option;
            }
        }
        return nullptr;
    }
}

int main() {
    const std::vector<MenuOption> options = {
        {"1", "Classic Chess", "Standard 8x8 chess with the usual starting position."},
        {"2", "Go", "Adversarial game between two players with the objective of capturing territory."},
        {"3", "Gobang", "A simple two-player-chess, also named 'five-in-a-row'."},
    };

    TerminalSession terminal_session;
    std::string status;

    while (true) {
        const std::vector<ClickRegion> regions = renderMenu(options, status);
        status.clear();

        const InputEvent event = readInputEvent();
        std::string action;

        if (event.type == EventType::MouseClick) {
            const auto map = actionAt(regions, event.col, event.row);
            if (!map) {
                status = "That click is outside the active menu items.";
                continue;
            }
            action = *map;
        } else if (event.type == EventType::Key) {
            action = event.key;
        } else {
            status = "No input received";
            continue;
        }

        if (action == "q" || action == "Q") {
            break;
        }

        const MenuOption *selected = findOptionByAction(options, action);
        if (selected == nullptr) {
            status = "Use 1-3 on the keyboard or click a menu item.";
            continue;
        }

        if (selected->key == "3") {
            playGobang();
        }
    }

    return 0;
}
