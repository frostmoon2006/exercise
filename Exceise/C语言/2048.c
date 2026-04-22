/*
 * ============================================================================
 *  2048 小游戏 - C 语言控制台版
 * ============================================================================
 * 
 * 【游戏规则】
 * - 4x4 网格，通过方向键或WASD控制
 * - 相同数字的方块碰撞会合并为两倍
 * - 每次移动后随机生成一个新方块（2或4）
 * - 达到2048方块即胜利
 * - 无法移动时游戏结束
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
// 【常量定义】
// ============================================================================

#define GRID_SIZE    4       // 网格大小
#define TARGET       2048    // 目标数字

// 颜色定义
#ifdef _WIN32
    #define COLOR_DEFAULT  7
    #define COLOR_WALL     8
#else
    #define COLOR_DEFAULT  37
    #define COLOR_WALL     90
#endif

// 方块数字对应的颜色代码
typedef struct {
    int value;
    int color;
} ColorPair;

ColorPair COLORS[] = {
    {0, 0},      // 空 - 黑色
    {2, 240},    // 2 - 亮白
    {4, 220},    // 4 - 亮黄
    {8, 208},    // 8 - 亮橙
    {16, 202},   // 16 - 橙色
    {32, 196},   // 32 - 亮红
    {64, 160},   // 64 - 红色
    {128, 226},  // 128 - 亮绿
    {256, 82},   // 256 - 绿色
    {512, 51},   // 512 - 深绿
    {1024, 21},  // 1024 - 亮蓝
    {2048, 21},  // 2048 - 金色（特殊）
};

// ============================================================================
// 【全局变量】
// ============================================================================

int board[GRID_SIZE][GRID_SIZE];
int score = 0;
int bestScore = 0;
bool gameOver = false;
bool won = false;

// ============================================================================
// 【Windows 控制台设置】
// ============================================================================

#ifdef _WIN32
void setColor(int color) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
}

void resetColor() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, 7);
}

/* No-flicker: position cursor at top-left instead of clearing screen */
void gotoxy(int x, int y) {
    COORD coord;
    coord.X = (SHORT)x;
    coord.Y = (SHORT)y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

void clearScreenBuffer() {
    COORD topLeft = {0, 0};
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO screen;
    DWORD written;
    GetConsoleScreenBufferInfo(console, &screen);
    FillConsoleOutputCharacterA(console, ' ', screen.dwSize.X * screen.dwSize.Y, topLeft, &written);
    SetConsoleCursorPosition(console, topLeft);
}

void hideCursor() {
    CONSOLE_CURSOR_INFO info;
    info.bVisible = FALSE;
    info.dwSize = 100;
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);
}
#else
void setColor(int color) {
    printf("\033[%dm", color);
}

void resetColor() {
    printf("\033[0m");
}

void gotoxy(int x, int y) {
    printf("\033[%d;%dH", y + 1, x + 1);
    fflush(stdout);
}

void hideCursor() { }
#endif

// ============================================================================
// 【游戏核心函数】
// ============================================================================

/* 函数前置声明 */
void addRandomTile(void);

/* 初始化游戏 */
void initGame() {
    memset(board, 0, sizeof(board));
    score = 0;
    gameOver = false;
    won = false;
    
    /* 初始生成两个方块 */
    addRandomTile();
    addRandomTile();
}

/* 添加随机方块 */
void addRandomTile() {
    int emptyCells[GRID_SIZE * GRID_SIZE][2];
    int count = 0;
    
    /* 找出所有空格 */
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            if (board[i][j] == 0) {
                emptyCells[count][0] = i;
                emptyCells[count][1] = j;
                count++;
            }
        }
    }
    
    if (count > 0) {
        int idx = rand() % count;
        int row = emptyCells[idx][0];
        int col = emptyCells[idx][1];
        /* 90%概率生成2，10%概率生成4 */
        board[row][col] = (rand() % 10 < 9) ? 2 : 4;
    }
}

