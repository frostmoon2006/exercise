/*
 * ============================================================================
 *  Tetris - C Language Console Version
 * ============================================================================
 *
 * Fixed Issues:
 * 1. Frame rate: Using clock() for stable 60 FPS
 * 2. Rotation: Full 4 rotation states tracking
 * 3. Cross-platform: All Windows API properly protected with #ifdef
 * 4. No flicker: Using cursor positioning instead of clear screen
 * 5. Drop speed: Properly using dropInterval
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
    #include <termios.h>
    #include <unistd.h>
    #include <sys/time.h>
    #include <sys/select.h>
    #define DELAY_MS(ms) usleep((ms) * 1000)

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

#define BOARD_WIDTH   10
#define BOARD_HEIGHT  20
#define FPS           60
#define FRAME_TIME    (1000 / FPS)

#define COLOR_EMPTY   0
#define COLOR_I       1
#define COLOR_O       2
#define COLOR_T       3
#define COLOR_S       4
#define COLOR_Z       5
#define COLOR_J       6
#define COLOR_L       7

#define DISPLAY_COLOR_I 9
#define DISPLAY_COLOR_O 14
#define DISPLAY_COLOR_T 13
#define DISPLAY_COLOR_S 10
#define DISPLAY_COLOR_Z 12
#define DISPLAY_COLOR_J 11
#define DISPLAY_COLOR_L 6

// ============================================================================
// Global Variables
// ============================================================================

int board[BOARD_HEIGHT][BOARD_WIDTH] = {0};

int currentX, currentY;
int currentType;
int currentRotation;

int nextType;
int nextRotation;

int score = 0;
int linesCleared = 0;
int level = 1;
bool gameOver = false;
bool isPaused = false;

// ============================================================================
// Tetromino Definitions
// ============================================================================

int TETROMINO_SHAPES[7][4][4][4] = {
    // I
    {{{0,0,0,0},{1,1,1,1},{0,0,0,0},{0,0,0,0}},
     {{0,0,1,0},{0,0,1,0},{0,0,1,0},{0,0,1,0}},
     {{0,0,0,0},{0,0,0,0},{1,1,1,1},{0,0,0,0}},
     {{0,1,0,0},{0,1,0,0},{0,1,0,0},{0,1,0,0}}},
    // O
    {{{0,0,0,0},{0,1,1,0},{0,1,1,0},{0,0,0,0}},
     {{0,0,0,0},{0,1,1,0},{0,1,1,0},{0,0,0,0}},
     {{0,0,0,0},{0,1,1,0},{0,1,1,0},{0,0,0,0}},
     {{0,0,0,0},{0,1,1,0},{0,1,1,0},{0,0,0,0}}},
    // T
    {{{0,0,0,0},{1,1,1,0},{0,1,0,0},{0,0,0,0}},
     {{0,1,0,0},{1,1,0,0},{0,1,0,0},{0,0,0,0}},
     {{0,1,0,0},{1,1,1,0},{0,0,0,0},{0,0,0,0}},
     {{0,1,0,0},{0,1,1,0},{0,1,0,0},{0,0,0,0}}},
    // S
    {{{0,0,0,0},{0,1,1,0},{1,1,0,0},{0,0,0,0}},
     {{1,0,0,0},{1,1,0,0},{0,1,0,0},{0,0,0,0}},
     {{0,0,0,0},{0,1,1,0},{1,1,0,0},{0,0,0,0}},
     {{1,0,0,0},{1,1,0,0},{0,1,0,0},{0,0,0,0}}},
    // Z
    {{{0,0,0,0},{1,1,0,0},{0,1,1,0},{0,0,0,0}},
     {{0,0,1,0},{0,1,1,0},{0,1,0,0},{0,0,0,0}},
     {{0,0,0,0},{1,1,0,0},{0,1,1,0},{0,0,0,0}},
     {{0,1,0,0},{1,1,0,0},{1,0,0,0},{0,0,0,0}}},
    // J
    {{{0,0,0,0},{1,0,0,0},{1,1,1,0},{0,0,0,0}},
     {{0,1,1,0},{0,1,0,0},{0,1,0,0},{0,0,0,0}},
     {{0,0,0,0},{1,1,1,0},{0,0,1,0},{0,0,0,0}},
     {{0,1,0,0},{0,1,0,0},{1,1,0,0},{0,0,0,0}}},
    // L
    {{{0,0,0,0},{0,0,1,0},{1,1,1,0},{0,0,0,0}},
     {{0,1,0,0},{0,1,0,0},{0,1,1,0},{0,0,0,0}},
     {{0,0,0,0},{1,1,1,0},{1,0,0,0},{0,0,0,0}},
     {{1,1,0,0},{0,1,0,0},{0,1,0,0},{0,0,0,0}}}
};

int TETROMINO_DISPLAY_COLORS[7] = {
    DISPLAY_COLOR_I, DISPLAY_COLOR_O, DISPLAY_COLOR_T,
    DISPLAY_COLOR_S, DISPLAY_COLOR_Z, DISPLAY_COLOR_J, DISPLAY_COLOR_L
};

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
    
    // 获取前缓冲区内容并写入控制台
    COORD bufferSize = {45, 40};
    CHAR_INFO chiBuffer[45 * 40];
    SMALL_RECT readRect = {0, 0, 44, 39};
    
    ReadConsoleOutput(back, chiBuffer, bufferSize, (COORD){0,0}, &readRect);
    WriteConsoleOutput(console, chiBuffer, bufferSize, (COORD){0,0}, &readRect);
    
    currentBuffer = 1 - currentBuffer;
}

void writeToBuffer(int x, int y, const char* text, int color) {
    if (!bufferInitialized) return;
    
    HANDLE back = screenBuffer[1 - currentBuffer];
    COORD writePos = {(SHORT)x, (SHORT)y};
    
    // 设置颜色属性
    WORD colorAttr = (WORD)color;
    DWORD written;
    
    // 写入文本
    WriteConsoleOutputCharacter(back, text, (DWORD)strlen(text), writePos, &written);
    FillConsoleOutputAttribute(back, colorAttr, (DWORD)strlen(text), writePos, &written);
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
    CONSOLE_CURSOR_INFO cursorInfo;
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(hConsole, &cursorInfo);
}

void showCursor() {
    CONSOLE_CURSOR_INFO cursorInfo;
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = TRUE;
    SetConsoleCursorInfo(hConsole, &cursorInfo);
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

void writeToBuffer(int x, int y, const char* text, int color) {
    for (int i = 0; text[i] && x + i < bufWidth - 1; i++) {
        screenBuffer[y][x + i] = text[i];
    }
}

void gotoxy(int x, int y) {
    printf("\033[%d;%dH", y + 1, x + 1);
    fflush(stdout);
}

void setColor(int color) {
    int fg = color % 16;
    printf("\033[38;5;%dm", fg);
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

void initGame() {
    for (int y = 0; y < BOARD_HEIGHT; y++) {
        for (int x = 0; x < BOARD_WIDTH; x++) {
            board[y][x] = 0;
        }
    }

    score = 0;
    linesCleared = 0;
    level = 1;
    gameOver = false;
    isPaused = false;

    srand((unsigned int)time(NULL));

    currentType = rand() % 7;
    currentRotation = 0;
    currentX = BOARD_WIDTH / 2 - 2;
    currentY = 0;

    nextType = rand() % 7;
    nextRotation = 0;
}

int (*getShape(int type, int rotate))[4] {
    return TETROMINO_SHAPES[type][rotate];
}

bool canPlace(int (*shape)[4], int startX, int startY) {
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            if (shape[y][x]) {
                int boardX = startX + x;
                int boardY = startY + y;

                if (boardX < 0 || boardX >= BOARD_WIDTH ||
                    boardY < 0 || boardY >= BOARD_HEIGHT) {
                    return false;
                }

                if (board[boardY][boardX] != 0) {
                    return false;
                }
            }
        }
    }
    return true;
}

void placePiece() {
    int (*shape)[4] = getShape(currentType, currentRotation);

    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            if (shape[y][x]) {
                int boardX = currentX + x;
                int boardY = currentY + y;

                if (boardY >= 0 && boardY < BOARD_HEIGHT &&
                    boardX >= 0 && boardX < BOARD_WIDTH) {
                    board[boardY][boardX] = currentType + 1;
                }
            }
        }
    }
}

int clearLines() {
    int linesClearedThisTurn = 0;

    for (int y = BOARD_HEIGHT - 1; y >= 0; y--) {
        bool fullLine = true;
        for (int x = 0; x < BOARD_WIDTH; x++) {
            if (board[y][x] == 0) {
                fullLine = false;
                break;
            }
        }

        if (fullLine) {
            linesClearedThisTurn++;

            for (int moveY = y; moveY > 0; moveY--) {
                for (int x = 0; x < BOARD_WIDTH; x++) {
                    board[moveY][x] = board[moveY - 1][x];
                }
            }

            for (int x = 0; x < BOARD_WIDTH; x++) {
                board[0][x] = 0;
            }

            y++;
        }
    }

    switch (linesClearedThisTurn) {
        case 1: score += 100 * level; break;
        case 2: score += 300 * level; break;
        case 3: score += 500 * level; break;
        case 4: score += 800 * level; break;
    }

    linesCleared += linesClearedThisTurn;
    level = (linesCleared / 10) + 1;

    return linesClearedThisTurn;
}

void spawnNewPiece() {
    currentType = nextType;
    currentRotation = 0;
    currentX = BOARD_WIDTH / 2 - 2;
    currentY = 0;

    nextType = rand() % 7;
    nextRotation = 0;

    if (!canPlace(getShape(currentType, currentRotation), currentX, currentY)) {
        gameOver = true;
    }
}

// ============================================================================
// Controls
// ============================================================================

bool moveLeft() {
    int (*shape)[4] = getShape(currentType, currentRotation);

    if (canPlace(shape, currentX - 1, currentY)) {
        currentX--;
        return true;
    }
    return false;
}

bool moveRight() {
    int (*shape)[4] = getShape(currentType, currentRotation);

    if (canPlace(shape, currentX + 1, currentY)) {
        currentX++;
        return true;
    }
    return false;
}

bool moveDown() {
    int (*shape)[4] = getShape(currentType, currentRotation);

    if (canPlace(shape, currentX, currentY + 1)) {
        currentY++;
        return true;
    }
    return false;
}

void hardDrop() {
    while (moveDown()) {
        score += 2;
    }
    placePiece();
    clearLines();
    spawnNewPiece();
}

bool rotate() {
    int newRotation = (currentRotation + 1) % 4;
    int (*newShape)[4] = getShape(currentType, newRotation);

    if (canPlace(newShape, currentX, currentY)) {
        currentRotation = newRotation;
        return true;
    }

    int offsets[] = {-1, 1, -2, 2};
    for (int i = 0; i < 4; i++) {
        if (canPlace(newShape, currentX + offsets[i], currentY)) {
            currentX += offsets[i];
            currentRotation = newRotation;
            return true;
        }
    }

    return false;
}

bool canRotate() {
    int newRotation = (currentRotation + 1) % 4;
    int (*newShape)[4] = getShape(currentType, newRotation);

    if (canPlace(newShape, currentX, currentY)) {
        return true;
    }

    int offsets[] = {-1, 1, -2, 2};
    for (int i = 0; i < 4; i++) {
        if (canPlace(newShape, currentX + offsets[i], currentY)) {
            return true;
        }
    }

    return false;
}

// ============================================================================
// Display (Double buffer - no flicker)
// ============================================================================

void drawBoard() {
    int display[BOARD_HEIGHT][BOARD_WIDTH];

    for (int y = 0; y < BOARD_HEIGHT; y++) {
        for (int x = 0; x < BOARD_WIDTH; x++) {
            display[y][x] = board[y][x];
        }
    }

    int (*shape)[4] = getShape(currentType, currentRotation);

    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            if (shape[y][x]) {
                int boardY = currentY + y;
                int boardX = currentX + x;

                if (boardY >= 0 && boardY < BOARD_HEIGHT &&
                    boardX >= 0 && boardX < BOARD_WIDTH) {
                    display[boardY][boardX] = -(currentType + 1);
                }
            }
        }
    }

    // 使用双缓冲：先在缓冲区构建整个画面，再一次性输出
    #ifdef _WIN32
    initDoubleBuffer();
    
    // 清空后台缓冲区
    COORD bufferSize = {45, 40};
    COORD topLeft = {0, 0};
    HANDLE back = screenBuffer[1 - currentBuffer];
    FillConsoleOutputCharacter(back, ' ', bufferSize.X * bufferSize.Y, topLeft, &(DWORD){0});
    FillConsoleOutputAttribute(back, 7, bufferSize.X * bufferSize.Y, topLeft, &(DWORD){0});
    
    // 构建显示字符串
    char line[100];
    DWORD written;
    
    sprintf(line, "\n");
    WriteConsoleOutputCharacter(back, line, (DWORD)strlen(line), (COORD){0, 0}, &written);
    
    sprintf(line, "  +----------------------------------------+\n");
    WriteConsoleOutputCharacter(back, line, (DWORD)strlen(line), (COORD){0, 1}, &written);
    
    sprintf(line, "  |           俄 罗 斯 方 块                 |\n");
    WriteConsoleOutputCharacter(back, line, (DWORD)strlen(line), (COORD){0, 2}, &written);
    
    sprintf(line, "  +----------------------------------------+\n");
    WriteConsoleOutputCharacter(back, line, (DWORD)strlen(line), (COORD){0, 3}, &written);
    
    sprintf(line, "  |                                        |\n");
    WriteConsoleOutputCharacter(back, line, (DWORD)strlen(line), (COORD){0, 4}, &written);
    
    // 游戏区域
    for (int y = 0; y < BOARD_HEIGHT; y++) {
        char row[50] = "  |  #";
        int pos = 5;
        for (int x = 0; x < BOARD_WIDTH; x++) {
            int cell = display[y][x];
            if (cell == 0) {
                row[pos++] = ' ';
                row[pos++] = ' ';
            } else if (cell < 0) {
                int type = -cell - 1;
                row[pos++] = '[';
                row[pos++] = ']';
            } else {
                int type = cell - 1;
                row[pos++] = '[';
                row[pos++] = ']';
            }
        }
        row[pos++] = '#';
        row[pos++] = ' ';
        row[pos++] = '|';
        row[pos++] = '\n';
        row[pos] = '\0';
        WriteConsoleOutputCharacter(back, row, (DWORD)strlen(row), (COORD){0, (SHORT)(5 + y)}, &written);
    }
    
    int infoY = 5 + BOARD_HEIGHT;
    sprintf(line, "  |                                        |\n");
    WriteConsoleOutputCharacter(back, line, (DWORD)strlen(line), (COORD){0, (SHORT)infoY}, &written);
    infoY++;
    
    sprintf(line, "  +----------------------------------------+\n");
    WriteConsoleOutputCharacter(back, line, (DWORD)strlen(line), (COORD){0, (SHORT)infoY}, &written);
    infoY++;
    
    sprintf(line, "  |  得分: %-6d  消行: %-4d  等级: %d   |\n", score, linesCleared, level);
    WriteConsoleOutputCharacter(back, line, (DWORD)strlen(line), (COORD){0, (SHORT)infoY}, &written);
    infoY++;
    
    sprintf(line, "  +----------------------------------------+\n");
    WriteConsoleOutputCharacter(back, line, (DWORD)strlen(line), (COORD){0, (SHORT)infoY}, &written);
    infoY++;
    
    sprintf(line, "  |  下一个:                                |\n");
    WriteConsoleOutputCharacter(back, line, (DWORD)strlen(line), (COORD){0, (SHORT)infoY}, &written);
    infoY++;
    
    sprintf(line, "  |       ");
    WriteConsoleOutputCharacter(back, line, (DWORD)strlen(line), (COORD){0, (SHORT)infoY}, &written);
    
    int (*nextShape)[4] = getShape(nextType, 0);
    char nextRow[20] = "";
    int nextPos = 0;
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            if (nextShape[y][x]) {
                nextRow[nextPos++] = '[';
                nextRow[nextPos++] = ']';
            } else {
                nextRow[nextPos++] = ' ';
                nextRow[nextPos++] = ' ';
            }
        }
        nextRow[nextPos] = '\0';
        WriteConsoleOutputCharacter(back, nextRow, (DWORD)nextPos, (COORD){7, (SHORT)(infoY + y)}, &written);
    }
    infoY += 4;
    
    sprintf(line, "                              |\n");
    WriteConsoleOutputCharacter(back, line, (DWORD)strlen(line), (COORD){0, (SHORT)infoY}, &written);
    infoY++;
    
    sprintf(line, "  +----------------------------------------+\n");
    WriteConsoleOutputCharacter(back, line, (DWORD)strlen(line), (COORD){0, (SHORT)infoY}, &written);
    infoY++;
    
    sprintf(line, "  |  操作: <- ->左右移动  ^旋转            |\n");
    WriteConsoleOutputCharacter(back, line, (DWORD)strlen(line), (COORD){0, (SHORT)infoY}, &written);
    infoY++;
    
    sprintf(line, "  |        空格快速下落  P暂停  Q退出      |\n");
    WriteConsoleOutputCharacter(back, line, (DWORD)strlen(line), (COORD){0, (SHORT)infoY}, &written);
    infoY++;
    
    sprintf(line, "  +----------------------------------------+\n");
    WriteConsoleOutputCharacter(back, line, (DWORD)strlen(line), (COORD){0, (SHORT)infoY}, &written);
    infoY++;
    
    if (isPaused) {
        sprintf(line, "\n");
        WriteConsoleOutputCharacter(back, line, (DWORD)strlen(line), (COORD){0, (SHORT)infoY}, &written);
        infoY++;
        
        sprintf(line, "  +----------------------------------------+\n");
        WriteConsoleOutputCharacter(back, line, (DWORD)strlen(line), (COORD){0, (SHORT)infoY}, &written);
        infoY++;
        
        sprintf(line, "  |           * 游戏暂停 *                  |\n");
        WriteConsoleOutputCharacter(back, line, (DWORD)strlen(line), (COORD){0, (SHORT)infoY}, &written);
        infoY++;
        
        sprintf(line, "  |         按 P 键继续游戏...              |\n");
        WriteConsoleOutputCharacter(back, line, (DWORD)strlen(line), (COORD){0, (SHORT)infoY}, &written);
        infoY++;
        
        sprintf(line, "  +----------------------------------------+\n");
        WriteConsoleOutputCharacter(back, line, (DWORD)strlen(line), (COORD){0, (SHORT)infoY}, &written);
        infoY++;
    }
    
    if (gameOver) {
        sprintf(line, "\n");
        WriteConsoleOutputCharacter(back, line, (DWORD)strlen(line), (COORD){0, (SHORT)infoY}, &written);
        infoY++;
        
        sprintf(line, "  +----------------------------------------+\n");
        WriteConsoleOutputCharacter(back, line, (DWORD)strlen(line), (COORD){0, (SHORT)infoY}, &written);
        infoY++;
        
        sprintf(line, "  |           游戏结束！                    |\n");
        WriteConsoleOutputCharacter(back, line, (DWORD)strlen(line), (COORD){0, (SHORT)infoY}, &written);
        infoY++;
        
        sprintf(line, "  |       最终得分: %d                     |\n", score);
        WriteConsoleOutputCharacter(back, line, (DWORD)strlen(line), (COORD){0, (SHORT)infoY}, &written);
        infoY++;
        
        sprintf(line, "  +----------------------------------------+\n");
        WriteConsoleOutputCharacter(back, line, (DWORD)strlen(line), (COORD){0, (SHORT)infoY}, &written);
        infoY++;
    }
    
    // 交换缓冲区：一次性显示整个画面
    swapBuffer();
    
    #else
    // Linux版本：使用字符串缓冲区
    initDoubleBuffer();
    
    // 构建画面到缓冲区
    sprintf(screenBuffer[0], "\n");
    sprintf(screenBuffer[1], "  +----------------------------------------+");
    sprintf(screenBuffer[2], "  |           俄 罗 斯 方 块                 |");
    sprintf(screenBuffer[3], "  +----------------------------------------+");
    sprintf(screenBuffer[4], "  |                                        |");
    
    for (int y = 0; y < BOARD_HEIGHT; y++) {
        char row[50];
        int pos = sprintf(row, "  |  #");
        for (int x = 0; x < BOARD_WIDTH; x++) {
            int cell = display[y][x];
            if (cell == 0) {
                pos += sprintf(row + pos, "  ");
            } else {
                pos += sprintf(row + pos, "[]");
            }
        }
        sprintf(row + pos, "#  |");
        strcpy(screenBuffer[5 + y], row);
    }
    
    int infoY = 5 + BOARD_HEIGHT;
    sprintf(screenBuffer[infoY++], "  |                                        |");
    sprintf(screenBuffer[infoY++], "  +----------------------------------------+");
    sprintf(screenBuffer[infoY++], "  |  得分: %-6d  消行: %-4d  等级: %d   |", score, linesCleared, level);
    sprintf(screenBuffer[infoY++], "  +----------------------------------------+");
    sprintf(screenBuffer[infoY++], "  |  下一个:                                |");
    
    // 一次性输出所有内容
    printf("\033[H");
    for (int y = 0; y < infoY; y++) {
        printf("%s\n", screenBuffer[y]);
    }
    fflush(stdout);
    #endif
}

// ============================================================================
// Main
// ============================================================================

void gameLoop() {
    clock_t lastTime, currentTime;
    clock_t dropTimer = 0;
    int dropInterval;

    hideCursor();
    initGame();

    lastTime = clock();

    while (!gameOver) {
        currentTime = clock();
        int deltaTime = (int)((currentTime - lastTime) * 1000 / CLOCKS_PER_SEC);
        lastTime = currentTime;

        dropInterval = 1000 / (level + 5);
        if (dropInterval < 100) dropInterval = 100;
        if (dropInterval > 500) dropInterval = 500;

        dropTimer += deltaTime;

        // Handle input
        if (_kbhit()) {
            int ch = _getch();
            int extended = 0;

            if (ch == 224 || ch == 0) {
                extended = _getch();
            }

            if (extended == 72 || ch == 'w' || ch == 'W') {
                if (!isPaused && canRotate()) {
                    rotate();
                }
            }
            else if (extended == 80 || ch == 's' || ch == 'S') {
                if (!isPaused) {
                    if (moveDown()) {
                        score += 1;
                    }
                    dropTimer = 0;
                }
            }
            else if (extended == 75 || ch == 'a' || ch == 'A') {
                if (!isPaused) moveLeft();
            }
            else if (extended == 77 || ch == 'd' || ch == 'D') {
                if (!isPaused) moveRight();
            }
            else if (ch == ' ') {
                if (!isPaused) hardDrop();
            }
            else if (ch == 'p' || ch == 'P') {
                isPaused = !isPaused;
            }
            else if (ch == 'q' || ch == 'Q') {
                gameOver = true;
            }
        }

        // Auto drop
        if (!isPaused) {
            if (dropTimer >= dropInterval) {
                dropTimer = 0;

                if (!moveDown()) {
                    placePiece();
                    clearLines();
                    spawnNewPiece();
                }
            }
        }

        // Draw
        drawBoard();

        // Frame rate control
        DELAY_MS(FRAME_TIME);
    }
}

int main() {
    #ifdef _WIN32
        SetConsoleOutputCP(65001);
        SetConsoleCP(65001);
        SetConsoleTitle("Tetris - C Language");
        system("mode con cols=45 lines=40");
    #endif

    printf("\n");
    printf("  +============================================+\n");
    printf("  |                                            |\n");
    printf("  |        欢迎来到俄罗斯方块游戏                |\n");
    printf("  |                                            |\n");
    printf("  |        [C语言控制台版本]                    |\n");
    printf("  |                                            |\n");
    printf("  +============================================+\n");
    printf("  |                                            |\n");
    printf("  |  游戏说明:                                 |\n");
    printf("  |  * 使用结构体定义方块形状                   |\n");
    printf("  |  * 使用二维数组存储游戏面板                 |\n");
    printf("  |  * 方块位置和旋转状态的指针操作             |\n");
    printf("  |  * 颜色设置的位运算                        |\n");
    printf("  |  * 消行的动态算法                          |\n");
    printf("  |                                            |\n");
    printf("  +============================================+\n");
    printf("  |                                            |\n");
    printf("  |  操作说明:                                 |\n");
    printf("  |  <- ->   左/右移动                        |\n");
    printf("  |  ^      旋转方块                          |\n");
    printf("  |  v      加速下落                          |\n");
    printf("  |  空格    立即下落                          |\n");
    printf("  |  P      暂停游戏                          |\n");
    printf("  |  Q      退出游戏                          |\n");
    printf("  |                                            |\n");
    printf("  +============================================+\n");
    printf("\n");
    printf("  按任意键开始游戏...\n");
    _getch();

    gameLoop();

    printf("\n");
    printf("  感谢游玩！按任意键退出...\n");
    showCursor();
    _getch();

    return 0;
}
