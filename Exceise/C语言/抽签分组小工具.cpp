/*
 * ============================================================================
 *  抽签分组小工具 - 美化版
 *  配色：黑底绿字，简洁清晰
 * ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
    #include <windows.h>
    #include <conio.h>
#endif

#define MAX_NUM  100
#define MAX_NAME 50

// ============================================================================
// 颜色设置
// ============================================================================

#ifdef _WIN32

#define CLR_BLACK        0
#define CLR_DARKGRAY     8
#define CLR_GREEN        2
#define CLR_BRIGHT_GREEN 10
#define CLR_WHITE        7
#define CLR_BRIGHT_WHITE 15
#define CLR_YELLOW       14
#define CLR_RED          12
#define CLR_CYAN         11

void setColor(int textColor, int bgColor) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, (bgColor << 4) | textColor);
}

void resetColor() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, 7);
}

void clearInputBuffer() {
    while (getchar() != '\n');
}

#else

void setColor(int textColor, int bgColor) { }
void resetColor() { }
void clearInputBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

#endif

// ============================================================================
// UI 绘制函数
// ============================================================================

void printLine(int width, char c) {
    for (int i = 0; i < width; i++) printf("%c", c);
}

void printHeader(const char* title) {
    printf("\n  ");
    setColor(CLR_BRIGHT_GREEN, CLR_BLACK);
    printLine(50, '=');
    printf("\n  ");
    setColor(CLR_BRIGHT_WHITE, CLR_BLACK);
    int len = strlen(title);
    int pad = (50 - len) / 2;
    for (int i = 0; i < pad; i++) printf(" ");
    printf("%s", title);
    for (int i = 0; i < 50 - pad - len; i++) printf(" ");
    printf("\n  ");
    setColor(CLR_BRIGHT_GREEN, CLR_BLACK);
    printLine(50, '=');
    printf("\n");
    resetColor();
}

void printSeparator() {
    printf("  ");
    setColor(CLR_GREEN, CLR_BLACK);
    printLine(50, '-');
    printf("\n");
    resetColor();
}

// ============================================================================
// 抽奖功能
// ============================================================================

void drawLottery() {
    int total, result;

    printHeader("随 机 抽 奖");

    printf("\n");
    setColor(CLR_BRIGHT_WHITE, CLR_BLACK);
    printf("  请输入参与抽奖的总人数：");
    resetColor();

    if (scanf("%d", &total) != 1 || total <= 0) {
        printf("\n  ");
        setColor(CLR_RED, CLR_BLACK);
        printf("[错误] 输入无效！人数必须大于0\n");
        resetColor();
        clearInputBuffer();
        return;
    }

    result = rand() % total + 1;

    printf("\n");
    printSeparator();

    setColor(CLR_BRIGHT_GREEN, CLR_BLACK);
    printf("\n         +------------------------+\n");
    printf("         |     抽 签 结 果        |\n");
    printf("         +------------------------+\n");
    printf("         |                        |\n");
    printf("         |   中奖号码：");
    setColor(CLR_YELLOW, CLR_BLACK);
    printf("第 %3d 号", result);
    setColor(CLR_BRIGHT_GREEN, CLR_BLACK);
    printf("  |\n");
    printf("         |                        |\n");
    printf("         +------------------------+\n\n");

    setColor(CLR_YELLOW, CLR_BLACK);
    printf("              恭喜中奖者！\n\n");
    resetColor();
}

// ============================================================================
// 分组功能 - 全新美化输出
// ============================================================================

void groupMembers() {
    char names[MAX_NUM][MAX_NAME];
    int count = 0;
    int groupNum;

    printHeader("随 机 分 组");

    printf("\n");
    setColor(CLR_BRIGHT_WHITE, CLR_BLACK);
    printf("  请输入名字（每行一个），输入 end 结束\n");
    resetColor();
    printSeparator();
    printf("\n");

    // 输入名字
    while (count < MAX_NUM) {
        setColor(CLR_GREEN, CLR_BLACK);
        printf("  [%2d] ", count + 1);
        resetColor();

        char temp[MAX_NAME];
        if (scanf("%s", temp) != 1) break;

        if (strcmp(temp, "end") == 0 || strcmp(temp, "End") == 0 ||
            strcmp(temp, "END") == 0) break;

        strcpy(names[count], temp);
        count++;
    }

    if (count == 0) {
        printf("\n  ");
        setColor(CLR_RED, CLR_BLACK);
        printf("[错误] 没有输入任何成员！\n");
        resetColor();
        return;
    }

    printf("\n");
    printSeparator();

    setColor(CLR_BRIGHT_GREEN, CLR_BLACK);
    printf("  已录入 %d 名成员\n", count);
    resetColor();

    printSeparator();

    // 输入组数
    printf("\n");
    setColor(CLR_BRIGHT_WHITE, CLR_BLACK);
    printf("  请输入要分成的组数（2-%d）：", count > 50 ? 50 : count);
    resetColor();

    int maxGroup = count > 50 ? 50 : count;
    if (scanf("%d", &groupNum) != 1 || groupNum <= 0 || groupNum > maxGroup) {
        printf("\n  ");
        setColor(CLR_RED, CLR_BLACK);
        printf("[错误] 组数无效！\n");
        resetColor();
        clearInputBuffer();
        return;
    }

    // Fisher-Yates 洗牌
    for (int i = count - 1; i > 0; i--) {
        int r = rand() % (i + 1);
        char temp[MAX_NAME];
        strcpy(temp, names[i]);
        strcpy(names[i], names[r]);
        strcpy(names[r], temp);
    }

    // 计算每组人数
    int baseCount = count / groupNum;
    int extra = count % groupNum;

    // 输出分组结果
    printf("\n");
    printHeader("分 组 结 果");

    int nameIndex = 0;

    for (int g = 0; g < groupNum; g++) {
        int membersInGroup = baseCount + (g < extra ? 1 : 0);

        // 组标题 - 简洁的边框样式
        printf("\n  ");
        setColor(CLR_BRIGHT_GREEN, CLR_BLACK);
        printf("+--[ 第 %d 组 ]", g + 1);
        int titleLen = 12;
        for (int i = 0; i < 50 - titleLen; i++) printf("-");
        printf("+\n");

        // 成员列表 - 每行显示多个
        printf("  ");
        setColor(CLR_GREEN, CLR_BLACK);
        printf("| 成员：");
        setColor(CLR_BRIGHT_WHITE, CLR_BLACK);

        int linePos = 8;
        for (int i = 0; i < membersInGroup; i++) {
            if (nameIndex < count) {
                int nameLen = strlen(names[nameIndex]);

                // 换行检查
                if (linePos + nameLen + 4 > 48 && i > 0) {
                    // 填充当前行
                    for (int j = linePos; j < 49; j++) printf(" ");
                    setColor(CLR_GREEN, CLR_BLACK);
                    printf("|\n  |       ");
                    setColor(CLR_BRIGHT_WHITE, CLR_BLACK);
                    linePos = 8;
                }

                setColor(CLR_YELLOW, CLR_BLACK);
                printf("%s", names[nameIndex]);
                linePos += nameLen;
                nameIndex++;

                if (i < membersInGroup - 1) {
                    setColor(CLR_DARKGRAY, CLR_BLACK);
                    printf("  ");
                    linePos += 2;
                }
            }
        }

        // 填充剩余空间
        for (int i = linePos; i < 49; i++) printf(" ");
        setColor(CLR_GREEN, CLR_BLACK);
        printf("|\n");

        // 人数统计行
        printf("  ");
        setColor(CLR_GREEN, CLR_BLACK);
        printf("| 人数：");
        setColor(CLR_CYAN, CLR_BLACK);
        printf("%d 人", membersInGroup);
        for (int i = 0; i < 50 - 12; i++) printf(" ");
        setColor(CLR_GREEN, CLR_BLACK);
        printf("|\n");

        // 组底部边框
        printf("  ");
        setColor(CLR_BRIGHT_GREEN, CLR_BLACK);
        printLine(50, '-');
        printf("+\n");
    }

    resetColor();

    // 汇总信息
    printf("\n");
    setColor(CLR_BRIGHT_GREEN, CLR_BLACK);
    printf("  +");
    printLine(48, '=');
    printf("+\n");

    printf("  |");
    setColor(CLR_BRIGHT_WHITE, CLR_BLACK);
    printf("  汇总：共 %d 组，%d 名成员", groupNum, count);
    int sumLen = 16;
    while (sumLen++ < 48) printf(" ");
    setColor(CLR_BRIGHT_GREEN, CLR_BLACK);
    printf("|\n");

    printf("  |");
    setColor(CLR_GREEN, CLR_BLACK);
    printf("  每组人数：");
    setColor(CLR_YELLOW, CLR_BLACK);
    for (int i = 0; i < groupNum; i++) {
        int m = baseCount + (i < extra ? 1 : 0);
        printf("%d", m);
        if (i < groupNum - 1) {
            setColor(CLR_DARKGRAY, CLR_BLACK);
            printf(" | ");
            setColor(CLR_YELLOW, CLR_BLACK);
        }
    }
    int distLen = 11 + groupNum * 2 + (groupNum - 1) * 3;
    for (int i = distLen; i < 48; i++) printf(" ");
    setColor(CLR_BRIGHT_GREEN, CLR_BLACK);
    printf("|\n");

    printf("  +");
    printLine(48, '=');
    printf("+\n\n");

    setColor(CLR_YELLOW, CLR_BLACK);
    printf("        分组完成！\n\n");
    resetColor();
}

// ============================================================================
// 主函数
// ============================================================================

int main() {
    int choice;

    #ifdef _WIN32
        SetConsoleOutputCP(65001);
        SetConsoleCP(65001);
        SetConsoleTitle("抽签分组小工具");
        system("mode con cols=56 lines=40");
    #endif

    srand((unsigned)time(NULL));

    while (1) {
        printHeader("抽 签 分 组 小 工 具");

        printf("\n");

        // 菜单选项
        setColor(CLR_BRIGHT_GREEN, CLR_BLACK);
        printf("        [1] ");
        setColor(CLR_BRIGHT_WHITE, CLR_BLACK);
        printf("随机抽奖\n\n");

        setColor(CLR_BRIGHT_GREEN, CLR_BLACK);
        printf("        [2] ");
        setColor(CLR_BRIGHT_WHITE, CLR_BLACK);
        printf("随机分组\n\n");

        setColor(CLR_GREEN, CLR_BLACK);
        printf("        [0] ");
        setColor(CLR_WHITE, CLR_BLACK);
        printf("退出程序\n");

        printf("\n");
        printSeparator();

        printf("\n");
        setColor(CLR_BRIGHT_WHITE, CLR_BLACK);
        printf("  请选择功能：");
        resetColor();

        if (scanf("%d", &choice) != 1) {
            printf("\n  ");
            setColor(CLR_RED, CLR_BLACK);
            printf("[错误] 请输入数字！\n");
            resetColor();
            clearInputBuffer();
            continue;
        }

        switch (choice) {
            case 1:
                drawLottery();
                break;
            case 2:
                groupMembers();
                break;
            case 0:
                printHeader("感 谢 使 用");
                printf("\n  按任意键退出...\n");
                #ifdef _WIN32
                    _getch();
                #else
                    getchar();
                #endif
                return 0;
            default:
                printf("\n  ");
                setColor(CLR_RED, CLR_BLACK);
                printf("[错误] 无效选项！\n");
                resetColor();
                break;
        }

        printf("\n");
    }

    return 0;
}
