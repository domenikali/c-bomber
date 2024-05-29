#ifndef NCURS_H
#define NCURS_H
#include <ncurses.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#define TEXT_SIZE 255
#define MAX_CHAT_MESAGES 100
typedef enum ACTION { SEND,NONE, UP, DOWN, LEFT, RIGHT, QUIT, BOMB } ACTION;
typedef struct board {
    char* grid;
    int w;
    int h;
} board;

typedef struct line {
    char data[TEXT_SIZE];
    int cursor;
} line;
typedef struct {
    line lines[TEXT_SIZE];
    int start;
    int count; 
} tchat;

typedef struct pos {
    int x;
    int y;
} pos;

void setup_board(board* board);
void free_board(board* board);
int get_grid(board* b, int x, int y);
void set_grid(board* b, int x, int y, int v);
void refresh_game(board* b, line* l, char chat_history[MAX_CHAT_MESAGES][TEXT_SIZE], int nbchat);
ACTION control(line* l) ;
bool perform_action(board* b, pos* p, ACTION a);
#endif