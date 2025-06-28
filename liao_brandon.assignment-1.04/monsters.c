#include <stdio.h>
#include <stdlib.h>
#include "dungeon_generator.h"
#include "heap.h"

/* helper function to test if a sprite can move to a location */
bool test_loc(struct Dungeon * dungeon, int x, int y, struct Sprite *s) {
    //ensure location in within dungeon bounds
	if(x > 0 && x < dungeon->w-1 && y > 0 && y < dungeon->h-1) {
		int hard = dungeon->d[y][x].hardness; //get the hardness of the location
        //ensure location is not an immutible wall
		if(dungeon->d[y][x].hardness < 255) {
            //non-tunneling sprites cannot move through hard terrain
			if(s->s.tu == FALSE && hard > 0)
				return FALSE; //location is invalid for movement
			return TRUE; //location is valid for movement
		}
	}
	return FALSE; //location is invalid for movement
}

/* helper function to check if a sprite is in the same room as the PC */
void with_pc(struct Dungeon * dungeon, struct Sprite * s, bool *in) {
	int pc_rid	= -1; //room ID of the pc
	int s_rid	= -1; //room ID of the sprite 
	struct Sprite *pc = &(dungeon->ss[dungeon->pcSS]); //get the PC sprite

    //iterate through all rooms to find which room the sprite and PC are in
	for(int i = 0; i < dungeon->num_rooms; i++) {
		if(s->p.x <= dungeon->r[i].bottom_right.x && s->p.y <= dungeon->r[i].bottom_right.y && s->p.x >= dungeon->r[i].top_left.x && s->p.y >= dungeon->r[i].top_left.y) {
			s_rid = i; //sprite is in this room
		}
		if(pc->p.x <= dungeon->r[i].bottom_right.x && pc->p.y <= dungeon->r[i].bottom_right.y && pc->p.x >= dungeon->r[i].top_left.x && pc->p.y >= dungeon->r[i].top_left.y) {
			pc_rid = i; //pc is in this room
		}
	}

    //if both the sprite and PC are in the same room, set the flag to TRUE
	if(pc_rid > 0 && s_rid > 0 && pc_rid == s_rid) {
		*in = TRUE;
    }
}

