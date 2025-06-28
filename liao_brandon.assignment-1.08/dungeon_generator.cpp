#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include <endian.h>
#include <unistd.h> 
#include <sys/stat.h>
#include <sys/types.h>
#include <limits.h>
#include <ncurses.h>
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <algorithm>
#include <sstream>
using namespace std;

#include "dungeon_generator.h"
#include "heap.h"

/* constants for window dimensions */
#define LIST_WIN_ROWS 24
#define LIST_WIN_COLS 80
#define MONS_STR_LEN 40

/* function to compare two ints used as costs (0 if same, <0 if higher than key, >0 if lower than key) */
int compare_int(const void *key, const void *with) {
	return (const int) ((*(struct Tile_Node *) key).cost - (*(struct Tile_Node *) with).cost);
}

/* function to calculate the cost of a particular hardness */
int h_calc(int h) {
	int hc = 0;

	if(h == 0) {
		return 1; //cost is 1 for floor tiles (hardness 0)
	}
	if(h > 0 && h < 255) {
		return 1 + (h/85); //cost increases with hardness
	}

	return hc;
}

/* function to map a dungeon for tunneling monsters */
void map_dungeon_t(struct Dungeon * dungeon) {
	heap_t h;
	struct Tile_Node tiles[dungeon->h][dungeon->w]; //grid of nodes for pathfinding

	//initialize the heap with the comparison function
	heap_init(&h, compare_int, NULL);

	//relative coordinates for 8 possible directions (including diagonals)
	int xs[8] = {-1,0,1,1,1,0,-1,-1};
	int ys[8] = {-1,-1,-1,0,1,1,1,0};

	int i;
	int j;

	//initialize all tiles with default values
	for(i = 0; i < dungeon->h; i++) {
		for(j = 0; j < dungeon->w; j++) {
			tiles[i][j].y = i;
			tiles[i][j].x = j;
			tiles[i][j].cost = INT_MAX; //set initial cost to INT_MAX
			tiles[i][j].v = FALSE; //mark as unvisited
		}
	}

    //start from the player character's position
	int px = getSpriteAPX(dungeon->ss, dungeon->pcSS);
	int py = getSpriteAPY(dungeon->ss, dungeon->pcSS);
	tiles[py][px].cost = 0;
	tiles[py][px].v = TRUE;
	heap_insert(&h, &tiles[py][px]);

	//set all valid adjacent cells to cost 1
	int dirs[8][2] = {{-1, -1}, {0, -1}, {1, -1},
					  {-1,  0},          {1,  0},
					  {-1,  1}, {0,  1}, {1,  1}};

	for (int d = 0; d < 8; d++) {
		int nx = px + dirs[d][0];
		int ny = py + dirs[d][1];
		if (nx >= 0 && nx < dungeon->w && ny >= 0 && ny < dungeon->h) {
			if (dungeon->d[ny][nx].hardness < 255) { //only passable tiles
				tiles[ny][nx].cost = 1; //force adjacent tiles to 1
				tiles[ny][nx].v = TRUE;
				heap_insert(&h, &tiles[ny][nx]); //add to heap for processing
			}
		}
	}

	heap_node_t	*p;

	//process the heap until it is empty
	while((p = (heap_node_t*)heap_remove_min(&h))) {
		int hx = ((struct Tile_Node *) p)->x;
		int hy = ((struct Tile_Node *) p)->y;
		int tc = ((struct Tile_Node *) p)->cost;

		int i;
		//explore all 8 possible directions
		for(i = 0; i < 8; i++) {
			int x = hx + xs[i];
			int y = hy + ys[i];
			if(x >= 0 && x < dungeon->w && y >= 0 && y < dungeon->h) {
				int hard = dungeon->d[y][x].hardness;
				if(hard < 255) { //only passable tiles
						int trial_cost = tc + h_calc(hard); //calculate new cost
						if((tiles[y][x].cost > trial_cost && tiles[y][x].v == TRUE) || tiles[y][x].v == FALSE) {
						    //update cost and mark as visited if a better path is found
							tiles[y][x].cost = tc + h_calc(hard);
							tiles[y][x].v = TRUE;

							heap_insert(&h, &tiles[y][x]); //add to heap for further processing
						}
				}
			}
		}
	}

	//store the calculated costs in the dungeon's tunneling cost grid
	for(i = 0; i < dungeon->h; i++) {
		for(j = 0; j < dungeon->w; j++) {
			dungeon->cst[i][j] = tiles[i][j].cost;
		}
	}

    //clean up the heap
	heap_delete(&h);
}

/* function to map a dungeon for nontunneling monsters */
void map_dungeon_nont(struct Dungeon * dungeon) {
	heap_t h;
	struct Tile_Node tiles[dungeon->h][dungeon->w]; //grid of nodes for pathfinding

	//initialize the heap with the comparison function
	heap_init(&h, compare_int, NULL);

    //relative coordinates for 8 possible directions (including diagonals)
	int xs[8] = {-1,0,1,1,1,0,-1,-1};
	int ys[8] = {-1,-1,-1,0,1,1,1,0};

	int i;
	int j;

	//initialize all tiles with default values
	for(i = 0; i < dungeon->h; i++) {
		for(j = 0; j < dungeon->w; j++) {
			tiles[i][j].y = i;
			tiles[i][j].x = j;
			tiles[i][j].cost = INT_MAX; //set initial cost to INT_MAX
			tiles[i][j].v = FALSE; //mark as unvisited
		}
	}

	//start from the player character's position
	int px = getSpriteAPX(dungeon->ss, dungeon->pcSS);
	int py = getSpriteAPY(dungeon->ss, dungeon->pcSS);
	tiles[py][px].cost = 0;
	tiles[py][px].v = TRUE;
	heap_insert(&h, &tiles[py][px]);

	heap_node_t	*p;

    //process the heap until it is empty
	while((p = (heap_node_t*)heap_remove_min(&h))) {
		int hx = ((struct Tile_Node *) p)->x;
		int hy = ((struct Tile_Node *) p)->y;
		int tc = ((struct Tile_Node *) p)->cost;

        //explore all 8 possible directions
		int i;
		for(i = 0; i < 8; i++) {
			int x = hx + xs[i];
			int y = hy + ys[i];
			if(x >= 0 && x < dungeon->w && y >= 0 && y < dungeon->h) {
				int hard = dungeon->d[y][x].hardness;
				if(hard == 0) { //only passable tiles (hardness 0)
					int trial_cost = tc + 1; //cost increases by 1 for each step
					if((trial_cost < tiles[y][x].cost)) {
						//update cost and mark as visited if a better path is found
						tiles[y][x].cost = trial_cost;
						tiles[y][x].v = TRUE;

						heap_insert(&h, &tiles[y][x]); //add to heap for further processing
					}
				}
			}
		}
	}

    //store the calculated costs in the dungeon's non-tunneling cost grid
	for(i = 0; i < dungeon->h; i++) {
		for(j = 0; j < dungeon->w; j++) {
			dungeon->csnt[i][j] = tiles[i][j].cost;
		}
	}

    //clean up the heap
	heap_delete(&h);
}

