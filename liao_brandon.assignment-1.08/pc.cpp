
#include "dungeon_generator.h"
#include <cstdlib>

/* update pc's fog-of-war memory grid */
void updateMemory(Dungeon * dungeon) {
	int i;
	int j;

    //scan 7x7 area around pc (3 tiles radius)
	for(i = 1; i < dungeon->h-1; i++) {
		for(j = 1; j < dungeon->w-1; j++) {
		    //check visibility range and passability
			if(abs(i - ((PC *) &dungeon->ss[0])->p.y) <= 3 && abs(j - ((PC *) &dungeon->ss[0])->p.x) <= 3 && dungeon->d[i][j].hardness == 0) {
			    //store visible tile in memory
				dungeon->plyr->mem[i][j].c = dungeon->p[i][j].c;
				dungeon->plyr->mem[i][j].v = true;
				dungeon->plyr->mem[i][j].color = dungeon->p[i][j].color;
			}
		}
	}
}

/* cast sprite array element to pc type */
PC * thisAPC(Sprite * arr, int i) {
	return arr[i].thisPC();
}

/* downcast sprite to pc type */
PC * Sprite::thisPC() {
    return (class PC *) this;
}

/* retrieve memorized tile character */
char getMem(Dungeon * dungeon, int y, int x) {
	return dungeon->plyr->mem[y][x].c;
}

/* get pc reference from sprite array */
PC * getPC(Sprite * arr) {
	return (class PC *) &(arr[0]);
}

/* initialize player character with default values */
PC * initPC(Dungeon * dungeon) {
	class PC * p= new PC;

    //basic sprite setup
	p->c = '@';
	p->sn = 0;
	p->a = TRUE;

    //default pc stats
	p->s.s = 10; //base speed
	p->s.tu = FALSE; //no tunneling
	p->s.eb = FALSE; //no erratic behavior
	p->s.te = FALSE; //no telepathy
	p->s.in = FALSE; //no intelligence

    //place pc in random room
	int r_id = rand() % dungeon->num_rooms;
	int x = (rand() % dungeon->r[r_id].w) + getPosX(dungeon->r[r_id].top_left);
	int y = (rand() % dungeon->r[r_id].h) + getPosY(dungeon->r[r_id].top_left);
	p->p.x = p->to.x = x;
	p->p.y = p->to.y = y;

	p->t = 0; //reset turn counter
	dungeon->plyr = p; //store reference

    //allocate fog-of-war memory grid
	p->mem = (class Memory **) malloc(dungeon->h * sizeof(Memory *));
	int i;
	for(i = 0; i < dungeon->h; i++) {
		p->mem[i] = (Memory *) malloc(dungeon->w * sizeof(Memory));
	}

    //initialize memory to empty state
	int j;
	for(i = 0; i < dungeon->h; i++) {
		for(j = 0; j < dungeon->w; j++) {
			p->mem[i][j].c = ' ';
			p->mem[i][j].v = false;
		}
	}

	return p;
}