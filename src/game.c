#include "game.h"


//  Game functions


/**
*   this function create a map of size passed by argument
*   the map needs to be freed by the user
*   @return a matrix (uint8**)
*   @param height of the map needed
*   @param width of the map needed
*   @author Leo
*/
Grid create_new_map(uint8_t height,uint8_t width){
    Grid grid;
    grid.height = height;
    grid.width = width;
    grid.map = my_malloc (sizeof(uint8_t*)*height);
    //grid allocation
    for(int i =0;i<height;i++){
        grid.map[i]=my_malloc (sizeof(uint8_t)*width);
    }

    for(int i = 0 ; i < height ; i++){
        for( int j = 0 ; j < width ; j++){
            if(i % 2 != 0 && j % 2 != 0)
                grid.map[i][j]=1;
            else
                grid.map[i][j]=2;
        }
    }

    //edges
    for(int i = 1; i< 3 ; i++){
        grid.map[i][0]=0;
        grid.map[0][i]=0;
        grid.map[height-i-1][0]=0;
        grid.map[0][width-i-1]=0;
        grid.map[i][width-1]=0;
        grid.map[height-1][i]=0;
        grid.map[height-1][width-i-1]=0;
        grid.map[height-i-1][width-1]=0;
    }
    // Placement aléatoire des zéros dans la grille
    int num_zeros = height * width / 2; // Nombre de zéros à placer (50% de la taille de la grille)
    for (int i = 0; i < num_zeros; i++) {
        int x, y;
        x = rand() % grid.width;
        y = rand() % grid.height;
        grid.map[y][x] = 0;
    }


    //initial position of players 
    grid.players_pos[0].y=0;
    grid.players_pos[0].x=0;

    grid.players_pos[1].y=height-1;
    grid.players_pos[1].x=0;

    grid.players_pos[2].y=height-1;
    grid.players_pos[2].x=width-1;

    grid.players_pos[3].y=0;
    grid.players_pos[3].x=width-1;

    //position in the map
    for (int i =0;i<4;i++){
        grid.map[grid.players_pos[i].y][grid.players_pos[i].x]=5+i;
    }

    //can be added a way to *randomly* fill the map 
    return grid;
}
/**
*   move the player in the new position on the grid
*   @param grid : grid struct
*   @param id : id of the player
*   @param act : action to perform
*   @author Leo 
*/
void move_player(Grid * grid, int id,uint16_t act, Bombs * bombs_array, Partial_grid * partial){
    print(LOG_INFO,"player:%d [%d:%d]\n",id,grid->players_pos[id].y,grid->players_pos[id].x);
    Player_pos * p = &grid->players_pos[id];
    //each proposition is next pos inside map && next pos tile = empty
    switch(act){
        case 0:
            if(p->y > 0 && grid->map[p->y-1][p->x]==0){
                print(LOG_INFO,"move right\n");
                if(grid->map[p->y][p->x]!=3){
                    grid->map[p->y][p->x]=0;
                }
                
                p->y -= 1;
                grid->map[p->y][p->x]=5+id;
                update_partial(partial, *p ,5+id);
            }
                
            break;
        
        case 1:
            if(p->x < grid->width-1 &&grid->map[p->y][p->x+1]==0){
                print(LOG_INFO,"move up\n");
                if(grid->map[p->y][p->x]!=3){
                    grid->map[p->y][p->x]=0;
                }
                p->x += 1;
                grid->map[p->y][p->x]=5+id;
                update_partial(partial, *p ,5+id);
            }
                
            break;
        
        case 2:
            if(p->x > 0 && grid->map[p->y][p->x-1]==0){
                print(LOG_INFO,"move up\n");
                if(grid->map[p->y][p->x]!=3){
                    grid->map[p->y][p->x]=0;
                }
                
                p->x -= 1;
                grid->map[p->y][p->x]=5+id;
                update_partial(partial, *p ,5+id);
            }
                
            break;
        
        case 3:
            if(p->y < grid->height-1 && grid->map[p->y+1][p->x]==0){
                print(LOG_INFO,"move down\n");
                if(grid->map[p->y][p->x]!=3){
                    grid->map[p->y][p->x]=0;
                }
                
                p->y += 1;
                grid->map[p->y][p->x]=5+id;
                update_partial(partial, *p ,5+id);
            }
                
            break;
        case 4:
            if(grid->map[p->y][p->x]!=3){//if not already a bomb
                print(LOG_INFO,"place bomb\n");
                grid->map[p->y][p->x]=3;
                Bomb_pos bomb_pos;
                bomb_pos.x = p->x;
                bomb_pos.y = p->y;
                bombs_array->bombs[bombs_array->index].position = bomb_pos;
                bombs_array->bombs[bombs_array->index].time = 8000; 
                bombs_array->index++;
                update_partial(partial, *p ,3);
            }
            
            break;
        default:
            break;
    }
    print(LOG_INFO,"player:%d [%d:%d]\n",id,grid->players_pos[id].y,grid->players_pos[id].x);
    
}
/**
*   bomb explosion logic, tile update is handled in a separet function 
*   @param bomb_pos : the position of the bomb exploding
*   @param grid : pointer to the game grid
*   @author Leo
*/
void bomb_explode(Bomb_pos bomb_pos, Grid * grid , Partial_grid * partial){
    if(bomb_pos.x>=grid->width || bomb_pos.y >= grid->height){
        fprintf(stderr,"wrong bomb placemnt\n");
        pthread_exit((void *) EXIT_FAILURE);
    }
    //check 2 tiles above
    for(int i = bomb_pos.y ; i < grid->height && i > bomb_pos.y-3 && i >= 0; i --){
        
        if(update_explosion_grid(grid , i , bomb_pos.x , partial)){
            break;
        }
        
    }
    //check 2 tiles right
    for(int i = bomb_pos.x ; i < grid->width && i < bomb_pos.x + 3; i ++){
        if(update_explosion_grid(grid , bomb_pos.y , i , partial)){
            break;
        }
    }
    //check 2 tiles below
    for(int i = bomb_pos.y ; i < grid->height && i < bomb_pos.y + 3; i ++){
        if(update_explosion_grid(grid , i , bomb_pos.x , partial)){
            break;
        }
    }
    //check 2 tiles left
    for(int i = bomb_pos.x ; i < grid->width && i > bomb_pos.x - 3 && i >= 0; i --){
        
        if(update_explosion_grid(grid , bomb_pos.y , i , partial)){
            break;
        }
        
    }

    //check diagonal
    for(int i = (bomb_pos.y -1 >= 0?-1:1) ; i+bomb_pos.y >= 0 && bomb_pos.y+i < grid->height && i<=1; i+=2 ){
        for( int j = (bomb_pos.x-1>=0?-1:1) ; j+bomb_pos.x>=0 && bomb_pos.x+j < grid->width && j <=1 ; j+=2){
            update_explosion_grid(grid,bomb_pos.y+i,bomb_pos.x+j , partial);
        }
    }
    

}
/**
*   update the grid tile for the explosion
*   @return 1 if the bomb is trying to explod a solid wall
*   @param grid : pointer to the game grid
*   @param y : y coordinate of the tile
*   @param x : x coordinate of the tile
*   @author Leo
*/
int update_explosion_grid(Grid * grid , int y , int x , Partial_grid * partial){
    Player_pos pos;
    pos.x = x;
    pos.y = y;
    switch(grid->map[y][x]){
        case 0:
            break;
        case 1:
            return 1;
            break;
        case 2:

            grid->map[y][x]=0;
            
            update_partial(partial , pos , 0);
            break;
        case 3:
            grid->map[y][x]=0;
            update_partial(partial , pos , 0);
            break;
        case 4:
            grid->map[y][x]=4;
            break;
        default:
            //int id = grid->map[i][j]-5;
            grid->map[y][x]=0;
            update_partial(partial , pos , 0);
            //kill player with id = id
            break;
    }
    return 0;
}