/* 获取方块颜色 */
int getTileColor(int value) {
    switch (value) {
        case 0:    return 0;
        case 2:    return 15;   // 亮白
        case 4:    return 14;  // 亮黄
        case 8:    return 12;  // 亮红
        case 16:   return 6;   // 橙色
        case 32:   return 4;   // 红色
        case 64:   return 1;   // 深红
        case 128:  return 10;  // 亮绿
        case 256:  return 2;   // 绿色
        case 512:  return 3;   // 深绿
        case 1024: return 11;  // 亮蓝
        case 2048: return 13;  // 粉色（胜利色）
        default:    return 15;
    }
}

/* 绘制单个方块 */
void drawTile(int value) {
    if (value == 0) {
        printf("       ");
    } else {
        setColor(getTileColor(value));
        printf("%6d ", value);
        resetColor();
    }
}

/* 绘制游戏界面 - No flicker: use gotoxy instead of clear screen */
void drawBoard() {
    // 移到左上角开始绘制
    gotoxy(0, 0);
    clearScreenBuffer();  // 先清屏再绘制，确保无残留
    gotoxy(0, 0);         // 清屏后再定位到开头
    
    printf("\n");
    printf("  +========================================+\n");
    printf("  |          2 0 4 8 小游戏               |\n");
    printf("  +========================================+\n");
    printf("\n");
    
    /* 分数显示 */
    printf("    +----------+    +----------+\n");
    printf("    |   得分   |    |   最高   |\n");
    setColor(14);
    printf("    |  %6d  |    |", score);
    resetColor();
    setColor(10);
    printf("  %6d  |\n", bestScore);
    resetColor();
    printf("    +----------+    +----------+\n");
    printf("\n");
    
    /* 游戏网格 */
    printf("    +------+------+------+------+\n");
    for (int i = 0; i < GRID_SIZE; i++) {
        printf("    |");
        for (int j = 0; j < GRID_SIZE; j++) {
            drawTile(board[i][j]);
            printf("|");
        }
        printf("\n");
        printf("    +------+------+------+------+\n");
    }
    
    printf("\n");
    printf("  +========================================+\n");
    printf("  |  控制: W/上 移动方块                   |\n");
    printf("  |       A/S/D 移动方块                   |\n");
    printf("  |       R 重新开始                       |\n");
    printf("  |       Q 退出游戏                       |\n");
    printf("  +========================================+\n");
    
    /* 胜利/失败提示 */
    if (won) {
        printf("\n");
        setColor(13);
        printf("  * 恭喜达到 2048！您已获胜！*\n");
        printf("  * 按 R 继续游戏以挑战更高分 *\n");
        resetColor();
    } else if (gameOver) {
        printf("\n");
        setColor(12);
        printf("  * 游戏结束！按 R 重新开始 *\n");
        resetColor();
    }
}

/* 向左移动并合并 */
bool moveLeft() {
    bool moved = false;
    
    for (int i = 0; i < GRID_SIZE; i++) {
        /* 压缩：把所有非零元素移到左边 */
        int writePos = 0;
        int temp[GRID_SIZE] = {0};
        
        for (int j = 0; j < GRID_SIZE; j++) {
            if (board[i][j] != 0) {
                temp[writePos++] = board[i][j];
            }
        }
        
        /* 合并相邻的相同元素 */
        for (int j = 0; j < GRID_SIZE - 1; j++) {
            if (temp[j] == temp[j + 1] && temp[j] != 0) {
                temp[j] *= 2;
                score += temp[j];
                if (temp[j] == TARGET) {
                    won = true;
                }
                temp[j + 1] = 0;
            }
        }
        
        /* 再次压缩 */
        int writePos2 = 0;
        int temp2[GRID_SIZE] = {0};
        for (int j = 0; j < GRID_SIZE; j++) {
            if (temp[j] != 0) {
                temp2[writePos2++] = temp[j];
            }
        }
        
        /* 检查是否移动了 */
        for (int j = 0; j < GRID_SIZE; j++) {
            if (board[i][j] != temp2[j]) {
                moved = true;
            }
            board[i][j] = temp2[j];
        }
    }
    
    return moved;
}

