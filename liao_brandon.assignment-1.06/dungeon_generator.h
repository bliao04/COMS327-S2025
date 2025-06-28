#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b)) //max value macro
#endif

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b)) //min value macro
#endif

#ifndef dungeon_generator
#define dungeon_generator
		typedef int Bool;
		#define	TRUE 1
		#define	FALSE 0
		
		class PC;

		class Memory {
		public:
			char c; //remembered tile tile char
			bool v; //visibility flag 
		};

		class Position {
		public:
			int	x; //x coordinate
			int	y; //y coordinate
		};

		class Stats {
		public:
			bool    in; //intelligence
			bool    te; //telepathy
			bool    tu; //tunneling ability
			bool    eb; //erratic behaviour
			int     s;  //speed
		};

		class Sprite {
		public:
			friend class PC;
			friend class Monster;
			Position	p;	//position
			char		c;	//sprite character
			Stats       s;	//stats
			int			t;	//turn count
			Position	to;	//position to move to
			int			sn;	//sprite number
			Position	pc;	//last known PC location
			bool		a;	//alive flag
			Sprite * 	thisSprite(void); //get self as base class
			PC	* 		thisPC(void); //cast to PC
		};

		class Monster : public Sprite {
		public:
			friend class Sprite;
		};

		class PC : public Sprite {
		public:
			friend class Sprite;
			int		view; //fog of war visibility radius
			Memory	**	mem; //grid tracking explored areas
		};

		class Dungeon {
		public:
			struct Tile **d; //dungeon grid
			struct Tile **p; //print grid
			struct Room *r;	//rooms grid
			int	w; //dungeon width 
			int	h; //dungeon height
			int	num_rooms; //number of rooms in dungeon 
			int	max_rooms;	//max number of rooms in dungeon
			int v; //file version
			int s; //file size
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
			PC *plyr; //direct PC reference
			int fog_enabled; //fog of war toggle
			int teleport_mode; //PC teleport state
			int cursor_x; //teleport target x
			int cursor_y; //teleport target y
		};

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

		struct Room
		{
			struct Position * top_left; //top left coordinate of the room
			struct Position	* bottom_right; //bottom right coordinate of the room 
			struct Position	* ctr; //center point of the room
			int	w; //room width 
			int	h; //room height
			int	id; //room ID 
			int	processed; //mark 1 if processed, 0 if not processed (ie has corridors)
			int connected; //true or false for if a room is connected or not
		};

		struct Tile_Node
		{
			int x; //x coordinate
			int y; //y coordinate
			int cost; //cost to reach this tile from starting point
			int v; //visited flag 
		};

		/* monsters.cpp */
		void add_sprite(struct Dungeon * dungeon, struct Sprite * s);
		struct Sprite * generate_sprite(struct Dungeon *dungeon, char c, int x, int y, int r);
		void generate_next_move(struct Dungeon *dungeon, int sn);
		void parse_move(struct Dungeon *dungeon, int sn);
		Bool check_monsters_alive(struct Dungeon *dungeon);

		/* dungeon.cpp */
		void place_corridors(struct Dungeon * dungeon);
		void place_staircases(struct Dungeon * dungeon);
		void generate_dungeon(struct Dungeon * dungeon);
		struct Dungeon init_dungeon(int h, int w, int mr);

		/* print.cpp */
		void print_dungeon(struct Dungeon * dungeon, int nt, int t);

		/* dungeon_generator.cpp */
		void map_dungeon_t(struct Dungeon *dungeon);

		typedef struct Position Position;
		typedef struct Stats Stats;
		typedef struct Sprite Sprite;
		typedef struct Monster Monster;
		typedef struct PC PC;

    	/* sprite management interface */
		Sprite * initSprite(); //create base sprite
		Sprite * initSprites(int); //allocate sprite array
		void copySprite(Sprite *, Sprite *); //duplicate sprite
		void copyASprite(Sprite * to, int n, Sprite * from); //array-to-array copy
		Sprite * thisASprite(Sprite * arr, int i); //get array element

		/* unified accessors */
		//singleton access (direct object)
		int getSpritePX(Sprite *);
		int getSpritePY(Sprite *);
		char getSpriteC(Sprite *);
		bool getSpriteSIn(Sprite *);
		bool getSpriteSTe(Sprite *);
		bool getSpriteSTu(Sprite *);
		bool getSpriteSEb(Sprite *);
		int getSpriteSS(Sprite *);
		int getSpriteT(Sprite *);
		int getSpriteToX(Sprite *);
		int getSpriteToY(Sprite *);
		int getSpriteSn(Sprite *);
		int getSpritePcX(Sprite *);
		int getSpritePcY(Sprite *);
		bool getSpriteA(Sprite *);

    	//array-based access (sprite collection)
		int getSpriteAPX(Sprite *, int);
		int getSpriteAPY(Sprite *, int);
		char getSpriteAC(Sprite *, int);
		bool getSpriteASIn(Sprite *, int);
		bool getSpriteASTe(Sprite *, int);
		bool getSpriteASTu(Sprite *, int);
		bool getSpriteASEb(Sprite *, int);
		int getSpriteASS(Sprite *, int);
		int getSpriteAT(Sprite *, int);
		int getSpriteAToX(Sprite *, int);
		int getSpriteAToY(Sprite *, int);
		int getSpriteASn(Sprite *, int);
		int getSpriteAPcX(Sprite *, int);
		int getSpriteAPcY(Sprite *, int);
		bool getSpriteAA(Sprite *, int);

		/* unified mutators */
		//singleton modification
		void setSpritePX(Sprite *, int);
		void setSpritePY(Sprite *, int);
		void setSpriteC(Sprite *, char);
		void setSpriteSIn(Sprite *, bool);
		void setSpriteSTe(Sprite *, bool);
		void setSpriteSTu(Sprite *, bool);
		void setSpriteSEb(Sprite *, bool);
		void setSpriteSS(Sprite *, int);
		void setSpriteT(Sprite *, int);
		void setSpriteToX(Sprite *, int);
		void setSpriteToY(Sprite *, int);
		void setSpriteSn(Sprite *, int);
		void setSpritePcX(Sprite *, int);
		void setSpritePcY(Sprite *, int);
		void setSpriteA(Sprite *, bool);

    	//array-based modification
		void setSpriteAPX(Sprite *, int, int);
		void setSpriteAPY(Sprite *, int, int);
		void setSpriteAC(Sprite *, int, char c);
		void setSpriteASIn(Sprite *, int, bool);
		void setSpriteASTe(Sprite *, int, bool);
		void setSpriteASTu(Sprite *, int, bool);
		void setSpriteASEb(Sprite *, int, bool);
		void setSpriteASS(Sprite *, int, int);
		void setSpriteAT(Sprite *, int, int);
		void setSpriteAToX(Sprite *, int, int);
		void setSpriteAToY(Sprite *, int, int);
		void setSpriteASn(Sprite *, int, int);
		void setSpriteAPcX(Sprite *, int, int);
		void setSpriteAPcY(Sprite *, int, int);
		void setSpriteAA(Sprite *, int, bool);

    	/* PC specific */
		PC * initPC(struct Dungeon *); //initialize PC

		//position helper
		int getPosX(Position * p);
		int getPosY(Position * p);
		void setPosX(Position * p, int n);
		void setPosY(Position * p, int n);

		Position * initPos(void); //fresh position allocator
		void updateMemory(struct Dungeon *); //refresh fog-of-war
		char getMem(struct Dungeon *, int, int); //recall stored tile
		PC * getPC(Sprite * arr); //find PC in array
		PC * thisAPC(Sprite * arr, int i); //cast to PC
#endif