/* function to read a dungeon from file */
void read_dungeon(struct Dungeon *dungeon, char *path) {
	FILE *file = fopen(path, "rb");
    if (file == NULL) {
        fprintf(stderr, "FILE ERROR: Could not open dungeon file at %s! read_dungeon()\n", path);
        exit(1);
    }

    //read the semantic file-type marker (12 bytes)
    char marker[13];
    fread(marker, 1, 12, file);
    marker[12] = '\0'; //null-terminate the string
    if (strcmp(marker, "RLG327-S2025") != 0) {
        fprintf(stderr, "FILE ERROR: Invalid file-type marker! read_dungeon()\n");
        fclose(file);
        exit(1);
    }

    //read the file version marker (4 bytes)
    uint32_t file_version;
    fread(&file_version, sizeof(uint32_t), 1, file);
    file_version = be32toh(file_version);
    dungeon->v = file_version;

    //read the size of the file (4 bytes)
    uint32_t size;
    fread(&size, sizeof(uint32_t), 1, file);
    size = be32toh(size);
    dungeon->s = size;

    //read the pc position (2 bytes: x and y)
	uint8_t pc_x = getSpriteAPX(dungeon->ss, dungeon->pcSS);
	uint8_t pc_y = getSpriteAPY(dungeon->ss, dungeon->pcSS);
	fread(&pc_x, sizeof(uint8_t), 1, file);
	fread(&pc_y, sizeof(uint8_t), 1, file);

    //read the dungeon matrix (1680 bytes: 21x80 grid)
    for (int i = 0; i < dungeon->h; i++) {
        for (int j = 0; j < dungeon->w; j++) {
            fread(&dungeon->d[i][j].hardness, sizeof(uint8_t), 1, file);
        }
    }

    //read the number of rooms (2 bytes)
    uint16_t room_count;
    fread(&room_count, sizeof(uint16_t), 1, file);
    room_count = be16toh(room_count);
    dungeon->num_rooms = room_count;
    dungeon->r = (Room*)calloc(room_count, sizeof(Room));

    //read the room positions (4 bytes per room)
    for (int i = 0; i < room_count; i++) {
		dungeon->r[i].top_left = (Position *)malloc(sizeof(Position));
		dungeon->r[i].bottom_right = (Position *)malloc(sizeof(Position));
	
		uint8_t x, y, w, h;
		fread(&x, sizeof(uint8_t), 1, file);
		fread(&y, sizeof(uint8_t), 1, file);
		fread(&w, sizeof(uint8_t), 1, file);
		fread(&h, sizeof(uint8_t), 1, file);
		
		setPosX(dungeon->r[i].top_left, x);
		setPosY(dungeon->r[i].top_left, y);
		dungeon->r[i].w = w;
		dungeon->r[i].h = h;
		
		setPosX(dungeon->r[i].bottom_right, x + w - 1);
		setPosY(dungeon->r[i].bottom_right, y + h - 1);
    }

    //read the number of upward staircases (2 bytes)
    uint16_t upward_count;
    fread(&upward_count, sizeof(uint16_t), 1, file);
    upward_count = be16toh(upward_count);
    dungeon->nu = upward_count;
    dungeon->u_stairs = (Position*)calloc(upward_count, sizeof(Position));

    //read the upward staircase positions (2 bytes per staircase)
    for (int i = 0; i < upward_count; i++) {
        fread(&dungeon->u_stairs[i].x, sizeof(uint8_t), 1, file);
        fread(&dungeon->u_stairs[i].y, sizeof(uint8_t), 1, file);
    }

    //read the number of downward staircases (2 bytes)
    uint16_t downward_count;
    fread(&downward_count, sizeof(uint16_t), 1, file);
    downward_count = be16toh(downward_count);
    dungeon->nd = downward_count;
    dungeon->d_stairs = (Position*)calloc(downward_count, sizeof(Position));

    //read the downward staircase positions (2 bytes per staircase)
    for (int i = 0; i < downward_count; i++) {
        fread(&dungeon->d_stairs[i].x, sizeof(uint8_t), 1, file);
        fread(&dungeon->d_stairs[i].y, sizeof(uint8_t), 1, file);
    }

	
	//when reading the dungeon matrix, load both 'c' and hardness.
	for (int i = 0; i < dungeon->h; i++) {
		for (int j = 0; j < dungeon->w; j++) {
			fread(&dungeon->d[i][j].c, sizeof(char), 1, file); //load corridor marker
			fread(&dungeon->d[i][j].hardness, sizeof(uint8_t), 1, file);
		}
	}


    fclose(file);
}