/* function to generate the next move for a sprite */
void generate_next_move(struct Dungeon * dungeon, int sn) {
	int sx = dungeon->ss[sn].p.x; //current x position of the sprite
	int sy = dungeon->ss[sn].p.y; //current y position of the sprite
	int xs[8] = {-1,0,1,1,1,0,-1,-1};
	int ys[8] = {-1,-1,-1,0,1,1,1,0};

	struct Sprite *s = &(dungeon->ss[sn]); //get the sprite

    //increment the turn counter based on the sprite's speed 
	dungeon->ss[sn].t += (100 / s->s.s);

	struct Position new = {-1, -1}; //new position to move to

    //reduce the hardness of the current location (for tunneling sprites)
	dungeon->d[s->p.y][s->p.x].hardness -= 85;
	if(dungeon->d[s->p.y][s->p.x].hardness < 0) {
		dungeon->d[s->p.y][s->p.x].hardness = 0;
    }

    //ensure the sprite is alive
	if(s->a == TRUE) {
		int i;
		int j;
		int eb = rand() % 2; //50% chance for erratic behavior

        //check all 8 possible directions for movement
		for(i = 0; i < 8; i++) {
			int px = sx + xs[i];
			int py = sy + ys[i];

			if(px >= 0 && px < dungeon->w && py >= 0 && py < dungeon->h) {
				//drunken/roving idiot PC movement (assignment 1.04)
				if(sn == dungeon->pcSS)
					goto PCEB;

				//check for erratic behavior
				if(s->s.eb == FALSE || (s->s.eb == TRUE && eb)) {
					/* check for intelligence and telepathy */
                    //if not telepathic, check if the sprite is in the same room as the PC
					if(s->s.te == FALSE) {
						/* see if you're in the same room */
						bool in_room = FALSE;
						with_pc(dungeon, s, &in_room);
						if(in_room == TRUE) {
                            //cache the PC's location
							s->pc = dungeon->ss[dungeon->pcSS].p;

							IN: ;
							if(s->s.in == TRUE) {
                                //intelligent sprites choose the best path
								int k;
								int lowest = 0;
								bool set = FALSE;
								if(s->s.tu) {
                                    //tunneling sprites consider hardness
									for(k = 0; k < 8; k++) {
										if(xs[k]+sx >= 0 && xs[k]+sx < dungeon->w && ys[k]+sy >= 0 && ys[k]+sy < dungeon->h) {
											if(dungeon->d[ys[k]+sy][xs[k]+sx].hardness < 255 && dungeon->cst[ys[k]+sy][xs[k]+sx] < dungeon->cst[ys[lowest]+sy][xs[lowest]+sx] && test_loc(dungeon, xs[k]+sx, ys[k]+sy, s) == TRUE && test_loc(dungeon, xs[lowest]+sx, ys[lowest]+sy, s) == TRUE) {
												lowest = k;
												set = TRUE;
											}
										}
									}
								} else {
                                    //non-tunneling sprites only move through open spaces
									for(k = 0; k < 8; k++) {
										px = xs[k]+sx;
										py = ys[k]+sy;
										if(px >= 0 && px < dungeon->w && py >= 0 && py < dungeon->h) {
											if(dungeon->d[py][px].hardness == 0 && dungeon->csnt[py][px] <= dungeon->csnt[ys[lowest]+sy][xs[lowest]+sx]) {
												lowest = k;
												set = TRUE;
											}
										}
									}
								}
								if(set == TRUE) {
									new.x = xs[lowest] + sx;
									new.y = ys[lowest] + sy;
									break;
								} else {
									new.x = sx;
									new.y = sy;
									break;
								}
							} else {
                                //non-intelligent sprites move in a straight line toward the PC
								if(s->pc.x < sx)
									px = sx - 1;
								if(s->pc.x > sx)
									px = sx + 1;
								if(s->pc.y < sy)
									py = sy - 1;
								if(s->pc.y > sy)
									py = sy + 1;

								if(test_loc(dungeon, px, py, s) == TRUE) {
									new.x = px;
									new.y = py;
									break;
								}
							}
						} else {
                            //if not in the same room and not telepathic, move randomly
							goto PCEB;
						}
					} else {
                        //telepathic sprites always know the PC's location
						s->pc = dungeon->ss[dungeon->pcSS].p;
						goto IN;
					}
				} else {
                    //erratic behavior, move randomly
					PCEB: ;
					j = 0;
					EB: ;
					int c = rand() % 9;
					px = xs[c] + sx;
					py = ys[c] + sy;
                    //try to find a valid location up to n times
					if(test_loc(dungeon, px, py, s) == FALSE && j < 8) {
						j++;
						goto EB;
					}
					if(test_loc(dungeon, px, py, s) == TRUE) {
						new.x = px;
						new.y = py;
					}

					break;
				}
			}
		}
	}

    //safety net, if no valid move is found, stay in place */
	if(new.x < 0)
		new.x = sx;
	if(new.y < 0)
		new.y = sy;

    //update the sprite's target position
	dungeon->ss[sn].to.x = new.x;
	dungeon->ss[sn].to.y = new.y;

    //check if the sprite is attacking the PC
	if (sn != dungeon->pcSS) { // Only check for monsters (not the PC)
		if (new.x == dungeon->ss[dungeon->pcSS].p.x && new.y == dungeon->ss[dungeon->pcSS].p.y) {
			dungeon->go = TRUE; // Monster attacks PC
		}
	}
    //check if the sprite is killing another sprite
	int i;
	for(i = 0; i < dungeon->ns; i++) {
		if(i != sn) {
			if(dungeon->ss[i].p.x == dungeon->ss[sn].to.x && dungeon->ss[i].p.y == dungeon->ss[sn].to.y && dungeon->ss[sn].a == TRUE) {
                dungeon->ss[i].a = FALSE; //kill the other sprite
            }
		}
	}
}

/* function to parse and then execute a movement */
void parse_move(struct Dungeon * dungeon, int sn) {
	dungeon->ss[sn].p.x = dungeon->ss[sn].to.x;
	dungeon->ss[sn].p.y = dungeon->ss[sn].to.y;
}

/* function to add a sprite to a dungeon */
void add_sprite(struct Dungeon * dungeon, struct Sprite s) {
    //ensure the maximum number of sprite's has not been reached
	if(dungeon->ns < dungeon->ms) {
		dungeon->ns++; //increment the number of sprites
	} else {
		goto END; //dungeon is full
	}

	if(s.c == '@') {
		dungeon->pcSS = dungeon->ns - 1; //set the PC's sprite index
    }

	dungeon->ss[dungeon->ns - 1] = s; //add the sprite to the dungeon

	END: ;
}

/* function to generate sprites with random characteristics */
struct Sprite generate_sprite(struct Dungeon * dungeon, char c, int x, int y, int r) {
	struct Sprite s;

	s.c = c; //set the sprite's character
	s.a = TRUE; //set the sprite as alive

