
#ifndef dungeon_generator
#define dungeon_generator
#include "heap.h"
#define	TRUE 1
#define	FALSE 0
typedef int Bool;

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

struct Dungeon
{
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
	int	**csnt; //cost grid (nontunneling monsters)
	int	**cst;	//cost grid (tunneling monsters)
    struct Sprite *ss; //sprite array
	int	ns;	//number of sprites
	int	ms;	//max number of sprites
    int pcSS; //location of PC in SpriteS array (.ss)
    int t; //turn number 
	Bool go; //game over flag 
};

struct Tile_Node
{
	int x; //x coordinate
	int y; //y coordinate
	int cost; //cost to reach this tile from starting point
	int v; //visited flag 
};

struct Stats
{
    Bool in; //intelligence
    Bool te; //telepathy
    Bool tu; //tunneling ability 
    Bool eb; //erratic behaviour 
    int s; //speed
};

struct Sprite
{
	struct Position	p; //position
	char c;	//sprite character
    struct Stats s; //stats 
	int	t; // turn count
	struct Position	to; //position to move to
	int	sn;	//sprite number
	struct Position	pc;	//last known pc location 
	Bool a; //alive flag
};

struct Event{
	int	sn;	//sprite number
	int speed; //sprite speed 
	int	turn; //turn counter 
	struct Position	to; //position to move to 
};

/* monsters.c */
void add_sprite(struct Dungeon *dungeon, struct Sprite s);
struct Sprite generate_sprite(struct Dungeon *dungeon, char c, int x, int y, int r);
void generate_next_move(struct Dungeon *dungeon, int sn);
void parse_move(struct Dungeon *dungeon, int sn);
Bool check_monsters_alive(struct Dungeon *dungeon);

/* dungeon.c */
void place_corridors(struct Dungeon * dungeon);
void place_staircases(struct Dungeon * dungeon);
void generate_dungeon(struct Dungeon * dungeon);
struct Dungeon init_dungeon(int h, int w, int mr);

/* print.c */
void print_dungeon(struct Dungeon * dungeon, int nt, int t);

/* dungeon_generator.c */
void map_dungeon_t(struct Dungeon *dungeon);

#endif