/* function to write the dungeon to file (~/.rlg327/dungeon) */
void write_dungeon(struct Dungeon *dungeon, char *path) {
	FILE *file = fopen(path, "wb");
    if (file == NULL) {
        fprintf(stderr, "FILE ERROR: Could not open dungeon file at %s! write_dungeon()\n", path);
        exit(1);
    }

    //write the semantic file-type marker (12 bytes)
    char marker[13] = "RLG327-S2025";
    fwrite(marker, sizeof(char), 12, file);

    //write the file version marker (4 bytes)
    uint32_t file_version = 0;
    file_version = htobe32(file_version);
    fwrite(&file_version, sizeof(uint32_t), 1, file);

    //write the size of the file (4 bytes)
    uint32_t size = 1708 + (4 * dungeon->num_rooms) + (2 * dungeon->nu) + (2 * dungeon->nd);
    size = htobe32(size);
    fwrite(&size, sizeof(uint32_t), 1, file);

    //write the PC position (2 bytes: x and y)
	uint8_t pc_x = getSpriteAPX(dungeon->ss, dungeon->pcSS);
	uint8_t pc_y = getSpriteAPY(dungeon->ss, dungeon->pcSS);
	fwrite(&pc_x, sizeof(uint8_t), 1, file);
	fwrite(&pc_y, sizeof(uint8_t), 1, file);

    //write the dungeon matrix (1680 bytes: 21x80 grid)
    for (int i = 0; i < dungeon->h; i++) {
        for (int j = 0; j < dungeon->w; j++) {
            fwrite(&dungeon->d[i][j].hardness, sizeof(uint8_t), 1, file);
        }
    }

    //write the number of rooms (2 bytes)
    uint16_t room_count = dungeon->num_rooms;
    room_count = htobe16(room_count);
    fwrite(&room_count, sizeof(uint16_t), 1, file);

    //write the room positions (4 bytes per room)
    for (int i = 0; i < dungeon->num_rooms; i++) {
		uint8_t x = getPosX(dungeon->r[i].top_left);
		uint8_t y = getPosY(dungeon->r[i].top_left);
		uint8_t w = dungeon->r[i].w;
		uint8_t h = dungeon->r[i].h;
		
		fwrite(&x, sizeof(uint8_t), 1, file);
		fwrite(&y, sizeof(uint8_t), 1, file);
		fwrite(&w, sizeof(uint8_t), 1, file);
		fwrite(&h, sizeof(uint8_t), 1, file);
    }

    //write the number of upward staircases (2 bytes)
    uint16_t upward_count = dungeon->nu;
    upward_count = htobe16(upward_count);
    fwrite(&upward_count, sizeof(uint16_t), 1, file);

    //write the upward staircase positions (2 bytes per staircase)
    for (int i = 0; i < dungeon->nu; i++) {
        fwrite(&dungeon->u_stairs[i].x, sizeof(uint8_t), 1, file);
        fwrite(&dungeon->u_stairs[i].y, sizeof(uint8_t), 1, file);
    }

    //write the number of downward staircases (2 bytes)
    uint16_t downward_count = dungeon->nd;
    downward_count = htobe16(downward_count);
    fwrite(&downward_count, sizeof(uint16_t), 1, file);

    //write the downward staircase positions (2 bytes per staircase)
    for (int i = 0; i < dungeon->nd; i++) {
        fwrite(&dungeon->d_stairs[i].x, sizeof(uint8_t), 1, file);
        fwrite(&dungeon->d_stairs[i].y, sizeof(uint8_t), 1, file);
    }

	
	//when writing the dungeon matrix, include the 'c' character (corridor marker).
	for (int i = 0; i < dungeon->h; i++) {
		for (int j = 0; j < dungeon->w; j++) {
			fwrite(&dungeon->d[i][j].c, sizeof(char), 1, file); //save corridor marker
			fwrite(&dungeon->d[i][j].hardness, sizeof(uint8_t), 1, file);
		}
	}

    fclose(file);
}

/* function to display scrollable list view of monsters (relative to pc position) */
void monster_list(struct Dungeon * dungeon) {
	//count alive monsters only
    int alive_monsters = 0;
    for (int i = 1; i < dungeon->ns; i++) {
        if (dungeon->ss[i].a) alive_monsters++;
    }

    if (alive_monsters <= 0) {
        clear();
        printw("No monsters detected.");
        refresh();
        getch();
        return;
    }

    //allocate for alive monsters
    char **mons = (char**)malloc(alive_monsters * sizeof(char *));
    if (!mons) return;

    for (int i = 0; i < alive_monsters; i++) {
        mons[i] = (char*)malloc(MONS_STR_LEN);
        if (!mons[i]) {
            for (int j = 0; j < i; j++) free(mons[j]);
            free(mons);
            return;
        }
    }

    //populate alive monsters
    int valid_index = 0;
    for (int i = 1; i < dungeon->ns; i++) {
        if (!getSpriteAA(dungeon->ss, i)) continue; //skip dead monsters
        
		int hd = getSpriteAPY(dungeon->ss, 0) - getSpriteAPY(dungeon->ss, i);
		int wd = getSpriteAPX(dungeon->ss, 0) - getSpriteAPX(dungeon->ss, i);

        snprintf(
            mons[valid_index], 
            MONS_STR_LEN, 
            "%c, %d %s and %d %s",
            getSpriteAC(dungeon->ss, i),
            abs(hd), 
            (hd > 0 ? "north" : "south"),
            abs(wd), 
            (wd > 0 ? "west" : "east")
        );
        valid_index++;
    }

    //window setup
    WINDOW *win = newwin(LIST_WIN_ROWS, LIST_WIN_COLS, 0, 0);
    keypad(win, TRUE);
    int top = 0;
    const int max_visible = LIST_WIN_ROWS - 2;
    int scroll_needed = alive_monsters > max_visible;
    int bot = scroll_needed ? max_visible : alive_monsters - 1;

    //UI loop
    bool exit = false;
    while(!exit) {
        werase(win);
        box(win, 0, 0);
        
        //draw monster list 
        for(int i = top, j = 1; i <= bot && i < alive_monsters && j < LIST_WIN_ROWS-1; i++, j++) {
            mvwprintw(win, j, 1, "%-*s", LIST_WIN_COLS-2, mons[i]);
        }

        //fixed status display
        if(scroll_needed) {
            mvwprintw(win, 0, 1, "Monsters %d-%d of %d (▲/▼ scroll)", 
                top + 1, bot + 1, alive_monsters);
        } else {
            mvwprintw(win, 0, 1, "Monsters (%d total)", alive_monsters);
        }

        wrefresh(win);

        //input handling
        switch(getch()) {
            case KEY_UP:
                if(top > 0) {
                    top--;
                    bot--;
                }
                break;
            case KEY_DOWN:
                if(bot < alive_monsters - 1) {
                    top++;
                    bot++;
                }
                break;
            case 27:
                exit = true;
                break;
            default:
                break;
        }
    }

    //cleanup 
    delwin(win);
    for(int i = 0; i < alive_monsters; i++) {
        free(mons[i]);
    }
    free(mons);
    print_dungeon(dungeon, 0, 0);
}

