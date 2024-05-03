#ifndef CONWAY_H
#define CONWAY_H

#include <stdint.h>
#include <stdbool.h>

#define STATE_DEAD 0
#define STATE_ALIVE 1

// 用作get越界时的返回值
#define STATE_NONE -1

// 其它状态，可扩展
#define STATE_SUPER 2

// 无需食物状态：这是因为食物在生成时会立刻被随机消耗

typedef int GridState;

// 康威生命游戏(Conway's Game of Life)
// 在一个二维方格中，每个格点有生存或者死亡的状态
// 每个格点在下一个世代的生死状态由周围8个格点的状态决定（硬边界 or 周期性边界条件）
// 基本演化规则如下：
// 1. 任何活格点，若邻居活格点数 < 2，则该格点在下一世代死亡
// 2. 任何活格点，若邻居活格点数 = 2 或 = 3，则该格点在下一世代存活
// 3. 任何活格点，若邻居活格点数 > 2，则该格点在下一世代死亡
// 4. 任何空格点，若邻居活格点数 = 3，则该格点在下一世代变为活格点
typedef struct
{
    // 行数
    uint16_t m;

    // 列数
    uint16_t n;

    GridState **_grids;

    // 其它状态，可扩展...
    int maxFoodNum;
    float density;
    bool food;
} Conway;

// 前言：涉及指针操作和内存分配，随时记得指针判空

// 构造新格点，分配内存
// 创建失败则返回NULL
// malloc()
Conway new_conway(const uint16_t m, const uint16_t n, bool b);

// 删除格点，回收内存
// free()
void delete_grids(Conway *c);

void set_density(Conway *c, float p);

void reverse_food_rule(Conway *c);

// 随机地初始化格点
void init_random(Conway *c);

int get_neighbor(const Conway *c,int x,int y);

void init_food(Conway *c);

// 将系统演化到下一世代
int next_generation(Conway *c);

// 自定义的系统演化
// void custom_next_generation(Conway *c);

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
GridState get_state(const Conway *c, const uint16_t x, const uint16_t y);

void set_state(Conway *c, const uint16_t x, const uint16_t y, GridState s);

// 获取格点下一个世代的状态
// 注意下标边界检查
// 0 <= x < m,
// 0 <= y < n,
GridState get_next_state(const Conway *c, const uint16_t x, const uint16_t y);

// 展示格点，一般来说是printf打印吧
// 不过长和宽设置的都是uint16_t类型，稍微大一点的格点就不好打印了
void show_conway(const Conway *c);

// 保存格点到文件（可能得考虑一下数据保存到文件的格式）
// 成功则返回0，失败返回非0值
int save_conway(const Conway *c, const char *path);

// 从文件读取格点
// 失败则Conway._grids = NULL
// 涉及malloc()
Conway new_conway_from_file(const char *path);

bool Conway_cmp(const Conway *a, const Conway *b);

bool Conway_empty(const Conway *c);

#endif