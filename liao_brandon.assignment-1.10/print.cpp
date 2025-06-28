#include <stdio.h>
#include <ncurses.h>
#include "dungeon_generator.h"

/* function to print the dungeon */
void print_dungeon(struct Dungeon *dungeon, int nt, int t) {
    clear();
	refresh();

	//define colors
	init_pair(COLOR_BLACK, COLOR_WHITE, COLOR_BLACK);
	init_pair(COLOR_RED, COLOR_RED, COLOR_BLACK);
	init_pair(COLOR_GREEN, COLOR_GREEN, COLOR_BLACK);
	init_pair(COLOR_YELLOW, COLOR_YELLOW, COLOR_BLACK);
	init_pair(COLOR_BLUE, COLOR_BLUE, COLOR_BLACK);
	init_pair(COLOR_MAGENTA, COLOR_MAGENTA, COLOR_BLACK);
	init_pair(COLOR_CYAN, COLOR_CYAN, COLOR_BLACK);
	init_pair(COLOR_WHITE, COLOR_WHITE, COLOR_BLACK);
	init_pair(10, COLOR_RED, COLOR_BLACK);  

	int i;
	int j;
	int h;
	
    //initialize the print grid by setting all tiles to walls using the ' ' symbol
	for(i = 0; i < dungeon->h; i++) {
		for(j = 0; j < dungeon->w; j++) {
			dungeon->p[i][j].c = ' ';
			dungeon->p[i][j].color = COLOR_BLACK;
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

	//mark items on the print grid 
	for(i = 0; i < dungeon->nit; i++) {
		char s = '*';
		
		switch(dungeon->items[i].t) {
		case WEAPON: s = '|'; break;
		case OFFHAND: s = ')'; break;
		case RANGED: s = '}'; break;
		case ARMOR: s = '['; break;
		case HELMET: s = ']'; break;
		case CLOAK: s = '('; break;
		case GLOVES: s = '{'; break;
		case BOOTS: s = '\\'; break;
		case RING: s = '='; break;
		case AMULET: s = '\"'; break;
		case LIGHT: s = '_'; break;
		case SCROLL: s = '~'; break;
		case BOOK: s = '\?'; break;
		case FLASK: s = '!'; break;
		case GOLD: s = '$'; break;
		case AMMUNITION: s = '/'; break;
		case FOOD: s = ','; break;
		case WAND: s = '-'; break;
		case CONTAINER: s = '%'; break;
		default: break;
		}
		
		int c = COLOR_WHITE;
		
		switch(dungeon->items[i].c) {
		case RED: c = COLOR_RED; break;
		case GREEN: c = COLOR_GREEN; break;
		case BLUE: c = COLOR_BLUE; break;
		case CYAN: c = COLOR_CYAN; break;
		case YELLOW: c = COLOR_YELLOW; break;
		case MAGENTA: c = COLOR_MAGENTA; break;
		case WHITE: c = COLOR_WHITE; break;
		case BLACK: c = COLOR_BLACK; break;
		default: c = c; break;
		}
		
		dungeon->p[dungeon->items[i].p.y][dungeon->items[i].p.x].c = s;
		dungeon->p[dungeon->items[i].p.y][dungeon->items[i].p.x].color = c;
	}

	//mark sprites on the print grid
	for(i = 0; i < dungeon->ns; i++) {
		if(getSpriteAA(dungeon->ss, i) == TRUE) {
			int c = COLOR_WHITE;
			
			switch(dungeon->ss[i].color) {
				case RED: c = COLOR_RED; break;
				case GREEN: c = COLOR_GREEN; break;
				case BLUE: c = COLOR_BLUE; break;
				case CYAN: c = COLOR_CYAN; break;
				case YELLOW: c = COLOR_YELLOW; break;
				case MAGENTA: c = COLOR_MAGENTA; break;
				case WHITE: c = COLOR_WHITE; break;
				case BLACK: c = COLOR_BLACK; break;
				default: c = COLOR_WHITE; break;
			}   
			
			dungeon->p[dungeon->ss[i].p.y][dungeon->ss[i].p.x].color = c;

			dungeon->p[getSpriteAPY(dungeon->ss, i)][getSpriteAPX(dungeon->ss, i)].c = getSpriteAC(dungeon->ss, i);
		}
	}

	updateMemory(dungeon);

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
			bool has_sprite = false;
			for(int s = 0; s < dungeon->ns; s++) {
				if(getSpriteAPX(dungeon->ss, s) == j && getSpriteAPY(dungeon->ss, s) == i && getSpriteAA(dungeon->ss, s)) {
					has_sprite = true;
					break;
				}
			}
			
			if(!has_sprite && dungeon->plyr->mem[i][j].v) {
				dungeon->plyr->mem[i][j].c = dungeon->p[i][j].c;
				dungeon->plyr->mem[i][j].color = COLOR_BLACK;
			}
		
			if(dungeon->teleport_mode) {
				//teleport mode, show full map with cursor
				if(i == dungeon->cursor_y && j == dungeon->cursor_x) {
					attron(COLOR_PAIR(dungeon->p[i][j].color));
					mvaddch(i+1, j, '*');
					attroff(COLOR_PAIR(dungeon->p[i][j].color));
				} else {
					attron(COLOR_PAIR(dungeon->p[i][j].color));
					mvaddch(i+1, j, dungeon->p[i][j].c);
					attroff(COLOR_PAIR(dungeon->p[i][j].color));
				}
			}
			else if(dungeon->look_mode) {
				//look mode, show explored areas with cursor
				if(i == dungeon->cursor_y && j == dungeon->cursor_x) {
					attron(A_REVERSE | COLOR_PAIR(dungeon->p[i][j].color));
					mvaddch(i+1, j, getMem(dungeon, i, j));
					attroff(A_REVERSE | COLOR_PAIR(dungeon->p[i][j].color));
				} else {
					attron(COLOR_PAIR(dungeon->p[i][j].color));
					mvaddch(i+1, j, getMem(dungeon, i, j));
					attroff(COLOR_PAIR(dungeon->p[i][j].color));
				}
			}
			else if(dungeon->ranged_mode) {
				//calculate and display targeting info
                int current_range = MAX(abs(dungeon->cursor_x - pc_x), abs(dungeon->cursor_y - pc_y));
                int max_range = 0;
				for (int i = 0; i < 12; i++) {
					if (dungeon->plyr->eqsp[i] && dungeon->plyr->eqs[i].t == RANGED) {
						max_range = dungeon->plyr->eqs[i].sa;
						break;
					}
				}
                mvprintw(0, 0, "Ranged Attack - Range %d/%d (f)ire (ESC)cancel", 
                        current_range, max_range);
                refresh();

				//ranged attack targeting - red 'X' cursor
                if(i == dungeon->cursor_y && j == dungeon->cursor_x) {
                    attron(COLOR_PAIR(10));
                    mvaddch(i+1, j, 'X');
                    attroff(COLOR_PAIR(10));
                } else {
                    attron(COLOR_PAIR(dungeon->p[i][j].color));
                    mvaddch(i+1, j, getMem(dungeon, i, j));
                    attroff(COLOR_PAIR(dungeon->p[i][j].color));
                }
            }
			else if(dungeon->fog_enabled) {
				//normal mode with fog of war
				attron(COLOR_PAIR(dungeon->p[i][j].color));
				mvaddch(i+1, j, getMem(dungeon, i, j));
				attroff(COLOR_PAIR(dungeon->p[i][j].color));
			}
			else {
				//fog disabled, show everything
				attron(COLOR_PAIR(dungeon->p[i][j].color));
				mvaddch(i+1, j, dungeon->p[i][j].c);
				attroff(COLOR_PAIR(dungeon->p[i][j].color));
			}
		}
	}

	//always draw PC on top
	mvaddch(pc_y+1, pc_x, '@');
}