/* function to processes pc movements based on keyboard input, checks for valid movement and NPC kills */
void process_pc_input(struct Dungeon * dungeon, Bool * run, Bool * regen) {
	INPUT_LOOP: ;

    int32_t k;
    k = getch();

    if(k == 'Q') {
        *run = FALSE;
        return;
    }

    //teleport mode input handling first
    if(dungeon->teleport_mode) {
        switch(k) {
            //cursor movement
            case 'h': case '4': 
                dungeon->cursor_x = MAX(0, dungeon->cursor_x - 1);
                break;
            case 'l': case '6': 
                dungeon->cursor_x = MIN(dungeon->w-1, dungeon->cursor_x + 1);
                break;
            case 'k': case '8': 
                dungeon->cursor_y = MAX(0, dungeon->cursor_y - 1);
                break;
            case 'j': case '2': 
                dungeon->cursor_y = MIN(dungeon->h-1, dungeon->cursor_y + 1);
                break;
            case 'y': case '7':
                dungeon->cursor_x = MAX(0, dungeon->cursor_x - 1);
                dungeon->cursor_y = MAX(0, dungeon->cursor_y - 1);
                break;
            case 'u': case '9':
                dungeon->cursor_x = MIN(dungeon->w-1, dungeon->cursor_x + 1);
                dungeon->cursor_y = MAX(0, dungeon->cursor_y - 1);
                break;
            case 'n': case '3':
                dungeon->cursor_x = MIN(dungeon->w-1, dungeon->cursor_x + 1);
                dungeon->cursor_y = MIN(dungeon->h-1, dungeon->cursor_y + 1);
                break;
            case 'b': case '1':
                dungeon->cursor_x = MAX(0, dungeon->cursor_x - 1);
                dungeon->cursor_y = MIN(dungeon->h-1, dungeon->cursor_y + 1);
                break;
            //confirm teleport
            case 'g':
				//execute teleport
				setSpriteAPX(dungeon->ss, dungeon->pcSS, dungeon->cursor_x);
				setSpriteAPY(dungeon->ss, dungeon->pcSS, dungeon->cursor_y);
				//also update target position to match actual position
				setSpriteAToX(dungeon->ss, dungeon->pcSS, dungeon->cursor_x);
				setSpriteAToY(dungeon->ss, dungeon->pcSS, dungeon->cursor_y);
				dungeon->teleport_mode = 0;
				map_dungeon_t(dungeon);
				map_dungeon_nont(dungeon);
                break;
            //random teleport
            case 'r': {
				int new_x, new_y;
				do {
					new_x = rand() % (dungeon->w - 2) + 1;
					new_y = rand() % (dungeon->h - 2) + 1;
				} while (dungeon->d[new_y][new_x].hardness >= 255);
				
				//set PC's actual and target positions to the new location
				setSpriteAPX(dungeon->ss, dungeon->pcSS, new_x);
				setSpriteAPY(dungeon->ss, dungeon->pcSS, new_y);
				setSpriteAToX(dungeon->ss, dungeon->pcSS, new_x);
				setSpriteAToY(dungeon->ss, dungeon->pcSS, new_y);
				
				//exit teleport mode
				dungeon->teleport_mode = 0;
				
				//update pathfinding maps
				map_dungeon_t(dungeon);
				map_dungeon_nont(dungeon);
				
				//refresh the dungeon display
				print_dungeon(dungeon, 0, 0);
				break;
            }
            //cancel teleport
            case 27:
                dungeon->teleport_mode = 0;
                break;
            default:
                break;
        }
        print_dungeon(dungeon, 0, 0);
        return;
    }

    //normal mode input handling
    switch(k) {
        case 'h': case '4': 
            setSpriteAToX(dungeon->ss, dungeon->pcSS, getSpriteAPX(dungeon->ss, dungeon->pcSS) -1);
            break;
        case 'l': case '6':
            setSpriteAToX(dungeon->ss, dungeon->pcSS, getSpriteAPX(dungeon->ss, dungeon->pcSS) +1);
            break;
        case 'k': case '8': 
            setSpriteAToY(dungeon->ss, dungeon->pcSS, getSpriteAPY(dungeon->ss, dungeon->pcSS) -1);
            break;
        case 'j': case '2':
            setSpriteAToY(dungeon->ss, dungeon->pcSS, getSpriteAPY(dungeon->ss, dungeon->pcSS) +1);
            break;
        case 'y': case '7': 
            setSpriteAToX(dungeon->ss, dungeon->pcSS, getSpriteAPX(dungeon->ss, dungeon->pcSS) -1);
            setSpriteAToY(dungeon->ss, dungeon->pcSS, getSpriteAPY(dungeon->ss, dungeon->pcSS) -1);
            break;
        case 'u': case '9':
            setSpriteAToX(dungeon->ss, dungeon->pcSS, getSpriteAPX(dungeon->ss, dungeon->pcSS) +1);
            setSpriteAToY(dungeon->ss, dungeon->pcSS, getSpriteAPY(dungeon->ss, dungeon->pcSS) -1);
            break;
        case 'n': case '3': 
            setSpriteAToX(dungeon->ss, dungeon->pcSS, getSpriteAPX(dungeon->ss, dungeon->pcSS) +1);
            setSpriteAToY(dungeon->ss, dungeon->pcSS, getSpriteAPY(dungeon->ss, dungeon->pcSS) +1);
            break;
        case 'b': case '1': 
            setSpriteAToX(dungeon->ss, dungeon->pcSS, getSpriteAPX(dungeon->ss, dungeon->pcSS) -1);
            setSpriteAToY(dungeon->ss, dungeon->pcSS, getSpriteAPY(dungeon->ss, dungeon->pcSS) +1);
            break;
        case '<':
            if(getSpriteAPX(dungeon->ss, 0) == getPosX(dungeon->u_stairs) && 
               getSpriteAPY(dungeon->ss, 0) == getPosY(dungeon->u_stairs))
                *regen = TRUE;
            break;
        case '>':
            if(getSpriteAPX(dungeon->ss, 0) == getPosX(dungeon->d_stairs) && 
               getSpriteAPY(dungeon->ss, 0) == getPosY(dungeon->d_stairs))
                *regen = TRUE;
            break;
        case '5': case ' ': case '.': break;
        case 'm':
            monster_list(dungeon);
            print_dungeon(dungeon, 0, 0);
            goto INPUT_LOOP;
        case 'g':
            dungeon->teleport_mode = 1;
            dungeon->cursor_x = getSpriteAPX(dungeon->ss, dungeon->pcSS);
            dungeon->cursor_y = getSpriteAPY(dungeon->ss, dungeon->pcSS);
            print_dungeon(dungeon, 0, 0);
			refresh();
            goto INPUT_LOOP;
        case 'f':
            dungeon->fog_enabled = !dungeon->fog_enabled;
            print_dungeon(dungeon, 0, 0);
            goto INPUT_LOOP;
        default:
            goto INPUT_LOOP;
    }
	
	//check if target coordinates are within dungeon boundaries
	if (getSpriteAToX(dungeon->ss, dungeon->pcSS) < 0 || 
		getSpriteAToX(dungeon->ss, dungeon->pcSS) >= dungeon->w || 
		getSpriteAToY(dungeon->ss, dungeon->pcSS) < 0 || 
		getSpriteAToY(dungeon->ss, dungeon->pcSS) >= dungeon->h) {
		//reset to current position if out of bounds
		setSpriteAToX(dungeon->ss, dungeon->pcSS, getSpriteAPX(dungeon->ss, dungeon->pcSS));
		setSpriteAToY(dungeon->ss, dungeon->pcSS, getSpriteAPY(dungeon->ss, dungeon->pcSS));
	}

    //check for valid movement, reset to if moving into a wall
	if(dungeon->d[getSpriteAToY(dungeon->ss, dungeon->pcSS)][getSpriteAToX(dungeon->ss, dungeon->pcSS)].hardness > 0) {
		setSpriteAToX(dungeon->ss, dungeon->pcSS, getSpriteAPX(dungeon->ss, dungeon->pcSS));
		setSpriteAToY(dungeon->ss, dungeon->pcSS, getSpriteAPY(dungeon->ss, dungeon->pcSS));
	} else {
		//update PC position if movement is valid
		setSpriteAPX(dungeon->ss, dungeon->pcSS, getSpriteAToX(dungeon->ss, dungeon->pcSS));
		setSpriteAPY(dungeon->ss, dungeon->pcSS, getSpriteAToY(dungeon->ss, dungeon->pcSS));
	}

	//progress PC's turn time based on speed
	setSpriteAT(dungeon->ss, 0, getSpriteAT(dungeon->ss, 0) + (100 / getSpriteASS(dungeon->ss, 0)));

	//check for NPC kills, if pc moves into an NPC's cell, kill the NPC
    int sn = 0;
    int i;
	for(i = 1; i < dungeon->ns; i++) {
		if(i != sn) {
			if ((getSpriteAToX(dungeon->ss, i) == getSpriteAToX(dungeon->ss, sn)) && (getSpriteAToY(dungeon->ss, i) == getSpriteAToY(dungeon->ss, sn)) && getSpriteAA(dungeon->ss, sn) == TRUE) {
				setSpriteAA(dungeon->ss, i, FALSE);
			}
        }
    }
}

