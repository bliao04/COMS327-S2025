#include <stdio.h>
#include <ncurses.h>
#include "dungeon_generator.h"

/* function to print the dungeon */
void print_dungeon(struct Dungeon *dungeon, int nt, int t) {
    clear();

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

	//mark sprites on the print grid
	for(i = 0; i < dungeon->ns; i++) {
		if(dungeon->ss[i].a == TRUE) {
			dungeon->p[dungeon->ss[i].p.y][dungeon->ss[i].p.x].c = dungeon->ss[i].c;
		}
	}

	//print non-tunneling dijkstra's
	if(nt > 0) {
		for(i = 0; i < dungeon->h; i++) {
			for(j = 0; j < dungeon->w; j++) {
				if(dungeon->d[i][j].hardness == 0) {
					int c = dungeon->csnt[i][j];
					dungeon->p[i][j].c = '0' + c % 10;
				}
			}
		}
		dungeon->p[dungeon->pc.y][dungeon->pc.x].c = '@';
	}

	//print tunneling dijkstra's
	if(t > 0) {
		for(i = 0; i < dungeon->h; i++) {
			for(j = 0; j < dungeon->w; j++) {
				if (dungeon->d[i][j].hardness < 255) {					
					int c = dungeon->cst[i][j];
					dungeon->p[i][j].c = '0' + c % 10;
				}

			}
		}
		dungeon->p[dungeon->pc.y][dungeon->pc.x].c = '@';
	}

    //print the final dungeon layout to the console
	for(i = 0; i < dungeon->h; i++) {
        int j;
		for(j = 0; j < dungeon->w; j++) {
            mvaddch(i+1, j, (dungeon->p[i][j]).c);
		}
	}
    
    refresh();
}