/**
*   free the map passed by argument
*   @param map 
*   @param width of the map
*   @param length of the map
*   @author Leo
*/
void free_map(Grid grid ){
    for(int i =0;i<grid.height;i++){
        uint8_t *p=grid.map[i];
        free(p);
    }
    free(grid.map);
}

/**
*   inizialize the bomb array
*   @return array of bombs
*   @param grid 
*   @author Leo
*/
Bombs bombs_init(Grid grid){
    int size = grid.width*grid.height;
    Bombs bombs;
    bombs.bombs = my_malloc(sizeof(Bomb)*size);
    bombs.index =0;
    return bombs;
}

/**
*   update bombs counter and explode bomb in case timer runs out
*   @param grid : pointer to game grid
*   @param bombs : pointer to bombs array
*   @author Leo
*/
void bomb_update(Bombs * bombs, Grid * grid, Partial_grid * partial){
    for(int i = 0 ; i < bombs->index ; i++){
        if(bombs->bombs[i].time==0){
            bomb_explode(bombs->bombs[i].position, grid, partial);
            memmove(bombs->bombs, bombs->bombs+1, sizeof(bombs->bombs) - sizeof(*bombs->bombs)); //array shift right
            bombs->index--;
            i--;
        }
        else{
            bombs->bombs[i].time-=1;
            
        }
    }
}

/**
*   update the list of the modified cases
*   @param partial : pointer to Partial_grid structure
*   @param pos : player pos strucure of the tile to send
*   @param act : action to send back
*   @author Leo
*/
void update_partial(Partial_grid * partial, Player_pos pos, uint8_t act){
    partial->nb+=1;
    partial->cases = realloc(partial->cases , sizeof(Case)*partial->nb);
    if(partial->cases==NULL){
        perror("memory reallocation failed in update_partial");
        pthread_exit((void*)EXIT_FAILURE);
    }
    Case c;
    c.ligne = pos.x;
    c.colonne = pos.y;
    c.contenu = act;
    partial->cases[partial->nb-1]=c;
}

/**
*   debug function to print the grid
*   @author leo
*/
void print_grid(Grid grid){
    if(DEBUG){
        for(int i =0;i<grid.height;i++){
            for(int j =0;j<grid.width;j++){
                printf("%d  ",grid.map[i][j]);
            }
            printf("\n");
        }
        printf("\n\n");
    }

    
}