/* function that rolls dice based on a string input in format "b+n*d" */
int rolldie(std::string s)
{
	int b, n, d;

    //convert string to C-style string for sscanf
	char* str = new char [s.length()+1];
	std::strcpy(str, s.c_str());

	//parse the dice notation using sscanf
    //%*c skips the '+' and '*' characters in the input
	sscanf(str, "%d%*c%d%*c%d", &b, &n, &d);

    //create new Dice object with parsed values
	Dice* di = new Dice(b, n, d);
	delete[] str;

    //return the result of rolling the dice
	return di->roll();
}

/* function to create a Dice object from a string in dice notation format */
Dice* getdie(std::string s)
{
	int b, n, d;

    //convert string to C-style string for sscanf
	char* str = new char [s.length()+1];
	std::strcpy(str, s.c_str());

	//parse the dice notation using sscanf
    //%*c skips the '+' and '*' characters in the input
	sscanf(str, "%d%*c%d%*c%d", &b, &n, &d);

    //create and return new Dice object with parsed values
	Dice* di = new Dice(b, n, d);
    delete[] str;
	return di;
}

/* function to parse monster descriptions from file */
int parse_monsters(Dungeon * dungeon) {
	//get home directory path
	char * env_path = getenv("HOME");
    //allocate memory for full path
    char * path = (char*)calloc(strlen(env_path) + 50, sizeof(char));
    strcpy(path, env_path);
    strcat(path, "/.rlg327/monster_desc.txt");
    

    int dm = 0; //monster count
    bool first = true; //flag for first line check
    dungeon->mm = 100; //max monsters
    vector<SpriteTemp> mons; //temporary monster storage
    
    string line;
    ifstream md(path);
    if(md.is_open()) {
        SpriteTemp b_mo = {0}; //base monster template
        SpriteTemp mo; //current monster being parsed
        while(getline(md, line)) {            
            if(first) {
			    //check file header
                if(line != "RLG327 MONSTER DESCRIPTION 1") {
                    cout << "Invalid file head!" << endl;
                    free(path);
                    return -2;
                }
                first = false;
            } else {                                
                if(line == "BEGIN MONSTER") {
                    mo = b_mo; //reset to base template
                } else if(line.find("NAME") != string::npos) {
				    //parse name (max 77 chars)
                    mo.n = line.substr(5, 77);
                } else if(line.find("SYMB") != string::npos) {
					//parse symbol
                    size_t space = line.find(' ');
                    if (space != string::npos) {
                        string sym = line.substr(space + 1);
                        if (!sym.empty()) {
                            mo.c = sym[0];
                        }
                    }
                } else if(line.find("COLOR") != string::npos) {
                    //parse color
                    istringstream iss(line.substr(line.find("COLOR") + 5));
                    string color;
                    while (iss >> color) {
                        if (color == "RED") { mo.color = RED; break; }
                        else if (color == "GREEN") { mo.color = GREEN; break; }
                        else if (color == "BLUE") { mo.color = BLUE; break; }
                        else if (color == "CYAN") { mo.color = CYAN; break; }
                        else if (color == "YELLOW") { mo.color = YELLOW; break; }
                        else if (color == "MAGENTA") { mo.color = MAGENTA; break; }
                        else if (color == "WHITE") { mo.color = WHITE; break; }
                        else if (color == "BLACK") { mo.color = BLACK; break; }
                    }
                } else if(line.find("ABIL") != string::npos) {
                    //parse abilities
                    mo.s.in = (line.find("SMART") != string::npos);
                    mo.s.te = (line.find("TELE") != string::npos);
                    mo.s.tu = (line.find("TUNNEL") != string::npos);
                    mo.s.eb = (line.find("ERRATIC") != string::npos);
                    mo.s.pa = (line.find("PASS") != string::npos);
                    mo.s.uq = (line.find("UNIQ") != string::npos);
                    mo.s.ds = (line.find("DESTROY") != string::npos);
                    mo.s.boss = (line.find("BOSS") != string::npos);
                    mo.s.pu = (line.find("PICKUP") != string::npos);
                } else if(line.find("DAM") != string::npos) {
                    //parse damage 
                    mo.s.a = getdie(line.substr(4));
                } else if(line.find("DESC") != string::npos) {
                    //parse description (multi-line)
                    vector<string> desc;
                    while(getline(md, line)) {
                        if(line == ".") break;
                        desc.push_back(line.substr(0, 77));
                    }
                    mo.desc = new string[desc.size()];
                    mo.dl = desc.size();
                    for(size_t i = 0; i < desc.size(); i++) {
                        mo.desc[i] = desc[i];
                    }
                } else if(line.find("SPEED") != string::npos) { 
                    //parse speed
                    mo.s.s = getdie(line.substr(6));
                } else if(line.find("HP") != string::npos) { 
                    //parse hit points
                    mo.s.hp = getdie(line.substr(3));
                } else if(line.find("RRTY") != string::npos) {
                    //parse rarity
                    mo.s.rrty = stoi(line.substr(4));
                } else if(line == "END") {
                    //save completed monster
                    mons.push_back(mo);
                    dm++;
                }
            }
        }
    } else {
        free(path);
        return -1; //file open error
    }
    md.close();
    free(path);
    
    //store parsed monsters in dungeon
    dungeon->dm = dm;
    dungeon->md = new SpriteTemp[mons.size()];
    
	int i = 0;
	while(mons.size() > 0) {
		SpriteTemp tmp = mons.back();
		dungeon->md[i] = tmp;
		mons.pop_back();
		i++;
	}
    
    return 0;
}
/*
    function to print all monster descriptions stored in a dungeon
    void print_monster_descriptions(Dungeon * dungeon)
    {
        for (int i = 0; i < dungeon->dm; i++) {
            Monster &m = dungeon->md[i];
            
            //basic info
            cout << "Symbol: " << m.c << endl;
            cout << "Name: " << m.n << endl;
            
            //description
            cout << "Description:" << endl;
            for (int j = 0; j < m.dl; j++) {
                cout << m.desc[j] << endl;
            }
            
            //color
            cout << "Color: ";
            switch (m.color) {
                case RED:     cout << "RED"; break;
                case GREEN:   cout << "GREEN"; break;
                case BLUE:    cout << "BLUE"; break;
                case CYAN:    cout << "CYAN"; break;
                case YELLOW:  cout << "YELLOW"; break;
                case MAGENTA: cout << "MAGENTA"; break;
                case WHITE:   cout << "WHITE"; break;
                case BLACK:   cout << "BLACK"; break;
                default:      cout << "UNKNOWN"; break;
            }
            cout << endl;
            
            //stats
            printf("Speed: %d\n", m.s.s);
            printf("HP: %d\n", m.s.hp);
            cout << "Damage: " << m.s.a->string() << endl;
            printf("Rarity: %d\n", m.s.rrty);
            
            //abilities
            cout << "Abilities: ";
            if (m.s.in)    cout << "SMART ";
            if (m.s.te)    cout << "TELE ";
            if (m.s.tu)    cout << "TUNNEL ";
            if (m.s.eb)    cout << "ERRATIC ";
            if (m.s.pa)    cout << "PASS ";
            if (m.s.uq)  cout << "UNIQ ";
            if (m.s.ds) cout << "DESTROY ";
            if (m.s.boss)  cout << "BOSS ";
            if (m.s.pu) cout << "PICKUP ";
            cout << endl;
            
            //separator between monsters
            cout << "----------------------------------------" << endl;
        }
    }
*/