/* 向右移动并合并 */
bool moveRight() {
    bool moved = false;
    
    for (int i = 0; i < GRID_SIZE; i++) {
        int temp[GRID_SIZE] = {0};
        int writePos = GRID_SIZE - 1;
        
        /* 从右到左压缩 */
        for (int j = GRID_SIZE - 1; j >= 0; j--) {
            if (board[i][j] != 0) {
                temp[writePos--] = board[i][j];
            }
        }
        
        /* 合并 */
        for (int j = GRID_SIZE - 1; j > 0; j--) {
            if (temp[j] == temp[j - 1] && temp[j] != 0) {
                temp[j] *= 2;
                score += temp[j];
                if (temp[j] == TARGET) {
                    won = true;
                }
                temp[j - 1] = 0;
            }
        }
        
        /* 再次压缩 */
        int temp2[GRID_SIZE] = {0};
        int writePos2 = GRID_SIZE - 1;
        for (int j = GRID_SIZE - 1; j >= 0; j--) {
            if (temp[j] != 0) {
                temp2[writePos2--] = temp[j];
            }
        }
        
        /* 检查是否移动了 */
        for (int j = 0; j < GRID_SIZE; j++) {
            if (board[i][j] != temp2[j]) {
                moved = true;
            }
            board[i][j] = temp2[j];
        }
    }
    
    return moved;
}

/* 向上移动并合并 */
bool moveUp() {
    bool moved = false;
    
    for (int j = 0; j < GRID_SIZE; j++) {
        int temp[GRID_SIZE] = {0};
        int writePos = 0;
        
        /* 从上到下压缩 */
        for (int i = 0; i < GRID_SIZE; i++) {
            if (board[i][j] != 0) {
                temp[writePos++] = board[i][j];
            }
        }
        
        /* 合并 */
        for (int i = 0; i < GRID_SIZE - 1; i++) {
            if (temp[i] == temp[i + 1] && temp[i] != 0) {
                temp[i] *= 2;
                score += temp[i];
                if (temp[i] == TARGET) {
                    won = true;
                }
                temp[i + 1] = 0;
            }
        }
        
        /* 再次压缩 */
        int temp2[GRID_SIZE] = {0};
        writePos = 0;
        for (int i = 0; i < GRID_SIZE; i++) {
            if (temp[i] != 0) {
                temp2[writePos++] = temp[i];
            }
        }
        
        /* 检查是否移动了 */
        for (int i = 0; i < GRID_SIZE; i++) {
            if (board[i][j] != temp2[i]) {
                moved = true;
            }
            board[i][j] = temp2[i];
        }
    }
    
    return moved;
}

/* 向下移动并合并 */
bool moveDown() {
    bool moved = false;
    
    for (int j = 0; j < GRID_SIZE; j++) {
        int temp[GRID_SIZE] = {0};
        int writePos = GRID_SIZE - 1;
        
        /* 从下到上压缩 */
        for (int i = GRID_SIZE - 1; i >= 0; i--) {
            if (board[i][j] != 0) {
                temp[writePos--] = board[i][j];
            }
        }
        
        /* 合并 */
        for (int i = GRID_SIZE - 1; i > 0; i--) {
            if (temp[i] == temp[i - 1] && temp[i] != 0) {
                temp[i] *= 2;
                score += temp[i];
                if (temp[i] == TARGET) {
                    won = true;
                }
                temp[i - 1] = 0;
            }
        }
        
        /* 再次压缩 */
        int temp2[GRID_SIZE] = {0};
        writePos = GRID_SIZE - 1;
        for (int i = GRID_SIZE - 1; i >= 0; i--) {
            if (temp[i] != 0) {
                temp2[writePos--] = temp[i];
            }
        }
        
        /* 检查是否移动了 */
        for (int i = 0; i < GRID_SIZE; i++) {
            if (board[i][j] != temp2[i]) {
                moved = true;
            }
            board[i][j] = temp2[i];
        }
    }
    
    return moved;
}

