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
		for(i = getPosY(dungeon->r[h].top_left); i < getPosY(dungeon->r[h].bottom_right)+1; i++) {
			for(j = getPosX(dungeon->r[h].top_left); j < getPosX(dungeon->r[h].bottom_right)+1; j++) {
				dungeon->p[i][j].c = '.';
			}
		}
	}

    //mark staircases (1 of each for now)
    dungeon->p[getPosY(dungeon->u_stairs)][getPosX(dungeon->u_stairs)].c = '<';
    dungeon->p[getPosY(dungeon->d_stairs)][getPosX(dungeon->d_stairs)].c = '>';

	updateMemory(dungeon);

	//mark sprites on the print grid
	for(i = 0; i < dungeon->ns; i++) {
		if(getSpriteAA(dungeon->ss, i) == TRUE) {
			dungeon->p[getSpriteAPY(dungeon->ss, i)][getSpriteAPX(dungeon->ss, i)].c = getSpriteAC(dungeon->ss, i);
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
		dungeon->p[getSpriteAPY(dungeon->ss, dungeon->pcSS)][getSpriteAPX(dungeon->ss, dungeon->pcSS)].c = '@';
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
		dungeon->p[getSpriteAPY(dungeon->ss, dungeon->pcSS)][getSpriteAPX(dungeon->ss, dungeon->pcSS)].c = '@';
	}

	//integrated display logic with fog and teleport
	int pc_x = getSpriteAPX(dungeon->ss, dungeon->pcSS);
	int pc_y = getSpriteAPY(dungeon->ss, dungeon->pcSS);

	for(int i = 0; i < dungeon->h; i++) {
		for(int j = 0; j < dungeon->w; j++) {
			if(dungeon->teleport_mode) {
				//teleport mode, show full map with cursor
				if(i == dungeon->cursor_y && j == dungeon->cursor_x) {
					mvaddch(i+1, j, '*');
				} else {
					mvaddch(i+1, j, dungeon->p[i][j].c);
				}
			}
			else if(dungeon->fog_enabled) {
				//normal mode with fog of war
				mvaddch(i+1, j, getMem(dungeon, i, j));
			}
			else {
				//fog disabled, show everything
				mvaddch(i+1, j, dungeon->p[i][j].c);
			}
		}
	}

	//always draw PC on top
	mvaddch(pc_y+1, pc_x, '@');
    
    refresh();
}