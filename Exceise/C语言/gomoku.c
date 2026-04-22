/**
 * 五子棋 (Gomoku) - C语言实现
 * 支持双人对战和人机对战
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <windows.h>

// 棋盘常量
#define BOARD_SIZE 15
#define EMPTY 0
#define BLACK 1      // 黑棋
#define WHITE 2      // 白棋
#define WIN_COUNT 5  // 获胜所需连子数

// 方向向量：水平、垂直、两个对角线
const int dx[4] = {1, 0, 1, 1};
const int dy[4] = {0, 1, 1, -1};

// 游戏状态
typedef struct {
    int board[BOARD_SIZE][BOARD_SIZE];
    int currentPlayer;
    bool vsAI;
    int gameOver;
    int winner;
    int lastX, lastY;  // 最后落子位置
} GameState;

GameState game;

// 初始化游戏
void initGame() {
    memset(game.board, 0, sizeof(game.board));
    game.currentPlayer = BLACK;
    game.gameOver = 0;
    game.winner = EMPTY;
    game.lastX = -1;
    game.lastY = -1;
}

// 绘制棋盘
void drawBoard() {
    printf("\n");
    printf("  ");
    for (int i = 0; i < BOARD_SIZE; i++) {
        printf("%2d", i);
    }
    printf("\n");

    for (int i = 0; i < BOARD_SIZE; i++) {
        printf("%2d ", i);
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (game.lastX == j && game.lastY == i) {
                // 最后落子位置标记
                if (game.board[i][j] == BLACK) {
                    printf(" X");
                } else {
                    printf(" O");
                }
            } else {
                switch (game.board[i][j]) {
                    case EMPTY:
                        printf(" .");
                        break;
                    case BLACK:
                        printf(" X");
                        break;
                    case WHITE:
                        printf(" O");
                        break;
                }
            }
        }
        printf("\n");
    }
    printf("\n");
}

// 检查落子是否有效
bool isValidMove(int x, int y) {
    if (x < 0 || x >= BOARD_SIZE || y < 0 || y >= BOARD_SIZE) {
        return false;
    }
    return game.board[y][x] == EMPTY;
}

// 放置棋子
void placeStone(int x, int y, int player) {
    game.board[y][x] = player;
    game.lastX = x;
    game.lastY = y;
}

// 检查是否获胜
int checkWin(int x, int y) {
    int player = game.board[y][x];

    // 检查四个方向
    for (int dir = 0; dir < 4; dir++) {
        int count = 1;

        // 正向检查
        for (int i = 1; i < WIN_COUNT; i++) {
            int nx = x + dx[dir] * i;
            int ny = y + dy[dir] * i;
            if (nx >= 0 && nx < BOARD_SIZE && ny >= 0 && ny < BOARD_SIZE &&
                game.board[ny][nx] == player) {
                count++;
            } else {
                break;
            }
        }

        // 反向检查
        for (int i = 1; i < WIN_COUNT; i++) {
            int nx = x - dx[dir] * i;
            int ny = y - dy[dir] * i;
            if (nx >= 0 && nx < BOARD_SIZE && ny >= 0 && ny < BOARD_SIZE &&
                game.board[ny][nx] == player) {
                count++;
            } else {
                break;
            }
        }

        if (count >= WIN_COUNT) {
            return player;
        }
    }

    return EMPTY;
}

// 检查棋盘是否已满
bool isBoardFull() {
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (game.board[i][j] == EMPTY) {
                return false;
            }
        }
    }
    return true;
}

// AI评分系统 - 评估每个位置的得分
int evaluatePosition(int x, int y, int AI_player, int human_player) {
    int score = 0;
    int directions[4][2] = {{1, 0}, {0, 1}, {1, 1}, {1, -1}};

    for (int d = 0; d < 4; d++) {
        int dx_dir = directions[d][0];
        int dy_dir = directions[d][1];

        // 统计AI和人类在该方向的连子情况
        int AI_count = 0, AI_space = 0;
        int human_count = 0, human_space = 0;

        // 正向
        for (int i = 1; i <= 4; i++) {
            int nx = x + dx_dir * i;
            int ny = y + dy_dir * i;
            if (nx < 0 || nx >= BOARD_SIZE || ny < 0 || ny >= BOARD_SIZE) break;
            if (game.board[ny][nx] == AI_player) AI_count++;
            else if (game.board[ny][nx] == EMPTY) { AI_space++; break; }
            else { break; }
        }

        // 反向
        for (int i = 1; i <= 4; i++) {
            int nx = x - dx_dir * i;
            int ny = y - dy_dir * i;
            if (nx < 0 || nx >= BOARD_SIZE || ny < 0 || ny >= BOARD_SIZE) break;
            if (game.board[ny][nx] == AI_player) AI_count++;
            else if (game.board[ny][nx] == EMPTY) { AI_space++; break; }
            else { break; }
        }

        // 统计人类棋子
        for (int i = 1; i <= 4; i++) {
            int nx = x + dx_dir * i;
            int ny = y + dy_dir * i;
            if (nx < 0 || nx >= BOARD_SIZE || ny < 0 || ny >= BOARD_SIZE) break;
            if (game.board[ny][nx] == human_player) human_count++;
            else if (game.board[ny][nx] == EMPTY) { human_space++; break; }
            else { break; }
        }

        for (int i = 1; i <= 4; i++) {
            int nx = x - dx_dir * i;
            int ny = y - dy_dir * i;
            if (nx < 0 || nx >= BOARD_SIZE || ny < 0 || ny >= BOARD_SIZE) break;
            if (game.board[ny][nx] == human_player) human_count++;
            else if (game.board[ny][nx] == EMPTY) { human_space++; break; }
            else { break; }
        }

        // 评分
        if (AI_count >= 4) score += 100000;      // AI能赢
        else if (AI_count == 3 && AI_space >= 2) score += 10000;
        else if (AI_count == 3) score += 1000;
        else if (AI_count == 2 && AI_space >= 2) score += 500;
        else if (AI_count == 2) score += 100;

        if (human_count >= 4) score += 80000;     // 阻挡对手获胜
        else if (human_count == 3 && human_space >= 2) score += 8000;
        else if (human_count == 3) score += 800;
        else if (human_count == 2 && human_space >= 2) score += 400;
        else if (human_count == 2) score += 80;
    }

    // 位置权重 - 中心位置更有价值
    int centerDist = abs(x - 7) + abs(y - 7);
    score += (14 - centerDist) * 2;

    return score;
}

// AI选择最佳位置
void AI_move() {
    int bestScore = -1;
    int bestX = -1, bestY = -1;

    int AI_player = WHITE;
    int human_player = BLACK;

    // 搜索所有空位
    for (int y = 0; y < BOARD_SIZE; y++) {
        for (int x = 0; x < BOARD_SIZE; x++) {
            if (game.board[y][x] == EMPTY) {
                int score = evaluatePosition(x, y, AI_player, human_player);
                if (score > bestScore) {
                    bestScore = score;
                    bestX = x;
                    bestY = y;
                }
            }
        }
    }

    if (bestX != -1 && bestY != -1) {
        printf("AI 思考中...\n");
        Sleep(500);
        placeStone(bestX, bestY, AI_player);
        printf("AI 落子: %d %d\n", bestX, bestY);
    }
}

// 玩家输入处理
bool playerInput() {
    int x, y;

    printf("当前 %s (输入坐标如 '8 8'，或 'q'退出 'r'重新开始): ",
           game.currentPlayer == BLACK ? "黑棋(X)" : "白棋(O)");

    char input[20];
    if (fgets(input, sizeof(input), stdin) != NULL) {
        // 移除换行符
        input[strcspn(input, "\n")] = 0;

        if (strcmp(input, "q") == 0 || strcmp(input, "Q") == 0) {
            printf("\n感谢游玩！再见！\n");
            game.gameOver = -1;
            return false;
        }

        if (strcmp(input, "r") == 0 || strcmp(input, "R") == 0) {
            initGame();
            return true;
        }

        if (sscanf(input, "%d %d", &x, &y) == 2) {
            if (isValidMove(x, y)) {
                placeStone(x, y, game.currentPlayer);
                return true;
            } else {
                printf("无效位置，请重新输入！\n");
                return false;
            }
        } else {
            printf("输入格式错误，请输入 '列 行' (如 '8 8')！\n");
            return false;
        }
    }
    return false;
}

// 游戏回合处理
void gameLoop() {
    while (!game.gameOver) {
        drawBoard();

        char playerName[20];
        if (game.currentPlayer == BLACK) {
            strcpy(playerName, "黑棋(X)");
        } else {
            if (game.vsAI) {
                strcpy(playerName, "白棋(AI)");
            } else {
                strcpy(playerName, "白棋(O)");
            }
        }

        printf("当前: %s\n", playerName);

        // 人机模式且是AI回合
        if (game.vsAI && game.currentPlayer == WHITE) {
            AI_move();
        } else {
            if (!playerInput()) {
                continue;
            }
        }

        // 检查胜负
        int winner = checkWin(game.lastX, game.lastY);
        if (winner != EMPTY) {
            game.gameOver = 1;
            game.winner = winner;
            drawBoard();
            printf("\n==============================\n");
            printf("  恭喜！%s 获胜！\n",
                   winner == BLACK ? "黑棋" : "白棋");
            printf("==============================\n");
            return;
        }

        // 检查平局
        if (isBoardFull()) {
            game.gameOver = 1;
            drawBoard();
            printf("\n==============================\n");
            printf("  平局！棋盘已满！\n");
            printf("==============================\n");
            return;
        }

        // 切换玩家
        game.currentPlayer = (game.currentPlayer == BLACK) ? WHITE : BLACK;
    }
}

// 显示标题
void showTitle() {
    printf("\n");
    printf("  ╔═══════════════════════════════╗\n");
    printf("  ║                               ║\n");
    printf("  ║        五 子 棋               ║\n");
    printf("  ║        Gomoku                 ║\n");
    printf("  ║                               ║\n");
    printf("  ╚═══════════════════════════════╝\n");
    printf("\n");
}

// 选择游戏模式
void selectMode() {
    int choice;

    printf("请选择游戏模式:\n");
    printf("  1. 双人对战\n");
    printf("  2. 人机对战\n");
    printf("\n请选择 (1/2): ");

    if (scanf("%d", &choice) == 1) {
        while (getchar() != '\n');  // 清空输入缓冲区
        if (choice == 1) {
            game.vsAI = false;
            printf("\n已选择双人对战模式\n");
        } else if (choice == 2) {
            game.vsAI = true;
            printf("\n已选择人机对战模式\n");
            printf("你执黑棋(X)，AI执白棋(O)\n");
        } else {
            printf("无效选择，默认双人对战\n");
            game.vsAI = false;
        }
    } else {
        while (getchar() != '\n');
        game.vsAI = false;
    }
}

// 显示帮助
void showHelp() {
    printf("\n游戏说明:\n");
    printf("  - 15x15 棋盘\n");
    printf("  - 黑棋先手\n");
    printf("  - 连成5子获胜\n");
    printf("  - 坐标格式: 列 行 (0-14)\n");
    printf("\n命令:\n");
    printf("  q - 退出游戏\n");
    printf("  r - 重新开始\n");
}

// 主函数
int main() {
    // 设置控制台编码为UTF-8
    SetConsoleOutputCP(CP_UTF8);

    srand((unsigned int)time(NULL));

    while (1) {
        showTitle();
        showHelp();

        initGame();
        selectMode();

        gameLoop();

        // 询问是否继续
        printf("\n是否再来一局? (y/n): ");
        char again;
        scanf(" %c", &again);
        while (getchar() != '\n');  // 清空输入缓冲区

        if (again != 'y' && again != 'Y') {
            printf("\n感谢游玩！再见！\n");
            break;
        }
    }

    return 0;
}
