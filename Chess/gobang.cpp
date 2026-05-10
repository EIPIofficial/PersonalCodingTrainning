#include "gobang.h"
#include "terminal.h"

#include <iostream>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace {
    const int BOARD_SIZE = 15;
    const int MENU_WIDTH = 78;

    std::string playerText(const int crtPlayer) {
        return crtPlayer == 1 ? "Black (X)" : "White (O)";
    }

    std::string stoneText(const char stone) {
        if (stone == 'X') {
            return std::string(HIGHLIGHT_COLOR) + "X" + RESET_COLOR;
        }

        if (stone == 'O') {
            return std::string(TITLE_COLOR) + "O" + RESET_COLOR;
        }

        return ".";
    }

    bool isFull(const std::vector<std::vector<char>> &board) {
        for (const auto &row : board) {
            for (const char stone : row) {
                if (stone == '.') {
                    return false;
                }
            }
        }

        return true;
    }

    int countDirection(
        const std::vector<std::vector<char>> &board,
        const int row,
        const int col,
        const int deltaRow,
        const int deltaCol,
        const char stone
    ) {
        int count = 0;
        int nextRow = row + deltaRow;
        int nextCol = col + deltaCol;

        while (nextRow >= 0 && nextRow < BOARD_SIZE &&
               nextCol >= 0 && nextCol < BOARD_SIZE &&
               board[static_cast<std::size_t>(nextRow)][static_cast<std::size_t>(nextCol)] == stone) {
            ++count;
            nextRow += deltaRow;
            nextCol += deltaCol;
               }

        return count;
    }

    bool isWin(
        const std::vector<std::vector<char>> &board,
        const int row,
        const int col,
        const int player
    ) {
        constexpr int directions[4][2] = {
            {0, 1},
            {1, 0},
            {1, 1},
            {1, -1},
        };

        const char stone = (player == 1) ? 'X' : 'O';

        for (const auto &direction: directions) {
            const int connected = 1
                + countDirection(board, row, col, direction[0], direction[1], stone)
                + countDirection(board, row, col, -direction[0], -direction[1], stone);

            if (connected >= 5) {
                return true;
            }
        }

        return false;
    }

    std::optional<std::pair<int, int>> parseMouseEvent(const std::string &action) {
        constexpr const char *prefix = "cell:";

        if (action.rfind(prefix, 0) != 0) {
            return std::nullopt;
        }

        const std::size_t separator = action.find(':', 5);
        if (separator == std::string::npos) {
            return std::nullopt;
        }

        try {
            const int row = std::stoi(action.substr(5, separator - 5));
            const int col = std::stoi(action.substr(separator + 1));
            return std::pair<int, int>{row, col};
        } catch (...) {
            return std::nullopt;
        }
    }

    std::vector<ClickRegion> renderGobang(
        const std::vector<std::vector<char>> &board,
        const int crtPlayer,
        const std::string status,
        const bool gameOver
    ) {
        int terminalRow = 1;
        std::vector<ClickRegion> regions;

        std::cout <<CLEAR_SCREEN;
        drawLine(MENU_WIDTH);
        ++terminalRow;
        drawBoxText(std::string(TITLE_COLOR) + "Gobang" + RESET_COLOR, MENU_WIDTH);
        ++terminalRow;
        drawBoxText(std::string(TEXT_COLOR) + "Click an empty point to place a stone. Five in a row wins." + RESET_COLOR, MENU_WIDTH);
        ++terminalRow;
        drawBoxText(gameOver
            ? std::string(TEXT_COLOR) + "Press r to restart, or q / Enter to return to the menu." + RESET_COLOR
            : std::string(TEXT_COLOR) + "Turn: " + HIGHLIGHT_COLOR + playerText(crtPlayer) + RESET_COLOR + ". Press q to return.",
            MENU_WIDTH
        );
        ++terminalRow;
        drawBoxText(status.empty() ? "" : std::string(ERROR_COLOR) + status + RESET_COLOR, MENU_WIDTH);
        ++terminalRow;
        drawLine(MENU_WIDTH);
        ++terminalRow;


        std::string header = "   ";
        for (int col = 1; col <= BOARD_SIZE; ++col) {
            if (col < 10) {
                header += " ";
            }
            header += std::to_string(col);
            header += " ";
        }
        drawBoxText(header, MENU_WIDTH);
        ++terminalRow;

        for (int row = 0; row < BOARD_SIZE; ++row) {
            std::string line;
            if (row + 1 < 10) {
                line += " ";
            }
            line += std::to_string(row + 1);
            line += " ";

            for (int col = 0; col < BOARD_SIZE; ++col) {
                line += " ";
                line += stoneText(board[static_cast<std::size_t>(row)][static_cast<std::size_t>(col)]);
                line += " ";

                regions.push_back(ClickRegion{
                    6 + col * 3,
                    terminalRow,
                    3,
                    1,
                    "cell:" + std::to_string(row) + ":" + std::to_string(col)
                });
            }

            drawBoxText(line, MENU_WIDTH);
            ++terminalRow;
        }
        drawLine(MENU_WIDTH);

        return regions;
    }




}

void playGobang() {
    std::vector<std::vector<char>> board(
        BOARD_SIZE, std::vector<char>(BOARD_SIZE, '.'));

    int crtPlayer = 1;
    bool gameOver = false;
    std::string status;

    while (true) {
        const std::vector<ClickRegion> regions = renderGobang(board, crtPlayer, status, gameOver);
        status.clear();

        const InputEvent event = readInputEvent();
        if (event.type == EventType::Key) {
            if (event.key == "q" || event.key == "Q" || event.key == "enter") {
                return;
            }

            if (event.key == "r" || event.key == "R") {
                board.assign(BOARD_SIZE, std::vector<char>(BOARD_SIZE, '.'));
                crtPlayer = 1;
                gameOver = false;
                status = "New Gobang game started.";
                continue;
            }

            status = "Use the mouse to place a stone, r to restart, or q to return.";
            continue;
        }

        if (event.type != EventType::MouseClick) {
            continue;
        }

        const auto action = actionAt(regions, event.col, event.row);
        if (!action) {
            status = "Click inside the chess board";
            continue;
        }

        const auto cell = parseMouseEvent(*action);
        if (!cell) {
            status = "That click is not a board point.";
            continue;
        }

        if (gameOver) {
            status = "Game over. Press r to restart, or q / Enter to return.";
            continue;
        }

        const int row = cell->first;
        const int col = cell->second;

        char &target = board[static_cast<std::size_t>(row)][static_cast<std::size_t>(col)];
        if (target != '.') {
            status = "That point is already occupied.";
            continue;
        }

        target = (crtPlayer == 1) ? 'X' : 'O';
        status.clear();

        if (isWin(board, row, col, crtPlayer)) {
            status = playerText(crtPlayer) + " wins.";
            gameOver = true;
            continue;
        }

        if (isFull(board)) {
            status = "The board is full. Draw game.";
            gameOver = true;
            continue;
        }

        crtPlayer = (crtPlayer == 1) ? 0 : 1;
    }
}

