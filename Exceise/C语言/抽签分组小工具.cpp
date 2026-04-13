#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_NUM  100    // 最多组数
#define MAX_NAME 20     // 名字最长长度

// 抽签功能
void drawLottery() {
    int total, result;
    char choice;

    printf("\n===== 随机抽奖 =====\n");
    printf("请输入参与抽奖的总人数：");
    scanf("%d", &total);

    if (total <= 0) {
        printf("人数必须大于0！\n");
        return;
    }

    
    result = rand() % total + 1;
    printf("\n 抽签结果：第 %d 号中奖！\n", result);
       
}


// 分组功能
void groupMembers() {
    char names[MAX_NUM][MAX_NAME];
    int count = 0;
    int groupNum;

    printf("\n===== 随机分组 =====\n");
    printf("请输入名字（空格/换行分隔），输入 end 结束：\n");

    // 读取名字
    while (1) {
        char temp[MAX_NAME];
        scanf("%s", temp);
        if (strcmp(temp, "end") == 0) break;
        strcpy(names[count], temp);
        count++;
        if (count >= MAX_NUM) break;
    }

    if (count == 0) {
        printf("没有成员！\n");
        return;
    }

    printf("\n总数：%d\n", count);
    printf("请输入组数量：");
    scanf("%d", &groupNum);

    if (groupNum <= 0) {
        printf("组数量必须大于0！\n");
        return;
    }
    
    if (groupNum <= 0 || groupNum > count) {
    printf("组数量必须大于0且不超过总人数！\n");
    return;
}

    // 随机打乱顺序（公平洗牌）
    for (int i = 0; i < count; i++) {
        int r = rand() % count;
        char temp[MAX_NAME];
        strcpy(temp, names[i]);
        strcpy(names[i], names[r]);
        strcpy(names[r], temp);
    }

   // 输出分组
printf("\n===== 分组结果 =====\n");
for (int g = 0; g < groupNum; g++) {
    printf("\n第 %d 组：", g + 1);

    for (int i = g; i < count; i += groupNum) {
        printf(" %s", names[i]);
    }
}
printf("\n====================\n");
}
int main() {
    int choice;
    srand((unsigned)time(NULL));  // 设置随机种子

    while (1) {
        printf("\n===== 菜单 =====\n");
        printf("1. 随机抽奖\n");
        printf("2. 随机分组\n");
        printf("================\n");
        printf("请输入功能编号：");
        scanf("%d", &choice);

        if (choice == 1) {
            drawLottery();
        }
        else if (choice == 2) {
            groupMembers();
        }
        else {
            printf("输入错误，请重新选择！\n");
        }
    }
    return 0;
}
