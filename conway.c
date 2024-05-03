#include "conway.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <stdbool.h>

// 构造新格点，分配内存
// malloc()
Conway new_conway(const uint16_t m, const uint16_t n, bool b)
{
    if (m == 0 || n == 0)
        return (Conway){0, 0, NULL, 0, 0, 0};
    Conway res;
    res.m = m;
    res.n = n;
    res.density = 0.5;
    res.food = b;
    res.maxFoodNum = ceil((m + n) / 4.0);
    // 食物数量可以随Conway结构记录：(average of {m,n})/2.
    res._grids = (int **)malloc(m * sizeof(int *));
    for (int i = 0; i < m; i++)
        res._grids[i] = (int *)malloc(n * sizeof(int));
    for (int i = 0; i < m; i++)
        for (int j = 0; j < n; j++)
            res._grids[i][j] = STATE_DEAD;
    return res;
}

// 删除格点，回收分配给格点二维数组的内存
// Conway游戏本身的结构体 c 不用删除，只删除其格点
// 使用free()
void delete_grids(Conway *c)
{
    if (c == NULL)
        return;
    for (int i = 0; i < c->m; i++)
        free(c->_grids[i]);
    free(c->_grids);
}
//设置密度（初始存活概率）
void set_density(Conway *c, float p)
{
    if (c == NULL)
        return;
    c->density = p;
}
//食物开关
void reverse_food_rule(Conway *c)
{
    if (c == NULL)
        return;
    c->food = !c->food;
}

// 随机地初始化格点
void init_random(Conway *c)
{
    if (c == NULL)
        return;
    srand(time(NULL));
    float r;
    for (int i = 0; i < c->m; i++)
        for (int j = 0; j < c->n; j++)
        {
            // 通过随机浮点数设置生死
            r = rand() * 1.0 / RAND_MAX;
            set_state(c, i, j, r < c->density ? STATE_ALIVE : STATE_DEAD);
        }
}

const int dx[4] = {-1, 1, 0, 0}, dy[4] = {0, 0, -1, 1};
//统计(x,y)的邻居中存活点的数量（超出边界不计数）
int get_neighbor(const Conway *c, int x, int y)
{
    if (c == NULL)
        return 0;
    int res = 0;
    for (int i = 0; i < 4; i++)
        if (get_state(c, x + dx[i], y + dy[i]) == STATE_ALIVE || get_state(c, x + dx[i], y + dy[i]) == STATE_SUPER)
            res++;
    return res;
}

// 附加功能：生成食物
void init_food(Conway *c)
{
    if (c == NULL)
        return;
    srand(time(NULL));
    int cnt = 0; // 记录已经生成的食物数量
    while (cnt < c->maxFoodNum)
    {
        // 随机选择食物位置
        int nx = rand() % c->m, ny = rand() % c->n;
        // 该位置已经是强化细胞：放弃生成该食物
        if (get_state(c, nx, ny) == STATE_SUPER)
            continue;
        // 该位置有活细胞：直接吃掉
        if (get_state(c, nx, ny) == STATE_ALIVE)
        {
            {
                set_state(c, nx, ny, STATE_SUPER);
                // printf("(eat)set super at %d,%d\n", nx, ny);
            }
            cnt++;
            continue;
        }
        // 该位置没有细胞：考虑周围的细胞
        if (get_state(c, nx, ny) == STATE_DEAD)
        {
            int alive_cnt = get_neighbor(c, nx, ny);
            // 周围没有细胞：该食物被浪费，但仍然记入已生成食物的个数
            if (alive_cnt == 0)
            {
                cnt++;
                continue;
            }
            // 周围正好有1个细胞：走过来、吃掉
            else if (alive_cnt == 1)
            {
                for (int i = 0; i < 4; i++)
                    if (get_state(c, nx + dx[i], ny + dy[i]) == STATE_ALIVE)
                        set_state(c, nx + dx[i], ny + dy[i], STATE_DEAD);
                {
                    // printf("(1)set super at %d,%d\n", nx, ny);
                    set_state(c, nx, ny, STATE_SUPER);
                }
                cnt++;
                continue;
            }
            // 周围有多个细胞：随机一个走过来、吃掉
            else
            {
                int alives[4], tmp = 0;
                for (int i = 0; i < 4; i++)
                    if (get_state(c, nx + dx[i], ny + dy[i]) == STATE_ALIVE)
                        alives[tmp++] = i;
                int luckynum = rand() % alive_cnt;
                set_state(c, nx + dx[alives[luckynum]], ny + dy[alives[luckynum]], STATE_DEAD);
                {

                    // printf("(%d)set super at %d,%d\n", alive_cnt, nx, ny);
                    set_state(c, nx, ny, STATE_SUPER);
                }
                cnt++;
                continue;
            }
        }
    }
}