/* 检查是否还有可移动的位置 */
bool canMove() {
    /* 检查是否有空格 */
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            if (board[i][j] == 0) {
                return true;
            }
        }
    }
    
    /* 检查是否有可以合并的相邻方块 */
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            /* 检查右边 */
            if (j < GRID_SIZE - 1 && board[i][j] == board[i][j + 1]) {
                return true;
            }
            /* 检查下边 */
            if (i < GRID_SIZE - 1 && board[i][j] == board[i + 1][j]) {
                return true;
            }
        }
    }
    
    return false;
}

/* 处理输入 */
void handleInput() {
    if (_kbhit()) {
        int ch = _getch();
        int extended = 0;
        
        /* 检查是否是扩展键（方向键） */
        if (ch == 224 || ch == 0) {
            extended = _getch();
        }
        
        bool moved = false;
        
        if (extended == 72 || ch == 'w' || ch == 'W') {  // 上
            moved = moveUp();
        }
        else if (extended == 80 || ch == 's' || ch == 'S') {  // 下
            moved = moveDown();
        }
        else if (extended == 75 || ch == 'a' || ch == 'A') {  // 左
            moved = moveLeft();
        }
        else if (extended == 77 || ch == 'd' || ch == 'D') {  // 右
            moved = moveRight();
        }
        else if (ch == 'r' || ch == 'R') {  // 重新开始
            if (score > bestScore) {
                bestScore = score;
            }
            initGame();
        }
        else if (ch == 'q' || ch == 'Q') {  // 退出
            gameOver = true;
        }
        
        /* 如果有移动，添加新的随机方块 */
        if (moved) {
            addRandomTile();
            
            /* 更新最高分 */
            if (score > bestScore) {
                bestScore = score;
            }
            
            /* 检查游戏是否结束 */
            if (!canMove()) {
                gameOver = true;
            }
        }
    }
}

// ============================================================================
// 【主函数】
// ============================================================================

int main() {
    #ifdef _WIN32
        SetConsoleOutputCP(65001);
        SetConsoleCP(65001);
        SetConsoleTitle("2048 小游戏");
        system("mode con cols=50 lines=25");
    #endif
    
    srand((unsigned)time(NULL));
    hideCursor();
    
    /* 打印欢迎信息 */
    printf("\n");
    printf("  +========================================+\n");
    printf("  |                                        |\n");
    printf("  |          欢 迎 玩 耍 2 0 4 8           |\n");
    printf("  |                                        |\n");
    printf("  |           C 语言控制台版               |\n");
    printf("  |                                        |\n");
    printf("  +========================================+\n");
    printf("\n");
    printf("  游戏规则：\n");
    printf("  * 使用方向键或WASD移动方块\n");
    printf("  * 相同数字碰撞会合并\n");
    printf("  * 达到 2048 即获胜！\n");
    printf("\n");
    printf("  按任意键开始游戏...\n");
    _getch();
    
    /* 初始化游戏 */
    initGame();
    
    /* 游戏主循环 */
    while (!gameOver) {
        drawBoard();
        handleInput();
        DELAY_MS(50);  /* 约20 FPS更新 */
    }
    
    /* 游戏结束 */
    drawBoard();
    printf("\n");
    printf("  最终得分: %d\n", score);
    printf("  最高分: %d\n", bestScore);
    printf("\n");
    printf("  感谢游玩！按任意键退出...\n");
    _getch();
    
    return 0;
}
