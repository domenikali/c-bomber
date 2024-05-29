#ifndef GAME_H
#define GAME_H
#include "utils.h"


struct Player_pos{
    uint8_t x;
    uint8_t y;
};
typedef struct Player_pos Player_pos;
typedef struct Player_pos Bomb_pos;

struct Grid{
    uint8_t height;
    uint8_t width;
    uint8_t ** map;
    Player_pos players_pos[4];
};
typedef struct Grid Grid;

struct Bomb{
    uint8_t time;
    Bomb_pos position;
};
typedef struct Bomb Bomb;


struct Bombs{
    uint8_t index;
    Bomb * bombs;
};
typedef struct Bombs Bombs;




Grid create_new_map(uint8_t height, uint8_t width);
void move_player(Grid * grid, int id,uint16_t act, Bombs * bombs, Partial_grid * partial);
void bomb_explode(Bomb_pos bomb_pos, Grid * grid, Partial_grid * partial);
int update_explosion_grid(Grid * grid , int y , int x , Partial_grid * partial);
void free_map(Grid grid);
void update_partial(Partial_grid * partial, Player_pos pos, uint8_t act);
void print_grid(Grid grid);
Bombs bombs_init(Grid grid);
void bomb_update(Bombs * bombs , Grid * grid, Partial_grid * partial);


#endif