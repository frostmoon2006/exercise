/*
 * ============================================================================
 *  Match-3 Game (Xiao Xiao Le) - C Language Console Version
 * ============================================================================
 *
 * Game Rules:
 * - 8x8 grid with 6 different colors
 * - Use arrow keys to move cursor, spacebar to select/swap
 * - Match 3 or more same-color blocks in a row to eliminate
 * - After elimination, blocks above fall down to fill gaps
 * - Support chain elimination (combo) for bonus points
 *
 * ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#ifdef _WIN32
    #include <windows.h>
    #include <conio.h>
    #define DELAY_MS(ms) Sleep(ms)
#else
    #include <unistd.h>
    #include <sys/time.h>
    #define DELAY_MS(ms) usleep((ms) * 1000)
    #define Sleep(ms) usleep((ms) * 1000)

    int _kbhit(void) {
        struct timeval tv;
        fd_set rdfs;
        tv.tv_sec = 0;
        tv.tv_usec = 0;
        FD_ZERO(&rdfs);
        FD_SET(STDIN_FILENO, &rdfs);
        select(STDIN_FILENO + 1, &rdfs, NULL, NULL, &tv);
        return FD_ISSET(STDIN_FILENO, &rdfs);
    }

    int _getch(void) {
        struct termios oldt, newt;
        int ch;
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
        ch = getchar();
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        return ch;
    }
#endif

// ============================================================================
// Constants
// ============================================================================

#define BOARD_SIZE   8
#define NUM_COLORS   6
#define MIN_MATCH    3

char COLOR_CHARS[] = {'A', 'B', 'C', 'D', 'E', 'F'};
int COLOR_VALUES[] = {1, 2, 3, 4, 5, 6};

// Console colors (Windows)
#ifdef _WIN32
    #define CLR_BLACK   0
    #define CLR_WHITE   7
    #define CLR_RED     12
    #define CLR_GREEN   10
    #define CLR_BLUE    11
    #define CLR_YELLOW  14
    #define CLR_PINK    13
    #define CLR_CYAN    3
#else
    #define CLR_BLACK   30
    #define CLR_WHITE   37
    #define CLR_RED     31
    #define CLR_GREEN   32
    #define CLR_BLUE    34
    #define CLR_YELLOW  33
    #define CLR_PINK    35
    #define CLR_CYAN    36
#endif

int COLOR_CODES[] = {
    CLR_RED,
    CLR_GREEN,
    CLR_BLUE,
    CLR_YELLOW,
    CLR_PINK,
    CLR_CYAN
};

// ============================================================================
// Global Variables
// ============================================================================

int board[BOARD_SIZE][BOARD_SIZE];
int cursorX = 0, cursorY = 0;
int selectedX = -1, selectedY = -1;
bool isSelected = false;
int score = 0;
int combo = 0;
int moves = 0;
bool gameOver = false;

// ============================================================================
// Console Functions (Double buffer for no flicker)
// ============================================================================

#ifdef _WIN32
// 双缓冲：维护一个离屏缓冲区来避免闪烁
static HANDLE screenBuffer[2] = {NULL, NULL};
static int currentBuffer = 0;
static bool bufferInitialized = false;

void initDoubleBuffer() {
    if (bufferInitialized) return;
    
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    GetConsoleScreenBufferInfo(hConsole, &csbi);
    
    // 创建两个离屏缓冲区
    for (int i = 0; i < 2; i++) {
        screenBuffer[i] = CreateConsoleScreenBuffer(
            GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL,
            CONSOLE_TEXTMODE_BUFFER,
            NULL
        );
        if (screenBuffer[i] != INVALID_HANDLE_VALUE) {
            SetConsoleScreenBufferSize(screenBuffer[i], csbi.dwSize);
        }
    }
    bufferInitialized = true;
}

void clearScreen() {
    COORD topLeft = {0, 0};
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO screen;
    DWORD written;
    GetConsoleScreenBufferInfo(console, &screen);
    FillConsoleOutputCharacterA(console, ' ', screen.dwSize.X * screen.dwSize.Y, topLeft, &written);
    FillConsoleOutputAttribute(console, screen.wAttributes, screen.dwSize.X * screen.dwSize.Y, topLeft, &written);
    SetConsoleCursorPosition(console, topLeft);
}

void swapBuffer() {
    HANDLE back = screenBuffer[1 - currentBuffer];
    HANDLE front = screenBuffer[currentBuffer];
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    
    // 获取后缓冲区内容并写入控制台
    COORD bufferSize = {50, 40};
    CHAR_INFO chiBuffer[50 * 40];
    SMALL_RECT readRect = {0, 0, 49, 39};
    
    ReadConsoleOutput(back, chiBuffer, bufferSize, (COORD){0,0}, &readRect);
    WriteConsoleOutput(console, chiBuffer, bufferSize, (COORD){0,0}, &readRect);
    
    currentBuffer = 1 - currentBuffer;
}

void gotoxy(int x, int y) {
    COORD coord;
    coord.X = (SHORT)x;
    coord.Y = (SHORT)y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

void setColor(int color) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
}

void resetColor() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, 7);
}

void hideCursor() {
    CONSOLE_CURSOR_INFO info;
    info.bVisible = FALSE;
    info.dwSize = 100;
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);
}

void showCursor() {
    CONSOLE_CURSOR_INFO info;
    info.bVisible = TRUE;
    info.dwSize = 100;
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);
}
#else
// Linux版本：使用printf直接输出
static bool bufferInitialized = false;
static char screenBuffer[50][100];
static int bufWidth = 100;
static int bufHeight = 50;

void initDoubleBuffer() {
    if (bufferInitialized) return;
    memset(screenBuffer, ' ', sizeof(screenBuffer));
    for (int y = 0; y < bufHeight; y++) {
        screenBuffer[y][bufWidth-1] = '\0';
    }
    bufferInitialized = true;
}

void clearScreen() {
    printf("\033[2J\033[H");
    fflush(stdout);
}

void swapBuffer() {
    printf("\033[H");
    for (int y = 0; y < bufHeight; y++) {
        printf("%s\n", screenBuffer[y]);
    }
    fflush(stdout);
    memset(screenBuffer, ' ', sizeof(screenBuffer));
}

void gotoxy(int x, int y) {
    printf("\033[%d;%dH", y + 1, x + 1);
    fflush(stdout);
}

void setColor(int color) {
    printf("\033[%dm", color);
}

void resetColor() {
    printf("\033[0m");
}

void hideCursor() { }
void showCursor() { }
#endif

// ============================================================================
// Game Functions
// ============================================================================

bool wouldCreateMatch(int x, int y, int color);

void initGame() {
    memset(board, 0, sizeof(board));
    cursorX = BOARD_SIZE / 2;
    cursorY = BOARD_SIZE / 2;
    selectedX = -1;
    selectedY = -1;
    isSelected = false;
    score = 0;
    combo = 0;
    moves = 30;
    gameOver = false;

    for (int y = 0; y < BOARD_SIZE; y++) {
        for (int x = 0; x < BOARD_SIZE; x++) {
            int color;
            do {
                color = rand() % NUM_COLORS;
                board[y][x] = color + 1;
            } while (wouldCreateMatch(x, y, color + 1));
        }
    }
}

bool wouldCreateMatch(int x, int y, int color) {
    if (x >= 2) {
        if (board[y][x-1] == color && board[y][x-2] == color) {
            return true;
        }
    }
    if (y >= 2) {
        if (board[y-1][x] == color && board[y-2][x] == color) {
            return true;
        }
    }
    return false;
}

void swapCells(int x1, int y1, int x2, int y2) {
    int temp = board[y1][x1];
    board[y1][x1] = board[y2][x2];
    board[y2][x2] = temp;
}

bool isAdjacent(int x1, int y1, int x2, int y2) {
    int dx = abs(x1 - x2);
    int dy = abs(y1 - y2);
    return (dx == 1 && dy == 0) || (dx == 0 && dy == 1);
}

int findMatches() {
    int matches = 0;
    bool matched[BOARD_SIZE][BOARD_SIZE] = {false};

    for (int y = 0; y < BOARD_SIZE; y++) {
        for (int x = 0; x < BOARD_SIZE - 2; x++) {
            int color = board[y][x];
            if (color != 0 &&
                board[y][x + 1] == color &&
                board[y][x + 2] == color) {
                matched[y][x] = matched[y][x + 1] = matched[y][x + 2] = true;
                for (int x2 = x + 3; x2 < BOARD_SIZE && board[y][x2] == color; x2++) {
                    matched[y][x2] = true;
                }
            }
        }
    }

    for (int x = 0; x < BOARD_SIZE; x++) {
        for (int y = 0; y < BOARD_SIZE - 2; y++) {
            int color = board[y][x];
            if (color != 0 &&
                board[y + 1][x] == color &&
                board[y + 2][x] == color) {
                matched[y][x] = matched[y + 1][x] = matched[y + 2][x] = true;
                for (int y2 = y + 3; y2 < BOARD_SIZE && board[y2][x] == color; y2++) {
                    matched[y2][x] = true;
                }
            }
        }
    }

    for (int y = 0; y < BOARD_SIZE; y++) {
        for (int x = 0; x < BOARD_SIZE; x++) {
            if (matched[y][x]) {
                board[y][x] = 0;
                matches++;
            }
        }
    }

    return matches;
}

void applyGravity() {
    for (int x = 0; x < BOARD_SIZE; x++) {
        int writeY = BOARD_SIZE - 1;
        for (int y = BOARD_SIZE - 1; y >= 0; y--) {
            if (board[y][x] != 0) {
                if (y != writeY) {
                    board[writeY][x] = board[y][x];
                    board[y][x] = 0;
                }
                writeY--;
            }
        }
        for (int y = writeY; y >= 0; y--) {
            board[y][x] = (rand() % NUM_COLORS) + 1;
        }
    }
}

int processMatches() {
    int totalMatches = 0;
    combo = 0;

    while (true) {
        int matches = findMatches();
        if (matches == 0) break;

        combo++;
        totalMatches += matches;
        score += matches * 10 * combo;
        DELAY_MS(300);
        applyGravity();
    }

    return totalMatches;
}

bool hasValidMoves() {
    for (int y = 0; y < BOARD_SIZE; y++) {
        for (int x = 0; x < BOARD_SIZE; x++) {
            if (x < BOARD_SIZE - 1) {
                swapCells(x, y, x + 1, y);
                int matches = findMatches();
                swapCells(x, y, x + 1, y);
                if (matches > 0) return true;
            }
            if (y < BOARD_SIZE - 1) {
                swapCells(x, y, x, y + 1);
                int matches = findMatches();
                swapCells(x, y, x, y + 1);
                if (matches > 0) return true;
            }
        }
    }
    return false;
}

// ============================================================================
// Display (Double buffer - no flicker)
// ============================================================================

void drawBoard() {
    #ifdef _WIN32
    initDoubleBuffer();
    
    // 清空后台缓冲区
    COORD bufferSize = {50, 40};
    COORD topLeft = {0, 0};
    HANDLE back = screenBuffer[1 - currentBuffer];
    FillConsoleOutputCharacter(back, ' ', bufferSize.X * bufferSize.Y, topLeft, &(DWORD){0});
    FillConsoleOutputAttribute(back, 7, bufferSize.X * bufferSize.Y, topLeft, &(DWORD){0});
    
    // 构建显示字符串
    char line[100];
    DWORD written;
    int lineY = 0;
    
    sprintf(line, "\n");
    WriteConsoleOutputCharacter(back, line, (DWORD)strlen(line), (COORD){0, (SHORT)lineY++}, &written);
    
    sprintf(line, "  +======================================+");
    WriteConsoleOutputCharacter(back, line, (DWORD)strlen(line), (COORD){0, (SHORT)lineY++}, &written);
    
    sprintf(line, "  |           消 消 乐 游 戏              |");
    WriteConsoleOutputCharacter(back, line, (DWORD)strlen(line), (COORD){0, (SHORT)lineY++}, &written);
    
    sprintf(line, "  +======================================+");
    WriteConsoleOutputCharacter(back, line, (DWORD)strlen(line), (COORD){0, (SHORT)lineY++}, &written);
    
    sprintf(line, "\n");
    WriteConsoleOutputCharacter(back, line, (DWORD)strlen(line), (COORD){0, (SHORT)lineY++}, &written);
    
    sprintf(line, "    ");
    for (int j = 0; j < BOARD_SIZE; j++) {
        char num[5];
        sprintf(num, " %d ", j + 1);
        WriteConsoleOutputCharacter(back, num, (DWORD)strlen(num), (COORD){4 + j*4, (SHORT)(lineY-1)}, &written);
    }
    
    sprintf(line, "   +");
    for (int j = 0; j < BOARD_SIZE; j++) {
        WriteConsoleOutputCharacter(back, "---+", 4, (COORD){4 + j*4, (SHORT)lineY}, &written);
    }
    lineY++;
    
    for (int y = 0; y < BOARD_SIZE; y++) {
        sprintf(line, " %d |", y + 1);
        WriteConsoleOutputCharacter(back, line, (DWORD)strlen(line), (COORD){0, (SHORT)lineY}, &written);
        
        for (int x = 0; x < BOARD_SIZE; x++) {
            int color = board[y][x];
            int isCursor = (x == cursorX && y == cursorY);
            int isSelectedCell = (x == selectedX && y == selectedY);
            
            char cell[5];
            if (color == 0) {
                strcpy(cell, "   ");
            } else if (isSelectedCell) {
                sprintf(cell, "[%c]", COLOR_CHARS[color - 1]);
            } else if (isCursor) {
                sprintf(cell, "<%c>", COLOR_CHARS[color - 1]);
            } else {
                sprintf(cell, " %c ", COLOR_CHARS[color - 1]);
            }
            WriteConsoleOutputCharacter(back, cell, (DWORD)strlen(cell), (COORD){4 + x*4, (SHORT)lineY}, &written);
        }
        lineY++;
        
        sprintf(line, "   +");
        for (int j = 0; j < BOARD_SIZE; j++) {
            WriteConsoleOutputCharacter(back, "---+", 4, (COORD){4 + j*4, (SHORT)lineY}, &written);
        }
        lineY++;
    }
    
    sprintf(line, "\n");
    WriteConsoleOutputCharacter(back, line, (DWORD)strlen(line), (COORD){0, (SHORT)lineY++}, &written);
    
    sprintf(line, "  +======================================+");
    WriteConsoleOutputCharacter(back, line, (DWORD)strlen(line), (COORD){0, (SHORT)lineY++}, &written);
    
    sprintf(line, "  |  得分: %-6d  步数: %-3d  连击: %-3d  |", score, moves, combo);
    WriteConsoleOutputCharacter(back, line, (DWORD)strlen(line), (COORD){0, (SHORT)lineY++}, &written);
    
    sprintf(line, "  +======================================+");
    WriteConsoleOutputCharacter(back, line, (DWORD)strlen(line), (COORD){0, (SHORT)lineY++}, &written);
    
    sprintf(line, "\n");
    WriteConsoleOutputCharacter(back, line, (DWORD)strlen(line), (COORD){0, (SHORT)lineY++}, &written);
    
    sprintf(line, "  操作: 方向键移动, 空格选择/交换, R重新开始");
    WriteConsoleOutputCharacter(back, line, (DWORD)strlen(line), (COORD){0, (SHORT)lineY++}, &written);
    
    if (gameOver) {
        sprintf(line, "\n");
        WriteConsoleOutputCharacter(back, line, (DWORD)strlen(line), (COORD){0, (SHORT)lineY++}, &written);
        
        sprintf(line, "  *** 游戏结束！最终得分: %d ***", score);
        WriteConsoleOutputCharacter(back, line, (DWORD)strlen(line), (COORD){0, (SHORT)lineY++}, &written);
    }
    
    if (combo > 1) {
        sprintf(line, "\n");
        WriteConsoleOutputCharacter(back, line, (DWORD)strlen(line), (COORD){0, (SHORT)lineY++}, &written);
        
        sprintf(line, "  ** 连击 x%d! **", combo);
        WriteConsoleOutputCharacter(back, line, (DWORD)strlen(line), (COORD){0, (SHORT)lineY++}, &written);
    }
    
    // 交换缓冲区：一次性显示整个画面
    swapBuffer();
    
    #else
    // Linux版本：使用字符串缓冲区
    initDoubleBuffer();
    
    int lineY = 0;
    sprintf(screenBuffer[lineY++], "\n");
    sprintf(screenBuffer[lineY++], "  +======================================+");
    sprintf(screenBuffer[lineY++], "  |           消 消 乐 游 戏              |");
    sprintf(screenBuffer[lineY++], "  +======================================+");
    sprintf(screenBuffer[lineY++], "\n");
    
    char header[50] = "    ";
    for (int j = 0; j < BOARD_SIZE; j++) {
        char num[5];
        sprintf(num, " %d ", j + 1);
        strcat(header, num);
    }
    strcpy(screenBuffer[lineY++], header);
    
    char sep[50] = "   +";
    for (int j = 0; j < BOARD_SIZE; j++) {
        strcat(sep, "---+");
    }
    strcpy(screenBuffer[lineY++], sep);
    
    for (int y = 0; y < BOARD_SIZE; y++) {
        char row[100];
        sprintf(row, " %d |", y + 1);
        for (int x = 0; x < BOARD_SIZE; x++) {
            int color = board[y][x];
            int isCursor = (x == cursorX && y == cursorY);
            int isSelectedCell = (x == selectedX && y == selectedY);
            
            if (color == 0) {
                strcat(row, "   ");
            } else if (isSelectedCell) {
                char cell[10];
                sprintf(cell, "[%c]", COLOR_CHARS[color - 1]);
                strcat(row, cell);
            } else if (isCursor) {
                char cell[10];
                sprintf(cell, "<%c>", COLOR_CHARS[color - 1]);
                strcat(row, cell);
            } else {
                char cell[10];
                sprintf(cell, " %c ", COLOR_CHARS[color - 1]);
                strcat(row, cell);
            }
            strcat(row, "|");
        }
        strcpy(screenBuffer[lineY++], row);
        strcpy(screenBuffer[lineY++], sep);
    }
    
    sprintf(screenBuffer[lineY++], "\n");
    sprintf(screenBuffer[lineY++], "  +======================================+");
    sprintf(screenBuffer[lineY++], "  |  得分: %-6d  步数: %-3d  连击: %-3d  |", score, moves, combo);
    sprintf(screenBuffer[lineY++], "  +======================================+");
    sprintf(screenBuffer[lineY++], "\n");
    sprintf(screenBuffer[lineY++], "  操作: 方向键移动, 空格选择/交换, R重新开始");
    
    if (gameOver) {
        sprintf(screenBuffer[lineY++], "\n");
        sprintf(screenBuffer[lineY++], "  *** 游戏结束！最终得分: %d ***", score);
    }
    
    if (combo > 1) {
        sprintf(screenBuffer[lineY++], "\n");
        sprintf(screenBuffer[lineY++], "  ** 连击 x%d! **", combo);
    }
    
    // 一次性输出所有内容
    printf("\033[H");
    for (int y = 0; y < lineY; y++) {
        printf("%s\n", screenBuffer[y]);
    }
    fflush(stdout);
    #endif
}

void handleInput() {
    if (_kbhit()) {
        int ch = _getch();
        int extended = 0;

        if (ch == 224 || ch == 0) {
            extended = _getch();
        }

        switch (extended) {
            case 72:
                if (cursorY > 0) cursorY--;
                break;
            case 80:
                if (cursorY < BOARD_SIZE - 1) cursorY++;
                break;
            case 75:
                if (cursorX > 0) cursorX--;
                break;
            case 77:
                if (cursorX < BOARD_SIZE - 1) cursorX++;
                break;
        }

        if (ch == ' ' || ch == '\r') {
            if (!isSelected) {
                selectedX = cursorX;
                selectedY = cursorY;
                isSelected = true;
            } else {
                if (cursorX == selectedX && cursorY == selectedY) {
                    isSelected = false;
                    selectedX = -1;
                    selectedY = -1;
                } else if (isAdjacent(cursorX, cursorY, selectedX, selectedY)) {
                    swapCells(selectedX, selectedY, cursorX, cursorY);
                    moves--;
                    isSelected = false;
                    selectedX = -1;
                    selectedY = -1;

                    processMatches();

                    if (moves <= 0 || !hasValidMoves()) {
                        gameOver = true;
                    }
                } else {
                    selectedX = cursorX;
                    selectedY = cursorY;
                }
            }
        }

        if (ch == 'r' || ch == 'R') {
            initGame();
        }

        if (ch == 'q' || ch == 'Q') {
            gameOver = true;
        }
    }
}

// ============================================================================
// Main
// ============================================================================

int main() {
    #ifdef _WIN32
        SetConsoleOutputCP(65001);
        SetConsoleCP(65001);
        SetConsoleTitle("Match-3 Game");
        system("mode con cols=45 lines=35");
    #endif

    srand((unsigned)time(NULL));
    hideCursor();

    printf("\n");
    printf("  +======================================+\n");
    printf("  |                                      |\n");
    printf("  |        欢迎来到消消乐游戏              |\n");
    printf("  |                                      |\n");
    printf("  |          C语言控制台版本               |\n");
    printf("  |                                      |\n");
    printf("  +======================================+\n");
    printf("\n");
    printf("  游戏规则:\n");
    printf("  * 方向键移动光标\n");
    printf("  * 空格键选择/交换相邻方块\n");
    printf("  * 三个或以上相同颜色连线消除\n");
    printf("  * 连续消除可获得额外加分\n");
    printf("  * 共有30步机会获得最高分!\n");
    printf("\n");
    printf("  按任意键开始游戏...\n");
    _getch();

    initGame();

    while (!gameOver) {
        drawBoard();
        handleInput();
        DELAY_MS(50);
    }

    drawBoard();
    printf("\n");
    printf("  最终得分: %d\n", score);
    printf("  最大连击: x%d\n", combo);
    printf("\n");
    printf("  感谢游玩！按任意键退出...\n");
    showCursor();
    _getch();

    return 0;
}
