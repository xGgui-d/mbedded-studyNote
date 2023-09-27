#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

/* 一条线 */
struct line_t {
    char start;
    char end;
    int weight;
};

/* U集合和S集合的元素 */
struct element_t {
    char e_point;
    int e_weight;
    int isInU; // 标志位，1代表U集合中(S中没有该元素) 0代表不在U集合中(S中存在该元素)
};
int s_count = 0;

char src = 'A'; // 起始点
/* 输入个路径的数据 */
#define LINE_COUNT 10 // 图的线数
#define POINT_COUNT 6 // 图的点数
struct line_t lines[LINE_COUNT] = {
    {'A', 'B', 10}, {'A', 'D', 4}, {'B', 'D', 2},
    {'B', 'C', 8}, {'B', 'E', 6}, {'F', 'C', 5},
    {'D', 'E', 6}, {'E', 'F', 12}, {'E', 'C', 1},
    {'D', 'C', 15}             
};
char point[POINT_COUNT] = {'A', 'B', 'C', 'D', 'E', 'F'}; // 所有点的集合

/* 以src为中心点计算U集合的元素 */
static void __calculate_U(struct element_t *src, struct element_t *U)
{
    int i, j;
    /* 遍历所有U中集合元素，并重新计算出U集合 */
    for(i = 0; i < POINT_COUNT; i++) {
        if(U[i].isInU == 0) { // 如果元素不在U中，下一个
            continue;
        }
        if((U[i].e_point == src->e_point)) { // 如果U中元素与起点相同或者，下一个
            U[i].isInU = 0;
            continue;
        }    
        for(j = 0; j < LINE_COUNT; j++) {
            if((U[i].e_point == lines[j].start) || (U[i].e_point == lines[j].end)) { // 先判断point在不在lines元素的成员里
                if((src->e_point == lines[j].start) || (src->e_point == lines[j].end)) { //如果在，继续判断src是否在lines元素的成员里
                    // 如果 src 和 point 都在，那么重新计算出U集合的元素
                    if(U[i].e_weight > lines[j].weight+src->e_weight)
                        U[i].e_weight = lines[j].weight+src->e_weight;
                    goto next_loop; // 跳出循环，执行下一个点
                }
            } 
        }
next_loop:;
    }
}

/* 计算U集合元素中的最小值并将元素移动到S集合中 */
static void cal_min_to_U(struct element_t *S, struct element_t *U)
{
    int i;
    int min_weight = 0;
    struct element_t *target;
    for(i = 0; i < POINT_COUNT; i++) {
        if(U[i].e_weight == INT_MAX || U[i].isInU == 0)
            continue;
        min_weight = U[i].e_weight;
        target = &U[i];
        break;
    }
    for(;i < POINT_COUNT; i++) {
        if(U[i].e_weight == INT_MAX || U[i].isInU == 0)
            continue;
        if(min_weight > U[i].e_weight) {
            min_weight = U[i].e_weight;
            target = &U[i];
        }    
    }
    // 将U集合中的元素添加到S集合当中
    S[s_count].e_point = target->e_point;
    S[s_count].e_weight = target->e_weight;
    s_count++;
    target->isInU = 0;
}

static void calculate_U(struct element_t *S, struct element_t *U)
{
    int i, j, m;
    for(i = 0; i < POINT_COUNT-1; i++) {
        for(j = 0; j < s_count; j++) {
            __calculate_U(&S[j], U);
        }
        cal_min_to_U(S, U);
        /* 打印测试 */
        printf("---------步骤<%d>------------\r\n",i);
        for(m = 0; m < POINT_COUNT; m++) {
            printf("U%d: e_point[%c] e_weight[%d]\r\n", m, U[m].e_point, U[m].e_weight);
        }
            printf("---------------------------\r\n");
        for(m = 0; m < s_count; m++) {
            printf("S%d: e_point[%c] e_weight[%d]\r\n", m, S[m].e_point, S[m].e_weight);
        }
         
    }
}

/* 目标：计算起点到各个顶点的距离 */
int main()
{
    int i;
    /* 初始化S集合 */
    struct element_t S[POINT_COUNT]; // S集合
    S[s_count].e_point = src;
    S[s_count].e_weight = 0;
    ++s_count;
    /* 初始化U集合 */
    struct element_t U[POINT_COUNT]; // U集合
    for(i = 0; i < POINT_COUNT; i++) {
        U[i].e_point = point[i];
        U[i].e_weight = INT_MAX;
        U[i].isInU = 1;
    }
    calculate_U(S, U);
    exit(0);
}