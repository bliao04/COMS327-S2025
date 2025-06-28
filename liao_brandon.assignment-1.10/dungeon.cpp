#include <stdlib.h>
#include <math.h>
#include "dungeon_generator.h"

/* function to place rooms within a given dungeon */
int place_rooms(struct Dungeon *dungeon) {
    //generate random coordinates (x, y) for the top-left corner of the room
	int x = (rand() % (dungeon->w-1)) +1;
	int y = (rand() % (dungeon->h-1)) +1;
	struct Room room;
	room.bottom_right = initPos();
	room.top_left = initPos();
	room.ctr = initPos();

    //set the top-left corner of the room
	setPosX(room.top_left, x);
	setPosY(room.top_left, y);
		
    //generate random width and height for the room
	int room_width = (rand() % 4) + 4; //width between 4 and 7
	int room_height = (rand() % 4) + 3; //height between 3 and 6

    //set the room's height and width
	room.h = room_height;
	room.w = room_width;
	
    //calculate the bottom-right corner of the room
	setPosX(room.bottom_right, x + room.w-1);
	setPosY(room.bottom_right, y + room.h-1);

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
	if(getPosX(room.bottom_right) >= dungeon->w || getPosY(room.bottom_right) >= dungeon->h) {
		return placed;
	}
	
    //check surrounding areas to ensure no adjacent rooms
    //top row check
	for(i = getPosX(room.top_left)-1; i < getPosX(room.bottom_right)+2 && getPosX(room.top_left)-1 >= 0 && getPosX(room.bottom_right)+1 < dungeon->w && getPosY(room.top_left)-1 >= 0; i++) {
		if((dungeon->p[getPosY(room.top_left)-1][i]).c == '.') {
			return placed;
		}
	}

	//bottom row check  
	for(i = getPosX(room.top_left)-1; i < getPosX(room.bottom_right)+2 && getPosX(room.top_left)-1 >= 0 && getPosX(room.bottom_right)+1 < dungeon->w && getPosY(room.bottom_right)+1 < dungeon->h; i++) {  
		if((dungeon->p[getPosY(room.bottom_right)+1][i]).c == '.') {  
			return placed; //adjacent room found, return "not placed"  
		}  
	}  

	//left side check  
	for(i = getPosY(room.top_left); i < getPosY(room.bottom_right)+1 && getPosY(room.bottom_right)+1 < dungeon->h && getPosX(room.top_left)-1 >= 0; i++) {  
		if((dungeon->p[i][getPosX(room.top_left)-1]).c == '.') {  
			return placed; //adjacent room found, return "not placed"  
		}  
	}  

	//right side check  
	for(i = getPosY(room.top_left); i < getPosY(room.bottom_right)+1 && getPosY(room.bottom_right)+1 < dungeon->h && getPosX(room.bottom_right)+1 < dungeon->w; i++) {  
		if((dungeon->p[i][getPosX(room.bottom_right)+1]).c == '.') {  
			return placed; //adjacent room found, return "not placed"  
		}  
	}  

    //mark the room area in the dungeon path grid
	for(i = y; i < y+room_height; i++) {
		for(j = x; j < x+room_width; j++) {
			dungeon->p[i][j].c = '.';
			dungeon->d[i][j].hardness = 0;
		}
	}

    //add the room to the dungeon's room array if there is space
	if(dungeon->num_rooms < dungeon->max_rooms) {
		dungeon->num_rooms++;
		room.id = dungeon->num_rooms-1; //assign a unique ID to the room (pos in array)
		setPosX(room.ctr, (room.w)/2 + getPosX(room.top_left));
		setPosY(room.ctr, (room.h)/2 + getPosY(room.top_left));
		room.connected = FALSE;    
        room.processed = FALSE;
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
    int i;

    //array used to track whether each room is connected to the corridor network
	int is_connected[dungeon->num_rooms];
	for(i = 0; i < dungeon->num_rooms; i++) {
		is_connected[i] = 0;
	}

    //array to store the distances from the current room to all other rooms
    double distances[dungeon->num_rooms];
	for(i = 0; i < dungeon->num_rooms; i++) {
		distances[i] = 0;
	}

    //max number of paths that can be generated
	int max_paths = dungeon->num_rooms * 3;

    //array to store the paths (corridors between rooms)
	struct Path paths[max_paths]; 

    //counter for the number of paths generated
	int path_cnt = 0;

    //starting position (index of the current room)
	int	room_pos = 0;


    //initialize the distances array with -1 (ie uncalculated distances)
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
			d =  sqrt(pow(getPosX(dungeon->r[i].ctr) - getPosX(dungeon->r[room_pos].ctr), 2) + pow(getPosY(dungeon->r[i].ctr) - getPosY(dungeon->r[room_pos].ctr), 2));
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
		int x = getPosX(dungeon->r[paths[i].prev].ctr); //starting x-coordinate of the path  
		int y = getPosY(dungeon->r[paths[i].prev].ctr); //starting y-coordinate of the path  

		//continue until the path reaches the destination room  
		while(x != getPosX(dungeon->r[paths[i].next].ctr) || y != getPosY(dungeon->r[paths[i].next].ctr)) {  
			int dirx = 0; //-1 for left, 1 for right  
			int diry = 0; //-1 for down, 1 for up  

			//determine the x-direction  
			if(x < getPosX(dungeon->r[paths[i].next].ctr)) {  
				dirx = 1; //move right  
			} else if(x > getPosX(dungeon->r[paths[i].next].ctr)) {  
				dirx = -1; //move left  
			}  

			//determine the y-direction  
			if(y < getPosY(dungeon->r[paths[i].next].ctr)) {  
				diry = 1; //move down  
			} else if(y > getPosY(dungeon->r[paths[i].next].ctr)) {  
				diry = -1; //move up  
			}  

			//mark the current tile as part of the corridor  
			dungeon->d[y][x].path = 1;  
			/* don't place corridors in rooms */  
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
    dungeon->u_stairs = (Position*)calloc(dungeon->nu, sizeof(Position));

    dungeon->nd = 1; //1 downward staircase
    dungeon->d_stairs = (Position*)calloc(dungeon->nd, sizeof(Position));

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
			//store the position
            dungeon->d_stairs[0].x = x; 
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
}

/* function to initialize the dungeon structure */
struct Dungeon init_dungeon(int h, int w, int mr) {
    //initialize the new dungeon
	struct Dungeon dungeon;
	dungeon.h = h;
	dungeon.w = w;
	dungeon.max_rooms = mr;
	dungeon.num_rooms = 0;
	dungeon.pcSS = 0;
	dungeon.nu = 0;
	dungeon.u_stairs = NULL;
	dungeon.nd = 0;
	dungeon.d_stairs = NULL;
	dungeon.ns	= 0;
	dungeon.ms	= w*h;
	dungeon.t	= 0;
	dungeon.go	= FALSE;
	dungeon.win = FALSE;
	dungeon.fog_enabled = 1;
    dungeon.teleport_mode = 0;
	dungeon.look_mode = 0;
	dungeon.ranged_mode = 0;
	dungeon.cursor_x = 0;
	dungeon.cursor_y = 0;

    //allocate memory for the dungeon grid (2D array of Tile pointers)
	dungeon.d = (Tile**)calloc(dungeon.h, sizeof(Tile *));
	
	int i;
    //allocate memory for each row in the dungeon grid
	for(i = 0; i < dungeon.h; i++) {
		dungeon.d[i] = (Tile*)calloc(dungeon.w, sizeof(Tile));
	}

    //allocate memory for the path grid (2D array of Tile pointers)
	dungeon.p = (Tile**)calloc(dungeon.h, sizeof(Tile *));

    //allocate memory for each row in the path grid
	for(i = 0; i < dungeon.h; i++) {
		dungeon.p[i] = (Tile*)calloc(dungeon.w, sizeof(Tile));
	}	

    //allocate memory for the array of rooms (maximum number of rooms)
	dungeon.r = (Room*)calloc(dungeon.max_rooms, sizeof(Room));

	//allocate memory for the sprites 
	dungeon.ss = (Sprite*)calloc(dungeon.ms, sizeof(Sprite *));

	//djikstra-based cost map allocation 
	dungeon.cst = (int**)calloc(w*h, sizeof(int *));
	for(i = 0; i < dungeon.h; i++) {
		dungeon.cst[i] = (int*)calloc(dungeon.w, sizeof(int));
	}

	//djikstra-based cost map allocation
	dungeon.csnt = (int**)calloc(w*h, sizeof(int *));
	for(i = 0; i < dungeon.h; i++) {
		dungeon.csnt[i] = (int*)calloc(dungeon.w, sizeof(int));
	}

	//items
	dungeon.items = new Item[dungeon.ms];
	dungeon.nit = 0;

    //return the initialized dungeon
	return dungeon;
}