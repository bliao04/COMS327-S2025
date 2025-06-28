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
#define	TRUE 1
#define	FALSE 0

struct Tile
{
	int	hardness; //hardness
	char c; //tile character (., #, <, >)
	int	path; //1 if path, 0 if not a path (ie corridors)
};

struct Path
{
	int prev; //previous room in the path 
	int next; //next room in the path (ie room the path leads to)
};

struct Position
{
	int	x; //x coordinate
	int	y; //y coordinate
};

 struct Room
 {
	struct Position	top_left; //top left coordinate of the room
	struct Position	bottom_right; //bottom right coordinate of the room 
    struct Position	ctr; //center point of the room
	int	w; //room width 
	int	h; //room height
	int	id; //room ID 
	int	processed; //mark 1 if processed, 0 if not processed (ie has corridors)
	int connected; //true or false for if a room is connected or not
};

struct Dungeon{
	struct Tile **d; //dungeon grid
	struct Tile **p; //print grid
    struct Room *r;	//rooms grid
    int	w; //dungeon width 
	int	h; //dungeon height
	int	num_rooms; //number of rooms in dungeon 
	int	max_rooms;	//max number of rooms in dungeon
	int v; //file version
	int s; //file size
	struct Position pc; //player character position
	int nu; //number of upwards staircases
	struct Position *u_stairs; //position of upward staircase (only 1 for now)
	int nd; //number of downwards staircases 
	struct Position *d_stairs; //position of downward staircase (only 1 for now)
};

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
    fread(&dungeon->pc.x, sizeof(uint8_t), 1, file);
    fread(&dungeon->pc.y, sizeof(uint8_t), 1, file);

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

	//add corridors to the dungeon buffer
	for(int i = 0; i < dungeon->h; i++) {
		for(int j = 0; j < dungeon->w; j++) {
			if(dungeon->d[i][j].c != '.' && dungeon->d[i][j].hardness == 0) {
				dungeon->d[i][j].c = '#';
				dungeon->d[i][j].path = 1;
			}
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
    fwrite(&dungeon->pc.x, sizeof(uint8_t), 1, file);
    fwrite(&dungeon->pc.y, sizeof(uint8_t), 1, file);

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

    fclose(file);
}

/* function to initialize the dungeon structure */
struct Dungeon init_dungeon(int h, int w, int mr) {
    //initialize the new dungeon
	struct Dungeon dungeon;
	dungeon.h = h;
	dungeon.w = w;
	dungeon.max_rooms = mr;
	dungeon.num_rooms = 0;
	dungeon.pc.x = 0;
	dungeon.pc.y = 0;
	dungeon.nu = 0;
	dungeon.u_stairs = NULL;
	dungeon.nd = 0;
	dungeon.d_stairs = NULL;

    //allocate memory for the dungeon grid (2D array of Tile pointers)
	dungeon.d = calloc(dungeon.h, sizeof(struct Tile *));
	
	int i;
    //allocate memory for each row in the dungeon grid
	for(i = 0; i < dungeon.h; i++) {
		dungeon.d[i] = calloc(dungeon.w, sizeof(struct Tile));
	}

    //allocate memory for the path grid (2D array of Tile pointers)
	dungeon.p = calloc(dungeon.h, sizeof(struct Tile *));

    //allocate memory for each row in the path grid
	for(i = 0; i < dungeon.h; i++) {
		dungeon.p[i] = calloc(dungeon.w, sizeof(struct Tile));
	}	

    //allocate memory for the array of rooms (maximum number of rooms)
	dungeon.r = calloc(dungeon.max_rooms, sizeof(struct Room));

    //return the initialized dungeon
	return dungeon;
}

/* function to place rooms within a given dungeon */
int place_rooms(struct Dungeon *dungeon) {
    //generate random coordinates (x, y) for the top-left corner of the room
	int x = (rand() % (dungeon->w-1)) +1;
	int y = (rand() % (dungeon->h-1)) +1;
	struct Room room;

    //set the top-left corner of the room
	room.top_left.x = x; 
	room.top_left.y = y;	
		
    //generate random width and height for the room
	int room_width = (rand() % 4) + 4; //width between 4 and 7
	int room_height = (rand() % 4) + 3; //height between 3 and 6

    //set the room's height and width
	room.h = room_height;
	room.w = room_width;
	
    //calculate the bottom-right corner of the room
	room.bottom_right.x = x + room.w-1;
	room.bottom_right.y = y + room.h-1;

	int i;
	int j;
	int placed = -1; //default to "not placed"
	int passed = 0; //valid tile counter
    //check if the proposed room area is free (no overlapping with existing rooms)
	for(i = y; i < dungeon->h-1 && i < y+room_height; i++) {
		for(j = x; j < dungeon->w-1 && j < x+room_width; j++) {
			if(dungeon->p[i][j].c != '.') {
				passed++; //count valid tiles
			}
		}
	}

    //if the room overlaps with existing rooms, return "not placed"
	if(passed < room_width*room_height) {
		return placed; 
	}
	
    //check if the room is within the dungeon boundaries
	if(room.bottom_right.x >= dungeon->w || room.bottom_right.y >= dungeon->h) {
		return placed;
	}
	
    //check surrounding areas to ensure no adjacent rooms
    //top row check
	for(i = room.top_left.x-1; i < room.bottom_right.x+2 && room.top_left.x-1 >= 0 && room.bottom_right.x+1 < dungeon->w && room.top_left.y-1 >= 0; i++) {
		if((dungeon->p[room.top_left.y-1][i]).c == '.') {
			return placed; //adjacent room found, return "not placed"
		}
	}

    //bottom row check
	for(i = room.top_left.x-1; i < room.bottom_right.x+2 && room.top_left.x-1 >= 0 && room.bottom_right.x+1 < dungeon->w && room.bottom_right.y+1 < dungeon->h; i++) {
		if((dungeon->p[room.bottom_right.y+1][i]).c == '.') {
			return placed; //adjacent room found, return "not placed"
		}
	}

    //left side check 
	for(i = room.top_left.y; i < room.bottom_right.y+1 && room.bottom_right.y+1 < dungeon->h && room.top_left.x-1 >= 0; i++) {
		if((dungeon->p[i][room.top_left.x-1]).c == '.') {
			return placed; //adjacent room found, return "not placed"
		}
	}

    //right side check 
	for(i = room.top_left.y; i < room.bottom_right.y+1 && room.bottom_right.y+1 < dungeon->h && room.bottom_right.x+1 < dungeon->w; i++) {
		if((dungeon->p[i][room.bottom_right.x+1]).c == '.') {
			return placed; //adjacent room found, return "not placed"
		}
	}

    //mark the room area in the dungeon path grid
	for(i = y; i < y+room_height; i++) {
		for(j = x; j < x+room_width; j++) {
			dungeon->p[i][j].c = '.';
		}
	}

    //add the room to the dungeon's room array if there is space
	if(dungeon->num_rooms < dungeon->max_rooms) {
		dungeon->num_rooms++;
		room.id = dungeon->num_rooms-1; //assign a unique ID to the room (pos in array)
		room.ctr.x = (room.w)/2 + room.top_left.x; //calculate center x
		room.ctr.y = (room.h)/2 + room.top_left.y; //calculate center y 
		dungeon->r[dungeon->num_rooms-1] = room; //add the room to the array
	} else {
		return -1; //no more rooms can be added
	}

    //return room placement as successful 
	placed = 0;
	return placed;
}

/* function for place_corridors() that checks if all rooms are connected */
int all_rooms_connected(int *cnxns, struct Dungeon *dungeon) {
	int i;

	//iterate through the connections array
	for(i = 0; i < dungeon->num_rooms; i++) {  
        //if any room is not connected (marked by 0), return FALSE
		if(cnxns[i] != 1 || dungeon->r[i].connected != TRUE) {
			return FALSE;
		}
	}
	
    //if all rooms are connected (marked by 1), return TRUE
	return TRUE;
}

/* function to generate and mark corridors */
void place_corridors(struct Dungeon * dungeon) {
    //array used to track whether each room is connected to the corridor network
	int is_connected[dungeon->num_rooms];
	memset(is_connected, 0, dungeon->num_rooms * sizeof(int));

    //array to store the distances from the current room to all other rooms
	double distances[dungeon->num_rooms];
	memset(distances, 0.0, dungeon->num_rooms * sizeof(double));

    // max number of paths that can be generated
	int max_paths = dungeon->num_rooms * 3;

    //array to store the paths (corridors between rooms)
	struct Path paths[max_paths]; 

    //counter for the number of paths generated
	int path_cnt = 0;

    //starting position (index of the current room)
	int	room_pos = 0;


    //initialize the distances array with -1 (ie uncalculated distances)
	int i;
	for(i = 0; i < dungeon->num_rooms; i++) {
		distances[i] = -1; 
	}
	distances[0] = 0; //distance to the starting room (room 0) is 0

	//make sure no rooms are connected before generating
	for(i = 0; i < dungeon->num_rooms; i++) {
		dungeon->r[i].connected = FALSE;
	}

    //continue generating paths until all rooms are connected or the maximum number of paths is reached
	while(all_rooms_connected(is_connected, dungeon) == FALSE && path_cnt < max_paths) {
		int i;
		double d;
		struct Path path;

        //calculate the Euclidean distance from the current room to all other rooms
		for(i = 0; i < dungeon->num_rooms; i++) {
			d = sqrt(pow(dungeon->r[i].ctr.x - dungeon->r[room_pos].ctr.x, 2) + pow(dungeon->r[i].ctr.y - dungeon->r[room_pos].ctr.y, 2));
			distances[i] = d;
		}

        //find the nearest unconnected room
		int next = -1; //index of the next room to connect to 
		for(i = 0; i < dungeon->num_rooms; i++) {
			if(is_connected[i] != 1 && next == -1 && room_pos != i) {
				next = i; //first unconnected room found
			} else if(is_connected[i] != 1 && distances[i] < distances[next] && room_pos != i) {
				next = i; //closer unconnected room found
			}
		}
		
        //if a valid next room is found, create a path to it
		if(next != -1) {
			dungeon->r[room_pos].connected = TRUE;
			dungeon->r[next].connected = TRUE;
			is_connected[room_pos] = 1; //mark the current room as connected
			path.prev = room_pos; //set the previous room in the path
			path.next = next; //set the next room in the path
			paths[path_cnt] = path; //add the path to the paths array
			room_pos = next; //move to the next room
			path_cnt++; //increment the path counter
		} else {
			break; //no valid next room found, exit the loop
		}
	}

    //mark the corridors on the dungeon grid
	for(i = 0; i < path_cnt; i++) {
		int x = dungeon->r[paths[i].prev].ctr.x; //starting x-coordinate of the path
		int y = dungeon->r[paths[i].prev].ctr.y; //starting y-coordinate of the path
				
        //continue until the path reaches the destination room 
		while(x != dungeon->r[paths[i].next].ctr.x || y != dungeon->r[paths[i].next].ctr.y) {
			int dirx = 0; // -1 for left, 1 for right
			int diry = 0; // -1 for down, 1 for up 
			
            //determine the x-direction 
			if(x < dungeon->r[paths[i].next].ctr.x) {
				dirx = 1; //move right
			} else if(x > dungeon->r[paths[i].next].ctr.x) {
				dirx = -1; //move left
			}

            //determine the y-direction
			if(y < dungeon->r[paths[i].next].ctr.y) {
				diry = 1; //move down
			} else if(y > dungeon->r[paths[i].next].ctr.y) {
				diry = -1; //move up 
			}

            //mark the current tile as part of the corridor
			dungeon->d[y][x].path = 1;
			if(dungeon->d[y][x].c != '.') {
				dungeon->d[y][x].c = '#';
				dungeon->d[y][x].hardness = 0;
			}

            //move to the next tile based on the direction
			if(dirx == -1) {
				x--; //move left
			} else if(dirx == 1) {
				x++; //move right
			} else if(diry == -1) {
				y--; //move up
			} else if(diry == 1) {
				y++; //move down
			}
		}
	}
}

/* function to place staircases in the dungeon */
void place_staircases(struct Dungeon *dungeon) {
	//allocate memory for upward and downward staircases
    dungeon->nu = 1; //1 upward staircase
    dungeon->u_stairs = calloc(dungeon->nu, sizeof(struct Position));

    dungeon->nd = 1; //1 downward staircase
    dungeon->d_stairs = calloc(dungeon->nd, sizeof(struct Position));

    //place upward staircase
    int up_placed = 0;
    while (!up_placed) {
        int x = (rand() % (dungeon->w - 2)) + 1;
        int y = (rand() % (dungeon->h - 2)) + 1;

        if (dungeon->d[y][x].c == '.' || dungeon->d[y][x].path == 1) {
            dungeon->d[y][x].c = '<'; //mark the tile with the up staircase symbol
            dungeon->u_stairs[0].x = x; //store the position
            dungeon->u_stairs[0].y = y;
            up_placed = 1;
        }
    }

    //place downward staircase
    int down_placed = 0;
    while (!down_placed) {
        int x = (rand() % (dungeon->w - 2)) + 1;
        int y = (rand() % (dungeon->h - 2)) + 1;

        if (dungeon->d[y][x].c == '.' || dungeon->d[y][x].path == 1) {
            dungeon->d[y][x].c = '>'; //mark the tile with the down staircase symbol
            dungeon->d_stairs[0].x = x; //store the position
            dungeon->d_stairs[0].y = y;
            down_placed = 1;
        }
    }
}

/* function to generate a blank dungeon */
void generate_dungeon(struct Dungeon * dungeon) {
	int i, j;

    //initialize the dungeon grid with empty spaces
	for(i = 0; i < dungeon->h; i++) {
		for(j = 0; j < dungeon->w; j++) {
			(dungeon->d[i][j]).c = ' ';	
			int h = (rand() % 254) + 1;
			(dungeon->d[i][j]).hardness = h;
		}
	}

    //set the hardness of the top and bottom borders to be immutible 
	for(i = 0; i < dungeon->w; i++) {
		(dungeon->d[0][i]).hardness = 255;
	}
	for(i = 0; i < dungeon->w; i++) {
		(dungeon->d[dungeon->h-1][i]).hardness = 255;
	}

    //set the hardness of the left and right borders to be immutible 
	for(i = 0; i < dungeon->h; i++) {
		(dungeon->d[i][0]).hardness = 255;
	}
	for(i = 0; i < dungeon->h; i++) {
		(dungeon->d[i][dungeon->w-1]).hardness = 255;
	}

    //copy the dungeon grid to the path grid (used for corridor generation)
	for(i = 0; i < dungeon->h; i++) {
		for(j = 0; j < dungeon->w; j++) {
			dungeon->p[i][j] = dungeon->d[i][j];
		}
	}

    //attempt to place rooms in the dungeon
	int failed_attempts = 0; //counter for failed room placement attempts
	int placement_result = 0; //stores the result of the place_rooms() function
	for(i = 0; dungeon->num_rooms < dungeon->max_rooms && failed_attempts < 2000; i++) {
		placement_result = place_rooms(dungeon); //try to place a room
		if(placement_result < 0) {
			failed_attempts++; //increment the counter if room placement fails
		}
	}

	//place the PC in the center of the first room
    if (dungeon->num_rooms > 0) {
        dungeon->pc.x = dungeon->r[0].ctr.x;
        dungeon->pc.y = dungeon->r[0].ctr.y;
        dungeon->d[dungeon->pc.y][dungeon->pc.x].c = '@'; //mark the PC position
    }
}

/* function to print the dungeon */
void print_dungeon(struct Dungeon *dungeon) {
	int i;
	int j;
	int h;
	
    //initialize the print grid by setting all tiles to walls using the ' ' symbol
	for(i = 0; i < dungeon->h; i++) {
		for(j = 0; j < dungeon->w; j++) {
			dungeon->p[i][j].c = ' ';
		}
	}

    //mark corridors on the print grid using the '#' symbol
	for(i = 0; i < dungeon->h; i++) {
		for(j = 0; j < dungeon->w; j++) {
			if(dungeon->d[i][j].path == 1 || dungeon->d[i][j].c == '#') {
				dungeon->p[i][j].c = '#';
			}
		}
	}

    //mark rooms on the print grid using the '.' symbol
	for(h = 0; h < dungeon->num_rooms; h++) {
		for(i = dungeon->r[h].top_left.y; i < dungeon->r[h].bottom_right.y+1; i++) {
			for(j = dungeon->r[h].top_left.x; j < dungeon->r[h].bottom_right.x+1; j++) {
				dungeon->p[i][j].c = '.';
			}
		}
	}

    //mark staircases
    for (i = 0; i < dungeon->nu; i++) {
        dungeon->p[dungeon->u_stairs[i].y][dungeon->u_stairs[i].x].c = '<';
    }
    for (i = 0; i < dungeon->nd; i++) {
        dungeon->p[dungeon->d_stairs[i].y][dungeon->d_stairs[i].x].c = '>';
    }

	//mark player character position (center of room0 for now cannot be updated)
	dungeon->p[dungeon->pc.y][dungeon->pc.x].c = '@';

    //print the final dungeon layout to the console
	for(i = 0; i < dungeon->h; i++) {
		int j;
		for(j = 0; j < dungeon->w; j++) {
			printf("%c", (dungeon->p[i][j]).c);
		}
		printf("\n");	
	}
}

/* function for main to help process commandline arguments */
void test_args(int argc, char ** argv, int this, int *s, int *l) {
	    //check if current argument is "--save"
		if(strcmp(argv[this], "--save") == 0) {
			*s = TRUE;
		} 
    	//check if current argument is "--load"
		else if(strcmp(argv[this], "--load") == 0) {
			*l = TRUE;
		} 
}

int main(int argc, char *argv[]) 
{
	//initialize commandline arguments
	int max_args = 5;
	int saving = FALSE;
	int loading = FALSE;

	//process commandline arguments
	//--save --load both
	if(argc > 2 && argc <= max_args) {
		int i;
		for(i = 1; i < argc; i++) {
			test_args(argc, argv, i, &saving, &loading);
		}
	} 
	//--save or --load alone
	else if(argc == 2) {
		test_args(argc, argv, 1, &saving, &loading);
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

    //initialize dungeon structure with specified dimensions and room count
	struct Dungeon dungeon = init_dungeon(21, 80, 12);

    //check if a dungeon file already exists
    int file_exists = access(path, F_OK) == 0;

    //load dungeon from file if --load is set or --save is used with existing file
	if (loading || (saving && file_exists)) {
		read_dungeon(&dungeon, path);
	} 
	//generate new dungeon if loading isn't requested or file doesn't exist
	else {
		generate_dungeon(&dungeon);
		place_corridors(&dungeon);
		place_staircases(&dungeon);
	}

    //save dungeon to file if --save is set
	if (saving == TRUE) {
		write_dungeon(&dungeon, path);
	}

    //output the dungeon to the console
	print_dungeon(&dungeon);

    //free dynamically allocated memory for dungeon 
	int i;
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
	free(path);

	return 0;
}