    /* set stats based on the sprite type */
    if(s.c == '@') {
        s.s.s = 10; //pc has a fixed speed
        s.s.tu = FALSE; //pc cannot tunnel
		s.s.eb = FALSE; //pc does not behave erratically
		s.s.te = FALSE; //pc is not telepathic
		s.s.in = FALSE; //pc is not intelligent
	} else {
        //randomize stats for monsters
        s.s.s = (rand() % 16) + 5; //speed between 5 and 20
        s.s.in = rand() % 2; //intelligence (50% chance)
        s.s.te = rand() % 2; //telepathic (50% chance)
        s.s.tu = rand() % 2; //tunneling (50% chance)
        s.s.eb = rand() % 2; //erratic behavior (50% chance)

        //assign a character based on the sprite's attributes
        if(s.s.in == FALSE && s.s.te == FALSE && s.s.tu == FALSE && s.s.eb == FALSE)
            s.c = '0';
        else if(s.s.in == FALSE && s.s.te == FALSE && s.s.tu == FALSE && s.s.eb == TRUE)
            s.c = '1';
        else if(s.s.in == FALSE && s.s.te == FALSE && s.s.tu == TRUE && s.s.eb == FALSE)
            s.c = '2';
        else if(s.s.in == FALSE && s.s.te == FALSE && s.s.tu == TRUE && s.s.eb == TRUE)
            s.c = '3';
        else if(s.s.in == FALSE && s.s.te == TRUE && s.s.tu == FALSE && s.s.eb == FALSE)
            s.c = '4';
        else if(s.s.in == FALSE && s.s.te == TRUE && s.s.tu == FALSE && s.s.eb == TRUE)
            s.c = '5';
        else if(s.s.in == FALSE && s.s.te == TRUE && s.s.tu == TRUE && s.s.eb == FALSE)
            s.c = '6';
        else if(s.s.in == FALSE && s.s.te == TRUE && s.s.tu == TRUE && s.s.eb == TRUE)
            s.c = '7';
        else if(s.s.in == TRUE && s.s.te == FALSE && s.s.tu == FALSE && s.s.eb == FALSE)
            s.c = '8';
        else if(s.s.in == TRUE && s.s.te == FALSE && s.s.tu == FALSE && s.s.eb == TRUE)
            s.c = '9';
        else if(s.s.in == TRUE && s.s.te == FALSE && s.s.tu == TRUE && s.s.eb == FALSE)
            s.c = 'a';
        else if(s.s.in == TRUE && s.s.te == FALSE && s.s.tu == TRUE && s.s.eb == TRUE)
            s.c = 'b';
        else if(s.s.in == TRUE && s.s.te == TRUE && s.s.tu == FALSE && s.s.eb == FALSE)
            s.c = 'c';
        else if(s.s.in == TRUE && s.s.te == TRUE && s.s.tu == FALSE && s.s.eb == TRUE)
            s.c = 'd';
        else if(s.s.in == TRUE && s.s.te == TRUE && s.s.tu == TRUE && s.s.eb == FALSE)
            s.c = 'e';
        else if(s.s.in == TRUE && s.s.te == TRUE && s.s.tu == TRUE && s.s.eb == TRUE)
            s.c = 'f';
    }

    //place tunneling monsters anywhere in the dungeon
    if(s.s.tu == TRUE) {
		int t = 0;
		PRT: ;
        if(x < 0 || x > dungeon->w) {
            x = (rand() % (dungeon->w-2)) + 1; //random x position
        }
        if(y < 0 || y > dungeon->h) {
            y = (rand() % (dungeon->h-2)) + 1; //random y position
        }

		if(s.c != '@' && dungeon->num_rooms > 1) {
			s.p.x = x;
		    s.p.y = y;

			bool w_pc = FALSE;
			with_pc(dungeon, &s, &w_pc);
			if(w_pc == TRUE && t < 8) {
				t++;
				goto PRT; //avoid placing the sprite in the same room as the PC
			}
		}
    }

    //place non-tunneling monsters in rooms 
    if(r > 0 || s.s.tu == FALSE) {
		int t = 0;
		PRNT: ;
        int r_id = rand() % dungeon->num_rooms; //random room ID
        x = (rand() % dungeon->r[r_id].w) + dungeon->r[r_id].top_left.x; //random x within the room
        y = (rand() % dungeon->r[r_id].h) + dungeon->r[r_id].top_left.y; //random y within the room

		if(s.c != '@' && dungeon->num_rooms > 1) {
			s.p.x = x;
		    s.p.y = y;

			bool w_pc = FALSE;
			with_pc(dungeon, &s, &w_pc);
			if(w_pc == TRUE && t < 8) {
				t++;
				goto PRNT; //avoid placing the sprite in the same room as the PC
			}
		}
    }

    s.p.x = x; //set the sprite's x position
    s.p.y = y; //set the sprite's y position
	s.t = 0; //initialize the turn counter

	return s;
}

/* function to check for win condition (any sprites other than pc are alive) */
bool check_monsters_alive(struct Dungeon * dungeon) {
	for(int i = 0; i < dungeon->ns; i++) {
		if(dungeon->ss[i].a == TRUE && i != 0) //ignore the PC (index 0)
			return TRUE; //at least one monster is alive
	}

	return FALSE; //no monsters are alive
}