// 将系统演化到下一世代
int next_generation(Conway *c)
{
    if (c == NULL)
        return 1;
    if (c->food)
    {
        // printf("init food!\n");
        init_food(c);
    }
    Conway nxt = new_conway(c->m, c->n, c->food);
    for (int i = 0; i < nxt.m; i++)
        for (int j = 0; j < nxt.n; j++)
            set_state(&nxt, i, j, get_next_state(c, i, j));
    if (c->food == 0 && Conway_cmp(c, &nxt))
        return 0;
    *c = nxt;
    if (Conway_empty(&nxt))
        return -1;
    else
        return 1;
}

// 获取格点的当前状态
// 注意下标边界检查
// 0 <= x < m,
// 0 <= y < n,
// 虽然看上去这样一个封装没有必要，毕竟可以 c->_grids[x][y]来访问
// 但是封装后会安全一点
// 越界或者遇到空指针返回GridState::None ?
// if (get_current_state(c, x, y) == GridState::None) {
//     // balabalabala
// }
GridState get_state(const Conway *c, const uint16_t x, const uint16_t y)
{
    if (c == NULL)
        return STATE_NONE;
    // 由于x,y都是uint16_t类型，实际上并不会<0
    if (x >= c->m || y >= c->n)
        return STATE_NONE;
    return c->_grids[x][y];
}

void set_state(Conway *c, const uint16_t x, const uint16_t y, GridState s)
{
    if (c == NULL)
        return;
    if (get_state(c, x, y) == STATE_NONE)
        return;
    c->_grids[x][y] = s;
}

// 获取格点下一个世代的状态
// 注意下标边界检查
// 0 <= x < m,
// 0 <= y < n,
GridState get_next_state(const Conway *c, const uint16_t x, const uint16_t y)
{
    if (get_state(c, x, y) == STATE_NONE)
        return STATE_NONE;
    if (get_state(c, x, y) == STATE_SUPER)
    {
        // printf("super to alive %d %d\n", x, y);
        return STATE_ALIVE;
    }
    int cnt = get_neighbor(c, x, y);
    if (get_state(c, x, y) == STATE_DEAD && cnt == 3)
    {
        // printf("birth %d %d\n", x, y);
        return STATE_ALIVE;
    }
    if (get_state(c, x, y) == STATE_ALIVE)
    {
        if (cnt == 2 || cnt == 3)
        {
            // printf("survive %d %d\n", x, y);
            return STATE_ALIVE;
        }
        else
            return STATE_DEAD;
    }
    return STATE_DEAD;
}

// 展示格点，一般来说是printf打印吧
// 不过长和宽设置的都是uint16_t类型，稍微大一点的格点就不好打印了
void show_conway(const Conway *c)
{
    if (c == NULL)
        return;
    for (int i = 0; i < c->m; i++)
    {
        printf("|");
        for (int j = 0; j < c->n; j++)
        {
            char now = get_state(c, i, j) == STATE_DEAD ? ' ' : '*';
            printf("%c|", now);
        }
        printf("\n");
    }
}

// 保存格点到文件（可能得考虑一下数据保存到文件的格式）
// 成功则返回0，失败返回非0值
int save_conway(const Conway *c, const char *path)
{
    if (c == NULL)
        return -1;
    FILE *fp = fopen(path, "w");
    if (fp == NULL)
    {
        printf("File path error!\n");
        fclose(fp);
        return -1;
    }
    fprintf(fp, "%d,%d\n", c->m, c->n);
    for (int i = 0; i < c->m; i++)
    {
        for (int j = 0; j < c->n; j++)
        {
            fprintf(fp, "%d,", get_state(c, i, j));
        }
        fprintf(fp, "\n");
    }
    fclose(fp);
    return 0;
}

// 从文件读取格点
// 失败则Conway._grids = NULL
// 涉及malloc()
Conway new_conway_from_file(const char *path)
{
    FILE *fp = fopen(path, "r");
    if (fp == NULL)
    {
        printf("No such file or directory!\n");
        fclose(fp);
        return (Conway){0, 0, NULL, 0, 0.0, 0};
    }
    int m, n;
    fscanf(fp, "%d,%d", &m, &n);
    if (m <= 0 || n <= 0)
    {
        printf("The width and height must be positive numbers!\n");
        fclose(fp);
        return (Conway){0, 0, NULL, 0, 0.0, 0};
    }
    Conway res = new_conway(m, n, 1);
    for (int i = 0; i < m; i++)
        for (int j = 0; j < n; j++)
            fscanf(fp, "%d,", &res._grids[i][j]);
    fclose(fp);
    show_conway(&res);
    printf("The grids are loaded successfully!\n");
    return res;
}
//比较两个状态是否相同
bool Conway_cmp(const Conway *a, const Conway *b)
{
    if (a == NULL || b == NULL)
        return 0;
    if (a->m != b->m || a->n != b->n)
        return 0;
    for (int i = 0; i < a->m; i++)
        for (int j = 0; j < a->n; j++)
            if (get_state(a, i, j) != get_state(b, i, j))
                return 0;
    return 1;
}
//判断一个状态是否为全死亡
bool Conway_empty(const Conway *c)
{
    if (c == NULL)
        return 0;
    for (int i = 0; i < c->m; i++)
        for (int j = 0; j < c->n; j++)
            if (get_state(c, i, j) != STATE_DEAD)
                return 0;
    return 1;
}