/* function to parse item descriptions from file */
int parse_items(Dungeon * dungeon) {
    //get home directory path
    char *env_path = getenv("HOME");
    if (!env_path) {
        std::cerr << "HOME environment variable not set!" << std::endl;
        return -1;
    }

    //build full path to item description file
    char *path = (char*)calloc(strlen(env_path) + 50, sizeof(char));
    strcpy(path, env_path);
    strcat(path, "/.rlg327/object_desc.txt");
    
    int di = 0; //item count
    bool first = true; //flag for first line check
    dungeon->mi = 100; //max items
    std::vector<ItemTemp> items; //temporary item storage
    
    std::string line;
    std::ifstream od(path);
    if(od.is_open()) {
        ItemTemp b_it{}; //base item template
        ItemTemp it; //current item being parsed
        while(getline(od, line)) {
            if(first) {
                //check file header
                if(line != "RLG327 OBJECT DESCRIPTION 1") {
                    std::cout << "Invalid file head!" << std::endl;
                    free(path);
                    return -2;
                }
                first = false;
            } else {
                if(line == "BEGIN OBJECT") {
                    it = b_it; //reset to base template
                }
                else if(line.rfind("NAME ", 0) == 0) {
                    //parse name
                    it.n = line.substr(5);
                }
                else if(line.rfind("TYPE ", 0) == 0) {
                    //parse item type
                    std::string type = line.substr(5);
                    if(type == "WEAPON") it.t = WEAPON;
                    else if(type == "OFFHAND") it.t = OFFHAND;
                    else if(type == "RANGED") it.t = RANGED;
                    else if(type == "ARMOR") it.t = ARMOR;
                    else if(type == "HELMET") it.t = HELMET;
                    else if(type == "CLOAK") it.t = CLOAK;
                    else if(type == "GLOVES") it.t = GLOVES;
                    else if(type == "BOOTS") it.t = BOOTS;
                    else if(type == "RING") it.t = RING;
                    else if(type == "AMULET") it.t = AMULET;
                    else if(type == "LIGHT") it.t = LIGHT;
                    else if(type == "SCROLL") it.t = SCROLL;
                    else if(type == "BOOK") it.t = BOOK;
                    else if(type == "FLASK") it.t = FLASK;
                    else if(type == "GOLD") it.t = GOLD;
                    else if(type == "AMMUNITION") it.t = AMMUNITION;
                    else if(type == "FOOD") it.t = FOOD;
                    else if(type == "WAND") it.t = WAND;
                    else if(type == "CONTAINER") it.t = CONTAINER;
                }
                else if(line.rfind("COLOR ", 0) == 0) {
                    //parse color
                    std::string color = line.substr(6);
                    if(color == "RED") it.c = RED;
                    else if(color == "GREEN") it.c = GREEN;
                    else if(color == "BLUE") it.c = BLUE;
                    else if(color == "CYAN") it.c = CYAN;
                    else if(color == "YELLOW") it.c = YELLOW;
                    else if(color == "MAGENTA") it.c = MAGENTA;
                    else if(color == "WHITE") it.c = WHITE;
                    else if(color == "BLACK") it.c = BLACK;
                }
                else if(line.rfind("WEIGHT ", 0) == 0) {
                    //parse weight
                    it.w = getdie(line.substr(7));
                }
                else if(line.rfind("HIT ", 0) == 0) {
                    //parse hit bonus
                    it.hib = getdie(line.substr(4));
                }
                else if(line.rfind("DAM ", 0) == 0) {
                    //parse damage
                    it.d = getdie(line.substr(4));
                }
                else if(line.rfind("ATTR ", 0) == 0) {
                    //parse special attribute
                    it.sa = getdie(line.substr(5));
                }
                else if(line.rfind("VAL ", 0) == 0) {
                    //parse value
                    it.v = getdie(line.substr(4));
                }
                else if(line.rfind("DODGE ", 0) == 0) {
                    //parse dodge bonus
                    it.dob = getdie(line.substr(6));
                }
                else if(line.rfind("DEF ", 0) == 0) {
                    //parse defense bonus
                    it.deb = getdie(line.substr(4));
                }
                else if(line.rfind("SPEED ", 0) == 0) {
                    //parse speed bonus
                    it.spb = getdie(line.substr(6));
                }
                else if(line.rfind("RRTY ", 0) == 0) {
                    //parse rarity
                    it.rrty = std::stoi(line.substr(5));
                }
                else if(line.rfind("ART ", 0) == 0) {
                    //parse artifact status
                    std::string art = line.substr(4);
                    it.art = (art == "TRUE");
                }
                else if(line.rfind("DESC", 0) == 0) {
                    //parse description (multi-line)
                    std::vector<std::string> desc;
                    while(getline(od, line)) {
                        if(line == ".") break;
                        desc.push_back(line);
                    }
                    it.desc = new std::string[desc.size()];
                    it.dl = static_cast<int>(desc.size());
                    for(size_t i = 0; i < desc.size(); i++) {
                        it.desc[i] = desc[i];
                    }
                }
                else if(line == "END") {
                    //save completed item
                    items.push_back(it);
                    di++;
                }
            }
        }
        od.close();
    } else {
        free(path);
        return -1; //file open error
    }
    free(path);
    
    //store parsed items in dungeon
    dungeon->di = di;
    dungeon->id = new ItemTemp[items.size()];

    int i = 0;
	while(items.size() > 0) {
		ItemTemp tmp = items.back();
		dungeon->id[i] = tmp;
		items.pop_back();
		i++;
	}
    
    return 0;
}
/*
    function to print all item descriptions stored in a dungeon 
    void print_item_descriptions(Dungeon * dungeon) {
        for (int i = 0; i < dungeon->di; i++) {
            Item &item = dungeon->id[i];
            
            //basic info
            std::cout << "Name: " << item.n << std::endl;
            
            //description
            std::cout << "Description:" << std::endl;
            for (int j = 0; j < item.dl; j++) {
                std::cout << item.desc[j] << std::endl;
            }
            
            //type
            std::cout << "Type: ";
            switch(item.t) {
                case WEAPON: std::cout << "WEAPON"; break;
                case OFFHAND: std::cout << "OFFHAND"; break;
                case RANGED: std::cout << "RANGED"; break;
                case ARMOR: std::cout << "ARMOR"; break;
                case HELMET: std::cout << "HELMET"; break;
                case CLOAK: std::cout << "CLOAK"; break;
                case GLOVES: std::cout << "GLOVES"; break;
                case BOOTS: std::cout << "BOOTS"; break;
                case RING: std::cout << "RING"; break;
                case AMULET: std::cout << "AMULET"; break;
                case LIGHT: std::cout << "LIGHT"; break;
                case SCROLL: std::cout << "SCROLL"; break;
                case BOOK: std::cout << "BOOK"; break;
                case FLASK: std::cout << "FLASK"; break;
                case GOLD: std::cout << "GOLD"; break;
                case AMMUNITION: std::cout << "AMMUNITION"; break;
                case FOOD: std::cout << "FOOD"; break;
                case WAND: std::cout << "WAND"; break;
                case CONTAINER: std::cout << "CONTAINER"; break;
                default: std::cout << "UNKNOWN"; break;
            }
            std::cout << std::endl;
            
            //color
            std::cout << "Color: ";
            switch(item.c) {
                case RED: std::cout << "RED"; break;
                case GREEN: std::cout << "GREEN"; break;
                case BLUE: std::cout << "BLUE"; break;
                case CYAN: std::cout << "CYAN"; break;
                case YELLOW: std::cout << "YELLOW"; break;
                case MAGENTA: std::cout << "MAGENTA"; break;
                case WHITE: std::cout << "WHITE"; break;
                case BLACK: std::cout << "BLACK"; break;
                default: std::cout << "UNKNOWN"; break;
            }
            std::cout << std::endl;
            
            //numeric attributes
            std::cout << "Weight: " << item.w << std::endl;
            std::cout << "Hit bonus: " << item.hib << std::endl;
            
            //damage (handle both dice and flat values)
            std::cout << "Damage: ";
            if (item.d) {
                std::cout << item.d->string();
            } else {
                std::cout << "0";
            }
            std::cout << std::endl;
            
            //defense and dodge
            std::cout << "Defense bonus: " << item.deb << std::endl;
            std::cout << "Dodge bonus: " << item.dob << std::endl;
            
            //speed (handle both dice and flat values)
            std::cout << "Speed bonus: " << item.spb;
            if (item.spd) {
                std::cout << "+" << item.spd->string();
            }
            std::cout << std::endl;
            
            //special attributes
            std::cout << "Special attribute: " << item.sa << std::endl;
            std::cout << "Value: " << item.v << std::endl;
            std::cout << "Rarity: " << item.rrty << std::endl;
            std::cout << "Artifact: " << (item.art ? "TRUE" : "FALSE") << std::endl;
            
            //separator between items
            std::cout << "----------------------------------------" << std::endl;
        }
    }
*/

