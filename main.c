#include <stdio.h>
#include <pthread.h>
#include "conway.h"
#include <unistd.h>

void clear_screen()
{
    // Use ANSI escape codes to clear the screen
    printf("\e[1;1H\e[2J");
    printf("\e[?25l");
}

void print_help()
{
    printf("--------------------\n");
    printf("i <row> <col>\t\tinitialize an empty grid with <row> rows and <col> columns\n");
    printf("r\t\t\trandomly set alive/dead states for all grids\n");
    printf("d <den>\t\t\tset the possibility of alive state to <den> when init grids\n");
    printf("n\t\t\tevolve into next generation\n");
    printf("c\t\t\tautomatically evolve, until receiving 'b' command\n");
    printf("b\t\t\tpause evolution\n");
    printf("s <path>\t\tsave grid states to file <path>\n");
    printf("l <path>\t\tload grid states from file <path> (with food rules opened)\n");
    printf("f\t\t\topen/close \"food\" rules (default=open)\n");
    printf("q\t\t\tquit\n");
    printf("h\t\t\thelp\n");
    printf("--------------------\n");
}

void print_game(const Conway *c)
{
    show_conway(c);
    print_help();
}

void *listen_break(void *flag)
{
    char c = 0;
    while (c != 'b')
    {
        scanf(" %c", &c);
        scanf("%*[^\n]"); // 清除未读内容
    }

    *(int *)flag = 1;
    return NULL;
}

void automatic_evolve(Conway *c)
{
    int flag = 0;
    pthread_t listener;
    pthread_create(&listener, NULL, listen_break, &flag);
    while (flag != 1)
    {
        int generation_state = next_generation(c);

        print_game(c);
        if (generation_state == 0)
        {
            printf("The grids are now stable!\n");
            printf("Enter 'b' to stop evolving!\n");
        }
        else if (generation_state == -1)
        {
            printf("The grids are now empty!\n");
            printf("Enter 'b' to stop evolving!\n");
        }
        printf("automatically evolving...\n");
        sleep(1); // 每秒演化一次
    }

    pthread_join(listener, NULL);
    print_game(c);
}

int main()
{
    Conway c = new_conway(0, 0, 1);
    print_game(&c);

    char running = 1;
    while (running)
    {
        char cmd;
        char path[40];
        scanf(" %c", &cmd);
        switch (cmd)
        {
        case 'c': // 自动演化指令
        {
            automatic_evolve(&c);
            break;
        }         // others
        case 'i': // 初始化指令
        {
            int m, n;
            scanf("%d%d", &m, &n);
            delete_grids(&c);
            c = new_conway(m, n, 1);
            print_game(&c);
            break;
        }
        case 'd': // 设置期望密度
        {
            float p;
            scanf("%f", &p);
            set_density(&c, p);
            break;
        }
        case 'r': // 随机设置状态
        {
            init_random(&c);
            print_game(&c);
            break;
        }
        case 'n': // 演化指令
        {
            next_generation(&c);
            print_game(&c);
            break;
        }
        case 's': // 保存文件指令
        {
            scanf("%s", path);
            save_conway(&c, path);
            printf("The grids are saved to %s successfully!\n", path);
            break;
        }
        case 'l': // 读取文件指令
        {
            scanf("%s", path);
            delete_grids(&c);
            c = new_conway_from_file(path);
            print_help();
            break;
        }
        case 'f': // 食物规则开关
        {
            reverse_food_rule(&c);
            printf("Now food rule is %s!\n", c.food ? "opened" : "closed");
            break;
        }
        case 'q': // 退出程序
        {
            delete_grids(&c);
            return 0;
        }
        case 'h': // 打印帮助菜单
        {
            print_help();
            break;
        }
        default: // 其他指令
        {
            break;
        }
        }
    }

    return 0;
}