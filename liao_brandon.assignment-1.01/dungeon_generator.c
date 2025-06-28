#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>
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
};

struct Dungeon{
	struct Tile **d; //dungeon grid
	struct Tile **p; //print grid
    struct Room *r;	//rooms grid
    int	w; //dungeon width 
	int	h; //dungeon height
	int	num_rooms; //number of rooms in dungeon 
	int	max_rooms;	//max number of rooms in dungeon
};

/* function to initialize the dungeon structure */
struct Dungeon init_dungeon(int h, int w, int mr) {
    //initialize the new dungeon
	struct Dungeon dungeon;
	dungeon.h = h;
	dungeon.w = w;
	dungeon.max_rooms = mr;
	dungeon.num_rooms = 0;

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
int all_rooms_connected(int *cnxns, int length) {
	int i;

	//iterate through the connections array
	for(i = 0; i < length; i++) {  
        //if any room is not connected (marked by 0), return FALSE
		if(cnxns[i] != 1) {
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

    //array to store the distances from the current room to all other rooms
	double distances[dungeon->num_rooms];

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

    //continue generating paths until all rooms are connected or the maximum number of paths is reached
	while(all_rooms_connected(is_connected, dungeon->num_rooms) == FALSE && path_cnt < max_paths) {
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
    //flags to track whether the up staircase and down staircase have been placed
    int up_placed = 0;
    int down_placed = 0;

    //continue until both staircases are placed
    while (up_placed == 0 || down_placed == 0) {
        //generate random coordinates (x, y) within the dungeon boundaries
        int x = (rand() % (dungeon->w - 2)) + 1;
        int y = (rand() % (dungeon->h - 2)) + 1;

        //check if the selected tile is part of a room or a corridor
        if (dungeon->d[y][x].c == '.' || dungeon->d[y][x].path == 1) {
            //place the up staircase if it hasn't been placed yet
            if (up_placed == 0) {
                dungeon->d[y][x].c = '<'; //mark the tile with the up staircase symbol
                up_placed = 1; //set the flag to indicate the up staircase has been 

            } 
            //place the down staircase if it hasn't been placed yet
            else if (down_placed == 0) {
                dungeon->d[y][x].c = '>'; //mark the tile with the down staircase symbol
                down_placed = 1; //set the flag to indicate the down staircase has been placed
            }
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
			(dungeon->d[i][j]).hardness = 0;
		}
	}

    //set the hardness of the top and bottom borders to be immutible (100 hardness for now)
	for(i = 0; i < dungeon->w; i++) {
		(dungeon->d[0][i]).hardness = 100;
	}
	for(i = 0; i < dungeon->w; i++) {
		(dungeon->d[dungeon->h-1][i]).hardness = 100;
	}

    //set the hardness of the left and right borders to be immutible (100 hardness for now)
	for(i = 0; i < dungeon->h; i++) {
		(dungeon->d[i][0]).hardness = 100;
	}
	for(i = 0; i < dungeon->h; i++) {
		(dungeon->d[i][dungeon->w-1]).hardness = 100;
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
			if(dungeon->d[i][j].path == 1) {
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

    //mark staircases on the print grid using either the '<' or '>' symbol
    for (int i = 0; i < dungeon->h; i++) {
        for (int j = 0; j < dungeon->w; j++) {
            if (dungeon->d[i][j].c == '<' || dungeon->d[i][j].c == '>') {
                dungeon->p[i][j].c = dungeon->d[i][j].c;
            }
        }
    }

    //print the final dungeon layout to the console
	for(i = 0; i < dungeon->h; i++) {
		int j;
		for(j = 0; j < dungeon->w; j++) {
			printf("%c", (dungeon->p[i][j]).c);
		}
		printf("\n");	
	}
}

int main(int argc, char * argv[]) 
{
	srand(time(NULL));

	struct Dungeon dungeon = init_dungeon(21, 80, 12);

    generate_dungeon(&dungeon);
	place_corridors(&dungeon);
    place_staircases(&dungeon);
	print_dungeon(&dungeon);

	free(dungeon.d);
	free(dungeon.r);

    return 0;
}