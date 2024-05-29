// Build with -lncurses option
#include "ncurs.h"
void setup_board(board* board) {
    int lines; int columns;
    getmaxyx(stdscr,lines,columns);
    board->h = 23; // 2 rows reserved for border, 1 row for chat
    board->w =23; // 2 columns reserved for border
    board->grid = calloc((board->w)*(board->h),sizeof(char));
}

void free_board(board* board) {
    free(board->grid);
}

int get_grid(board* b, int x, int y) {
    return b->grid[y*b->w + x];
}

void set_grid(board* b, int x, int y, int v) {
    b->grid[y*b->w + x] = v;
}
void refresh_game(board* b, line* l, char chat_history[MAX_CHAT_MESAGES][TEXT_SIZE], int nbchat) {
    // Update grid
    int x,y;
    for (y = 0; y < b->h; y++) {
        for (x = 0; x < b->w; x++) {
            char c;
            switch (get_grid(b,x,y)) {
                case 0:
                    c = ' ';
                    break;
                case 1:
                    c = 'O';//soft wall
                    break;

                case 2: 
                    c = '0';//hard wall
                    break;
                
                case 3:
                    c = 'X';//bomb
                    break;
                default:
                    c = '?';
                    break;
            }
            mvaddch(y+1,x+1,c);
        }
    }
    for (x = 0; x < b->w+2; x++) {
        mvaddch(0, x, '-');
        mvaddch(b->h+1, x, '-');
    }
    for (y = 0; y < b->h+2; y++) {
        mvaddch(y, 0, '|');
        mvaddch(y, b->w+1, '|');
    }
    // Update chat text
    attron(COLOR_PAIR(1)); // Enable custom color 1
    attron(A_BOLD); // Enable bold
    for (x = 0; x < b->w+2; x++) {
       mvaddch(b->h+2, x, ' ');
    }
    for (x = 0; x < b->w+2; x++) {
        if (x >= TEXT_SIZE || x >= l->cursor){
            mvaddch(b->h+2, x, ' ');
        }
            
        else{
            mvaddch(b->h+2, x, l->data[x]);
        }
    }
    for (int i = 0; i < nbchat; i++)
    {
        for (x = 0; x < strlen(chat_history[i]); x++) {
            if (x >= TEXT_SIZE)
            {
                mvaddch(b->h+2+i, x, ' ');
            }
                
            else{
                mvaddch(b->h+2+i, x, chat_history[i][x]);
            }
                
        }
    }
    
    attroff(A_BOLD); // Disable bold
    attroff(COLOR_PAIR(1)); // Disable custom color 1
    refresh(); // Apply the changes to the terminal
}

ACTION control(line* l) {
    int c;
    int prev_c = ERR;
    // We consume all similar consecutive key presses
    while ((c = getch()) != ERR) { // getch returns the first key press in the queue
        if (prev_c != ERR && prev_c != c) {
            ungetch(c); // put 'c' back in the queue
            break;
        }
        prev_c = c;
    }
    ACTION a = NONE;
    
    switch (prev_c) {
        case ERR: break;
        case KEY_LEFT:
            a = LEFT; break;
        case KEY_RIGHT:
            a = RIGHT; break;
        case KEY_UP:
            a = UP; break;
        case KEY_DOWN:
            a = DOWN; break;
        case 113:
            a = QUIT; break;
        case 120: //space
            a = BOMB;break;
        case KEY_BACKSPACE:
            if (l->cursor > 0) l->cursor--;
            break;
        case 10:
            a=SEND; break;
        default:
            if (prev_c >= ' ' && prev_c <= '~' && l->cursor < TEXT_SIZE)
                l->data[(l->cursor)++] = prev_c;
            break;
    }
    return a;
}

bool perform_action(board* b, pos* p, ACTION a) { // pas besoin d
    int xd = 0;
    int yd = 0;
    switch (a) {
        case LEFT:
            xd = -1; yd = 0; break;
        case RIGHT:
            xd = 1; yd = 0; break;
        case UP:
            xd = 0; yd = -1; break;
        case DOWN:
            xd = 0; yd = 1; break;  
        case QUIT:
            return true;
        default: break;
    }
    p->x += xd; p->y += yd;
    p->x = (p->x + b->w)%b->w;
    p->y = (p->y + b->h)%b->h;
    set_grid(b,p->x,p->y,1);
    return false;
}
