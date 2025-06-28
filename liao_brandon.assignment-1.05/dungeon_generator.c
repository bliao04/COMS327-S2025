

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

#include "dungeon_generator.h"
#include "heap.h"

/* constants for window dimensions */
#define LIST_WIN_ROWS 24
#define LIST_WIN_COLS 80
#define MONS_STR_LEN 40

/* function to compare two ints used as costs (0 if same, <0 if higher than key, >0 if lower than key) */
int compare_int(const void *key, const void *with) {
	return *(const int *) key - *(const int *) with;
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
	int px = dungeon->ss[dungeon->pcSS].p.x;
	int py = dungeon->ss[dungeon->pcSS].p.y;
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
	while((p = heap_remove_min(&h))) {
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
	int px = dungeon->ss[dungeon->pcSS].p.x;
	int py = dungeon->ss[dungeon->pcSS].p.y;
	tiles[py][px].cost = 0;
	tiles[py][px].v = TRUE;
	heap_insert(&h, &tiles[py][px]);

	heap_node_t	*p;

    //process the heap until it is empty
	while((p = heap_remove_min(&h))) {
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

    //read the PC position (2 bytes: x and y)
    fread(&(dungeon->pc.x), sizeof(uint8_t), 1, file);
    fread(&(dungeon->pc.y), sizeof(uint8_t), 1, file);

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
    dungeon->r = calloc(room_count, sizeof(struct Room));

    //read the room positions (4 bytes per room)
    for (int i = 0; i < room_count; i++) {
		uint8_t x, y, w, h;
		fread(&x, sizeof(uint8_t), 1, file);
		fread(&y, sizeof(uint8_t), 1, file);
		fread(&w, sizeof(uint8_t), 1, file);
		fread(&h, sizeof(uint8_t), 1, file);
		
		dungeon->r[i].top_left.x = x;
		dungeon->r[i].top_left.y = y;
		dungeon->r[i].w = w;
		dungeon->r[i].h = h;
		
		dungeon->r[i].bottom_right.x = x + w - 1;
		dungeon->r[i].bottom_right.y = y + h - 1;
    }

    //read the number of upward staircases (2 bytes)
    uint16_t upward_count;
    fread(&upward_count, sizeof(uint16_t), 1, file);
    upward_count = be16toh(upward_count);
    dungeon->nu = upward_count;
    dungeon->u_stairs = calloc(upward_count, sizeof(struct Position));

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
    dungeon->d_stairs = calloc(downward_count, sizeof(struct Position));

    //read the downward staircase positions (2 bytes per staircase)
    for (int i = 0; i < downward_count; i++) {
        fread(&dungeon->d_stairs[i].x, sizeof(uint8_t), 1, file);
        fread(&dungeon->d_stairs[i].y, sizeof(uint8_t), 1, file);
    }

	//when reading the dungeon matrix, load both 'c' and hardness.
	for (int i = 0; i < dungeon->h; i++) {
		for (int j = 0; j < dungeon->w; j++) {
			fread(&dungeon->d[i][j].c, sizeof(char), 1, file); // Load corridor marker
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
    char marker[12] = "RLG327-S2025";
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
    fwrite(&(dungeon->ss[dungeon->pcSS].p.x), sizeof(uint8_t), 1, file);
    fwrite(&(dungeon->ss[dungeon->pcSS].p.y), sizeof(uint8_t), 1, file);

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
		uint8_t x = dungeon->r[i].top_left.x;
		uint8_t y = dungeon->r[i].top_left.y;
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
    char **mons = malloc(alive_monsters * sizeof(char *));
    if (!mons) return;

    for (int i = 0; i < alive_monsters; i++) {
        mons[i] = malloc(MONS_STR_LEN);
        if (!mons[i]) {
            for (int j = 0; j < i; j++) free(mons[j]);
            free(mons);
            return;
        }
    }

    //populate alive monsters
    int valid_index = 0;
    for (int i = 1; i < dungeon->ns; i++) {
        if (!dungeon->ss[i].a) continue; //skip dead monsters
        
        int hd = dungeon->ss[0].p.y - dungeon->ss[i].p.y;
        int wd = dungeon->ss[0].p.x - dungeon->ss[i].p.x;
        
        snprintf(
            mons[valid_index], 
            MONS_STR_LEN, 
            "%c, %d %s and %d %s",
            dungeon->ss[i].c,
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
	k = getch(); //read keyboard input

	//quit game if 'Q' is pressed
	if(k == 'Q') {
		*run = FALSE;
		return;
	}

	switch(k) {
		//move left (h or 4)
		case 'h':
			H: ;
			dungeon->ss[dungeon->pcSS].to.x = dungeon->ss[dungeon->pcSS].p.x - 1;
			break;
		case '4':
			goto H;

		//move right (l or 6)
		case 'l':
			L: ;
			dungeon->ss[dungeon->pcSS].to.x = dungeon->ss[dungeon->pcSS].p.x + 1;
			break;
		case '6':
			goto L;

		//move up (k or 8)
		case 'k':
			K: ;
			dungeon->ss[dungeon->pcSS].to.y = dungeon->ss[dungeon->pcSS].p.y - 1;
			break;
		case '8':
			goto K;

		//move down (j or 2)
		case 'j':
			J: ;
			dungeon->ss[dungeon->pcSS].to.y = dungeon->ss[dungeon->pcSS].p.y + 1;
			break;
		case '2':
			goto J;

		//move upper left (y or 7)
		case 'y':
			Y: ;
			dungeon->ss[dungeon->pcSS].to.y = dungeon->ss[dungeon->pcSS].p.y - 1;
			dungeon->ss[dungeon->pcSS].to.x = dungeon->ss[dungeon->pcSS].p.x - 1;
			break;
		case '7':
			goto Y;

		//move upper right (u or 9)
		case 'u':
			U: ;
			dungeon->ss[dungeon->pcSS].to.y = dungeon->ss[dungeon->pcSS].p.y - 1;
			dungeon->ss[dungeon->pcSS].to.x = dungeon->ss[dungeon->pcSS].p.x + 1;
			break;
		case '9':
			goto U;

		//move lower right (n or 3)
		case 'n':
			N: ;
			dungeon->ss[dungeon->pcSS].to.y = dungeon->ss[dungeon->pcSS].p.y + 1;
			dungeon->ss[dungeon->pcSS].to.x = dungeon->ss[dungeon->pcSS].p.x + 1;
			break;
		case '3':
			goto N;

		//move lower left (b or 1)
		case 'b':
			B: ;
			dungeon->ss[dungeon->pcSS].to.y = dungeon->ss[dungeon->pcSS].p.y + 1;
			dungeon->ss[dungeon->pcSS].to.x = dungeon->ss[dungeon->pcSS].p.x - 1;
			break;
		case '1':
			goto B;

		//attempt to go up stairs (<) - triggers regen if on up staircase
		case '<':
			if(dungeon->ss[0].p.x == dungeon->u_stairs->x && dungeon->ss[0].p.y == dungeon->u_stairs->y)
				*regen = TRUE;
			break;
		
		//attempt to go down stairs (>) - triggers regen if on down staircase
		case '>':
			if(dungeon->ss[0].p.x == dungeon->d_stairs->x && dungeon->ss[0].p.y == dungeon->d_stairs->y)
				*regen = TRUE;
			break;
		
		//rest for a turn (5, space, or .)
		case '5':
			break;
		case ' ':
			break;
		case '.':
			break;
		
		//display monster list (m) and refresh dungeon
		case 'm':
			monster_list(dungeon);
			print_dungeon(dungeon, 0, 0);
			goto INPUT_LOOP;

		//invalid input - re-read
		default:
			goto INPUT_LOOP;
	}

	//check if target coordinates are within dungeon boundaries
	if (dungeon->ss[dungeon->pcSS].to.x < 0 || 
		dungeon->ss[dungeon->pcSS].to.x >= dungeon->w || 
		dungeon->ss[dungeon->pcSS].to.y < 0 || 
		dungeon->ss[dungeon->pcSS].to.y >= dungeon->h) {
		// Reset to current position if out of bounds
		dungeon->ss[dungeon->pcSS].to.x = dungeon->ss[dungeon->pcSS].p.x;
		dungeon->ss[dungeon->pcSS].to.y = dungeon->ss[dungeon->pcSS].p.y;
	}

    //check for valid movement, reset to if moving into a wall
	if(dungeon->d[dungeon->ss[dungeon->pcSS].to.y][dungeon->ss[dungeon->pcSS].to.x].hardness > 0) {
		dungeon->ss[dungeon->pcSS].to.x = dungeon->ss[dungeon->pcSS].p.x;
		dungeon->ss[dungeon->pcSS].to.y = dungeon->ss[dungeon->pcSS].p.y;
	} else {
		//update PC position if movement is valid
		dungeon->ss[dungeon->pcSS].p.x = dungeon->ss[dungeon->pcSS].to.x;
		dungeon->ss[dungeon->pcSS].p.y = dungeon->ss[dungeon->pcSS].to.y;
	}

	//progress PC's turn time based on speed
	dungeon->ss[0].t += (100 / dungeon->ss[0].s.s);

	//check for NPC kills, if pc moves into an NPC's cell, kill the NPC
    int sn = 0;
    int i;
	for(i = 1; i < dungeon->ns; i++) {
		if(i != sn) {
			if (dungeon->ss[i].a && dungeon->ss[i].p.x == dungeon->ss[dungeon->pcSS].p.x &&dungeon->ss[i].p.y == dungeon->ss[dungeon->pcSS].p.y) {
				dungeon->ss[i].a = FALSE;  
			}
        }
    }
}

/* function for main to help process commandline arguments */
void test_args(int argc, char ** argv, int this, int *s, int *l, int *nm) {
	    //check if current argument is "--save"
		if(strcmp(argv[this], "--save") == 0) {
			*s = TRUE;
		} 
    	//check if current argument is "--load"
		else if(strcmp(argv[this], "--load") == 0) {
			*l = TRUE;
		} 
		//check if current argument is "--nummon"
		else if(strcmp(argv[this], "--nummon") == 0) {
			*nm = atoi(argv[this+1]);
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
	char *path = calloc(strlen(env_path) + strlen("./rlg327/dungeon") + 1, sizeof(char)); //allocate memory for path
	strcpy(path, env_path); //start with home directory
	strcat(path, "/.rlg327/dungeon"); //append hidden directory	

	Bool regen = FALSE; //flag to indicate if the dungeon should be regenerated

	DUNGEN: ;

    //initialize dungeon structure with specified dimensions and room count
	struct Dungeon dungeon = init_dungeon(21, 80, 12);

	//check if a dungeon file already exists
	int file_exists = access(path, F_OK) == 0;

	//load dungeon from file if --load is set or --save is used with existing file
	if ((loading && regen==false)|| (saving && file_exists)) {
		read_dungeon(&dungeon, path);
	} 
	//generate new dungeon if loading isn't requested or file doesn't exist
	else {
		generate_dungeon(&dungeon);
		place_corridors(&dungeon);
		place_staircases(&dungeon);
	}

	//generate the pc sprite and add it to the dungeon
	struct Sprite pc = generate_sprite(&dungeon, '@', -1, -1, 1);
	add_sprite(&dungeon, pc);

	//generate and add monster sprites to the dungeon
	int i;
	for(i = 0; i < num_mon; i++) {
		struct Sprite m = generate_sprite(&dungeon,'m' , -1, -1, 1);
		m.sn = i;
		add_sprite(&dungeon, m);
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
			if(dungeon.ss[i].t < dungeon.ss[l].t) {
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
					if((dungeon.ss[i].p.x == dungeon.ss[sn].p.x) && (dungeon.ss[i].p.y == dungeon.ss[sn].p.y) && dungeon.ss[sn].a == TRUE) {
						dungeon.ss[i].a = FALSE;
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

		refresh(); //refresh the ncurses screen

	    //check if the game should end (PC is dead or game over condition is met), stops before new dungeon is printed
		if(dungeon.go == TRUE || dungeon.ss[dungeon.pcSS].a == FALSE) {
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