/* function for main to help process commandline arguments */
void test_args(int argc, char ** argv, int ts, int *s, int *l, int *nm) {
	    //check if current argument is "--save"
		if(strcmp(argv[ts], "--save") == 0) {
			*s = TRUE;
		} 
    	//check if current argument is "--load"
		else if(strcmp(argv[ts], "--load") == 0) {
			*l = TRUE;
		} 
		//check if current argument is "--nummon"
		else if(strcmp(argv[ts], "--nummon") == 0) {
			*nm = atoi(argv[ts+1]);
		}
}

int main(int argc, char *argv[]) 
{
	//initialize commandline arguments
	int max_args = 7;
	int saving = FALSE;
	int loading = FALSE;
	int num_mon = 10;

	//process commandline arguments
	//--save --load both
	if(argc > 2 && argc <= max_args) {
		int i;
		for(i = 1; i < argc; i++) {
			test_args(argc, argv, i, &saving, &loading, &num_mon);
		}
	} 
	//only one argument
	else if(argc == 2) {
		test_args(argc, argv, 1, &saving, &loading, &num_mon);
	//more than 2 commandline arguments
	} else if(argc > max_args) {
		printf("Too many arguments!\n");
	} 

    //seed random number generator
	srand(time(NULL));

    //build path to dungeon file using HOME environment variable
	char *env_path = getenv("HOME"); //get user's home directory
	char *path = (char*)calloc(strlen(env_path) + strlen("./rlg327/dungeon") + 1, sizeof(char)); //allocate memory for path
	strcpy(path, env_path); //start with home directory
	strcat(path, "/.rlg327/dungeon"); //append hidden directory	

	Bool regen = FALSE; //flag to indicate if the dungeon should be regenerated

	DUNGEN: ;

    //initialize dungeon structure with specified dimensions and room count
	struct Dungeon dungeon = init_dungeon(21, 80, 12);

	parse_monsters(&dungeon);
	parse_items(&dungeon);
	dungeon.of = new ObjFac(dungeon.di, dungeon.id);
	dungeon.mf = new MonFac(dungeon.dm, dungeon.md);

	//check if a dungeon file already exists
	int file_exists = access(path, F_OK) == 0;

	//load dungeon from file if --load is set or --save is used with existing file
	if (regen == FALSE && (loading || (saving && file_exists))) {
		read_dungeon(&dungeon, path);
	} 
	//generate new dungeon if loading isn't requested or file doesn't exist
	else {
		generate_dungeon(&dungeon);
		place_corridors(&dungeon);
		place_staircases(&dungeon);
	}

	//generate the pc sprite and add it to the dungeon
	Sprite *pc = generate_sprite_fac(&dungeon, '@', -1, -1, 1);
	add_sprite(&dungeon, pc);

	//generate and add monster sprites to the dungeon
	int i;
	for(i = 0; i < num_mon; i++) {
		Sprite *m = generate_sprite_fac(&dungeon,'m' , -1, -1, 1);
		//m.sn = i;
		setSpriteSn(m, i);
		add_sprite(&dungeon, m);
	}

    //generate and add items to the dungeon
    for(i = 0; i < 10; i++) {
		Item it = dungeon.of->GetObj();
		placeitem(&dungeon, it);
	}

	//map nontunneling and tunneling 
	map_dungeon_nont(&dungeon);
	map_dungeon_t(&dungeon);
	
	//generate next moves for all monsters
	for(i = 1; i < dungeon.ns; i++) {
		generate_next_move(&dungeon, i);
	}
	
	if(regen == TRUE) {
		goto POST_REGEN;
	}

	//function pointer for printing the dungeon
	void (*printer)(struct Dungeon*, int, int);
	printer = &print_dungeon;

	//initialize ncurses screen
	initscr();
	raw();
	noecho();
	curs_set(0);
	set_escdelay(25);
	keypad(stdscr, TRUE);
    start_color();

	POST_REGEN: ;
	
	regen = FALSE; //reset regeneration flag

	print_dungeon(&dungeon, 0, 0);

	Bool first = TRUE; //flag for first iteration
	Bool run = TRUE; //flag to keep the game running

	//main game loop
	while(run == TRUE) {
		int l = 0; //index of the sprite with the lowest turn count

	    //find the sprite with the lowest turn count
		for(i = 0; i < dungeon.ns; i++) {
			if(getSpriteAT(dungeon.ss, i) < getSpriteAT(dungeon.ss, l)) {
				l = i;
			}
		}

    	//pc's turn
		if(l == dungeon.pcSS || first == TRUE) {
			process_pc_input(&dungeon, &run, &regen); //process player input
			if(regen == TRUE) {
				goto FREE;
			}
			
        	//update dungeon maps for non-tunneling and tunneling monsters
			map_dungeon_nont(&dungeon);
			map_dungeon_t(&dungeon);
			
        	//check for collisions between monsters and the player
			int sn = 0;
			for(i = 1; i < dungeon.ns; i++) {
				if(i != sn) {
					if((getSpriteAPX(dungeon.ss, i) == getSpriteAPX(dungeon.ss, sn)) && (getSpriteAPY(dungeon.ss, i) == getSpriteAPY(dungeon.ss, sn)) && getSpriteAA(dungeon.ss, sn) == TRUE) {
						setSpriteAA(dungeon.ss, i, FALSE);
					}
				}
			}
			
        	//print the updated dungeon	
			print_dungeon(&dungeon, 0, 0);

		//monster's turn	
		} else {
			parse_move(&dungeon, l); //process monster movement
			generate_next_move(&dungeon, l); //generate next move for the monster
		}

	    //check if the game should end (PC is dead or game over condition is met), stops before new dungeon is printed
		if(dungeon.go == TRUE || getSpriteAA(dungeon.ss, dungeon.pcSS) == FALSE) {
			break;
		}

	    //check if all monsters are defeated
		Bool any = check_monsters_alive(&dungeon);
		if(any == FALSE) {
			//printf("You won!\n");
			goto END;
		}

		first = FALSE; //reset first iteration flag
	}

	//print final dungeon state
	printer(&dungeon, 0, 0);
	//printf("Game Over! You died :( \n");

	//game over sequence 
	END: ;
	delwin(stdscr); //delete ncurses window
	endwin(); //end ncurses mode

	//save dungeon state if saving flag is set
	if(saving == TRUE) {
		write_dungeon(&dungeon, path);
	}

	FREE: ;

    //free dynamically allocated memory for dungeon 
    dungeon.current_uniques.clear();
    delete[] dungeon.items;
	delete[] dungeon.md;
	delete[] dungeon.id;
	delete dungeon.plyr;

	for(i = 0; i < dungeon.h; i++) {
		free(dungeon.d[i]);
	}
	free(dungeon.d);
	for(i = 0; i < dungeon.h; i++) {
		free(dungeon.p[i]);
	}
	free(dungeon.p);

	free(dungeon.r);
	free(dungeon.u_stairs);
	free(dungeon.d_stairs);
	free(dungeon.ss);
	for(i = 0; i < dungeon.h; i++) {
		free(dungeon.csnt[i]);
	}
	free(dungeon.csnt);
	for(i = 0; i < dungeon.h; i++) {
		free(dungeon.cst[i]);
	}
	free(dungeon.cst);

	//go back to dungeon generation if regeneration flag is true 
	if(regen == TRUE) {
		goto DUNGEN;
	}

	free(path);

	return 0;
}