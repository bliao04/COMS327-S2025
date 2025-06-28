#include <stdio.h>
#include <stdlib.h>
#include "dungeon_generator.h"
#include "heap.h"

/* helper function to test if a sprite can move to a location */
Bool test_loc(Dungeon * dungeon, int x, int y, Sprite *s) {
    //ensure location is within dungeon bounds and passable
	if(x > 0 && x < dungeon->w-1 && y > 0 && y < dungeon->h-1) {
		int hard = dungeon->d[y][x].hardness;

        //check if location is passable based on sprite type
		if(dungeon->d[y][x].hardness < 255) {
            //non-tunneling sprites can't move through hard terrain
			if(getSpriteSTu(s) == FALSE && hard > 0)
				return FALSE;
			return TRUE;
		}
	}
	return FALSE;
}

/* helper function to check if a sprite is in the same room as PC */
void with_pc(Dungeon * dungeon, Sprite * s, Bool *in) {
	int pc_rid = -1;
	int s_rid = -1;
	Sprite *pc = thisASprite(dungeon->ss, dungeon->pcSS);

    //find which rooms contain PC and sprite
	for(int i = 0; i < dungeon->num_rooms; i++) {
		if(getSpritePX(s) <= getPosX(dungeon->r[i].bottom_right) && 
           getSpritePY(s) <= getPosY(dungeon->r[i].bottom_right) && 
           getSpritePX(s) >= getPosX(dungeon->r[i].top_left) && 
           getSpritePY(s) >= getPosY(dungeon->r[i].top_left)) {
			s_rid = i;
		}
		if(getSpritePX(pc) <= getPosX(dungeon->r[i].bottom_right) && 
           getSpritePY(pc) <= getPosY(dungeon->r[i].bottom_right) && 
           getSpritePX(pc) >= getPosX(dungeon->r[i].top_left) && 
           getSpritePY(pc) >= getPosY(dungeon->r[i].top_left)) {
			pc_rid = i;
		}
	}

    //set flag if both in same room
	if(pc_rid > 0 && s_rid > 0 && pc_rid == s_rid) {
		*in = TRUE;
	}
}

/* function to generate next move for monster using dungeon cost maps */
void generate_next_move(Dungeon * dungeon, int sn) {
    //current position and possible directions
	int sx = getSpriteAPX(dungeon->ss, sn);
	int sy = getSpriteAPY(dungeon->ss, sn);
	int xs[8] = {-1,0,1,1,1,0,-1,-1};
	int ys[8] = {-1,-1,-1,0,1,1,1,0};

	Sprite *s = thisASprite(dungeon->ss, sn);

    //update turn counter based on speed
	setSpriteAT(dungeon->ss, sn, getSpriteAT(dungeon->ss, sn) + (100 / getSpriteSS(s)));

	Position * neu = new Position;
	neu = initPos();
	setPosX(neu, -1);
	setPosY(neu, -1);

    //reduce hardness for tunneling monsters
	dungeon->d[getSpritePY(s)][getSpritePX(s)].hardness -= 85;
	if(dungeon->d[getSpritePY(s)][getSpritePX(s)].hardness < 0)
		dungeon->d[getSpritePY(s)][getSpritePX(s)].hardness = 0;

    //only process alive sprites
	if(getSpriteA(s) == TRUE) {
		Bool in_room;
		int i;
		int j;
		int eb = rand() % 2; //50% chance for erratic behavior
		
        //check all 8 possible directions
		for(i = 0; i < 8; i++) {
			int px = sx + xs[i];
			int py = sy + ys[i];

			if(px >= 0 && px < dungeon->w && py >= 0 && py < dungeon->h) {
				//handle erratic behavior chance
				if(getSpriteSEb(s) == FALSE || (getSpriteSEb(s) == TRUE && eb)) {
					//non-telepathic pathfinding
					if(getSpriteSTe(s) == FALSE) {
						with_pc(dungeon, s, &in_room);
						if(in_room == TRUE) {
							setSpritePcX(s, getSpriteAPX(dungeon->ss, dungeon->pcSS));
							setSpritePcY(s, getSpriteAPY(dungeon->ss, dungeon->pcSS));

							IN: ;
							in_room = TRUE;
							//intelligent monster path selection
							if(getSpriteSIn(s) == TRUE) {
								int k;
								int lowest = 0;
								Bool set = FALSE;
								//tunneling vs non-tunneling path cost checks
								if(getSpriteSTu(s)) {
									for(k = 0; k < 8; k++) {
										if(xs[k]+sx >= 0 && xs[k]+sx < dungeon->w && 
                                           ys[k]+sy >= 0 && ys[k]+sy < dungeon->h) {
											if(dungeon->d[ys[k]+sy][xs[k]+sx].hardness < 255 && 
                                               dungeon->cst[ys[k]+sy][xs[k]+sx] < dungeon->cst[ys[lowest]+sy][xs[lowest]+sx] && 
                                               test_loc(dungeon, xs[k]+sx, ys[k]+sy, s) == TRUE && 
                                               test_loc(dungeon, xs[lowest]+sx, ys[lowest]+sy, s) == TRUE) {
												lowest = k;
												set = TRUE;
											}
										}
									}
								} else {
									//non-tunneling path validation
									for(k = 0; k < 8; k++) {
										px = xs[k]+sx;
										py = ys[k]+sy;
										if(px >= 0 && px < dungeon->w && py >= 0 && py < dungeon->h) {
											if(dungeon->d[py][px].hardness == 0 && 
                                               dungeon->csnt[py][px] <= dungeon->csnt[ys[lowest]+sy][xs[lowest]+sx]) {
												lowest = k;
												set = TRUE;
											}
										}
									}
								}
								//set new position if valid
								if(set == TRUE) {
									setPosX(neu, xs[lowest] + sx);
									setPosY(neu, ys[lowest] + sy);
									break;
								} else {
									setPosX(neu, sx);
									setPosY(neu, sy);
									break;
								}

							} else {
								//non-intelligent direct movement
								if(getSpritePcX(s) < sx)
									px = sx - 1;
								if(getSpritePcX(s) > sx)
									px = sx + 1;
								if(getSpritePcY(s) < sy)
									py = sy - 1;
								if(getSpritePcY(s) > sy)
									py = sy + 1;

								if(test_loc(dungeon, px, py, s) == TRUE) {
									setPosX(neu, px);
									setPosY(neu, py);
									break;
								}
							}
						} else {
							goto PCEB;
						}
					} else {
						//telepathic direct tracking
						setSpritePcX(s, getSpriteAPX(dungeon->ss, dungeon->pcSS));
						setSpritePcY(s, getSpriteAPY(dungeon->ss, dungeon->pcSS));
						goto IN;
					}
				} else {
					//erratic behavior handler
					PCEB: ;
					j = 0;
					EB: ;
					int c = rand() % 9;
					px = xs[c] + sx;
					py = ys[c] + sy;
					if(test_loc(dungeon, px, py, s) == FALSE && j < 8) {
						j++;
						goto EB;
					}
					if(test_loc(dungeon, px, py, s) == TRUE) {
						setPosX(neu, px);
						setPosY(neu, py);
					}
					break;
				}
			}
		}
	}

	//default to current position if no valid move
	if(getPosX(neu) < 0)
		setPosX(neu, sx);
	if(getPosY(neu) < 0)
		setPosY(neu, sy);

    //update target position
	setSpriteAToX(dungeon->ss, sn, getPosX(neu));
	setSpriteAToY(dungeon->ss, sn, getPosY(neu));

    //check for PC attack
	if(getPosX(neu) == getSpriteAPX(dungeon->ss, dungeon->pcSS) && 
       getPosY(neu) == getSpriteAPY(dungeon->ss, dungeon->pcSS))
		dungeon->go = TRUE;

    //handle sprite collisions
	for(int i = 0; i < dungeon->ns; i++) {
		if(i != sn) {
			if(getSpriteAToX(dungeon->ss, i) == getSpriteAToX(dungeon->ss, sn) && 
               getSpriteAToY(dungeon->ss, i) == getSpriteAToY(dungeon->ss, sn) && 
               getSpriteAA(dungeon->ss, sn) == TRUE) {
				setSpriteAA(dungeon->ss, i, FALSE);
				if(thisASprite(dungeon->ss, i)->s.uq) {
					dungeon->current_uniques.erase(s->n);
					MonFac::MarkUniqueDead(thisASprite(dungeon->ss, i)->n);
				}
			}
		}
	}
}

/* function to commit generated move to sprite position */
void parse_move(Dungeon * dungeon, int sn) {
	setSpriteAPX(dungeon->ss, sn, getSpriteAToX(dungeon->ss, sn));
	setSpriteAPY(dungeon->ss, sn, getSpriteAToY(dungeon->ss, sn));
}

/* function to add sprite to dungeon with dynamic array expansion */
void add_sprite(Dungeon * dungeon, Sprite * s) {
    //expand sprite array if needed
    if (dungeon->ns >= dungeon->ms) {
        int new_capacity = dungeon->ms == 0 ? 4 : dungeon->ms * 2;
        Sprite *new_ss = new Sprite[new_capacity];  
        
        //copy existing sprites
        for (int i = 0; i < dungeon->ns; i++) {
            new_ss[i] = dungeon->ss[i];
        }
        
        //free old memory if it exists
        if (dungeon->ss) {
            delete[] dungeon->ss;
        }
        
        dungeon->ss = new_ss;
        dungeon->ms = new_capacity;
    }
    
    //add sprite and update indices
    copyASprite(dungeon->ss, dungeon->ns, s);
    dungeon->ns++;
    
    //mark PC index
    if (getSpriteC(s) == '@') {
        dungeon->pcSS = dungeon->ns - 1;
    }
}

/* generate new sprite with randomized attributes */
Sprite * generate_sprite(Dungeon * dungeon, char c, int x, int y, int r) {
	Sprite * s = initSprite();

	setSpriteC(s, c);
	setSpriteA(s, TRUE);

    //pc initialization
    if(getSpriteC(s) == '@') {
        PC * p = initPC(dungeon);
		return (Sprite *) p;
	} else {
        //randomize monster attributes
        setSpriteSS(s, (rand() % 16) + 5); //speed 5-20
        setSpriteSIn(s, rand() % 2); //intelligence
        setSpriteSTe(s, rand() % 2); //telepathy
        setSpriteSTu(s, rand() % 2); //tunneling
        setSpriteSEb(s, rand() % 2); //erratic behavior

        //map attributes to display character
        if(getSpriteSIn(s) == FALSE && getSpriteSTe(s) == FALSE && getSpriteSTu(s) == FALSE && getSpriteSEb(s) == FALSE)
            setSpriteC(s, '0');
        else if(getSpriteSIn(s) == FALSE && getSpriteSTe(s) == FALSE && getSpriteSTu(s) == FALSE && getSpriteSEb(s) == TRUE)
            setSpriteC(s, '1');
        else if(getSpriteSIn(s) == FALSE && getSpriteSTe(s) == FALSE && getSpriteSTu(s) == TRUE && getSpriteSEb(s) == FALSE)
            setSpriteC(s, '2');
        else if(getSpriteSIn(s) == FALSE && getSpriteSTe(s) == FALSE && getSpriteSTu(s) == TRUE && getSpriteSEb(s) == TRUE)
            setSpriteC(s, '3');
        else if(getSpriteSIn(s) == FALSE && getSpriteSTe(s) == TRUE && getSpriteSTu(s) == FALSE && getSpriteSEb(s) == FALSE)
            setSpriteC(s, '4');
        else if(getSpriteSIn(s) == FALSE && getSpriteSTe(s) == TRUE && getSpriteSTu(s) == FALSE && getSpriteSEb(s) == TRUE)
            setSpriteC(s, '5');
        else if(getSpriteSIn(s) == FALSE && getSpriteSTe(s) == TRUE && getSpriteSTu(s) == TRUE && getSpriteSEb(s) == FALSE)
            setSpriteC(s, '6');
        else if(getSpriteSIn(s) == FALSE && getSpriteSTe(s) == TRUE && getSpriteSTu(s) == TRUE && getSpriteSEb(s) == TRUE)
            setSpriteC(s, '7');
        else if(getSpriteSIn(s) == TRUE && getSpriteSTe(s) == FALSE && getSpriteSTu(s) == FALSE && getSpriteSEb(s) == FALSE)
            setSpriteC(s, '8');
        else if(getSpriteSIn(s) == TRUE && getSpriteSTe(s) == FALSE && getSpriteSTu(s) == FALSE && getSpriteSEb(s) == TRUE)
            setSpriteC(s, '9');
        else if(getSpriteSIn(s) == TRUE && getSpriteSTe(s) == FALSE && getSpriteSTu(s) == TRUE && getSpriteSEb(s) == FALSE)
            setSpriteC(s, 'a');
        else if(getSpriteSIn(s) == TRUE && getSpriteSTe(s) == FALSE && getSpriteSTu(s) == TRUE && getSpriteSEb(s) == TRUE)
            setSpriteC(s, 'b');
        else if(getSpriteSIn(s) == TRUE && getSpriteSTe(s) == TRUE && getSpriteSTu(s) == FALSE && getSpriteSEb(s) == FALSE)
            setSpriteC(s, 'c');
        else if(getSpriteSIn(s) == TRUE && getSpriteSTe(s) == TRUE && getSpriteSTu(s) == FALSE && getSpriteSEb(s) == TRUE)
            setSpriteC(s, 'd');
        else if(getSpriteSIn(s) == TRUE && getSpriteSTe(s) == TRUE && getSpriteSTu(s) == TRUE && getSpriteSEb(s) == FALSE)
            setSpriteC(s, 'e');
        else if(getSpriteSIn(s) == TRUE && getSpriteSTe(s) == TRUE && getSpriteSTu(s) == TRUE && getSpriteSEb(s) == TRUE)
            setSpriteC(s, 'f');
    }

    //tunneling monster placement
    if(getSpriteSTu(s) == TRUE) {
		int t = 0;
		PRT: ;
        //random position validation
        if(x < 0 || x > dungeon->w) x = (rand() % (dungeon->w-2)) + 1;
        if(y < 0 || y > dungeon->h) y = (rand() % (dungeon->h-2)) + 1;

		if(getSpriteC(s) != '@' && dungeon->num_rooms > 1) {
			setSpritePX(s, x);
		    setSpritePY(s, y);

			Bool w_pc = FALSE;
			with_pc(dungeon, s, &w_pc);
			if(w_pc == TRUE && t < 8) {  //avoid PC's room
				t++;
				goto PRT;
			}
		}
    }

    //non-tunneling monster placement
    if(r > 0 || getSpriteSTu(s) == FALSE) {
		int t = 0;
		PRNT: ;
        int r_id = rand() % dungeon->num_rooms;
        x = (rand() % dungeon->r[r_id].w) + getPosX(dungeon->r[r_id].top_left);
        y = (rand() % dungeon->r[r_id].h) + getPosY(dungeon->r[r_id].top_left);

		if(getSpriteC(s) != '@' && dungeon->num_rooms > 1) {
			setSpritePX(s, x);
		    setSpritePY(s, y);

			Bool w_pc = FALSE;
			with_pc(dungeon, s, &w_pc);
			if(w_pc == TRUE && t < 8) {  //avoid PC's room
				t++;
				goto PRNT;
			}
		}
    }

    //final position assignment
    setSpritePX(s, x);
    setSpritePY(s, y);
	setSpriteToX(s, x);
	setSpriteToY(s, y);
	setSpriteT(s, 0);

	return s;
}

/* function to generate a sprite from a factory */
Sprite * generate_sprite_fac(Dungeon * dungeon, char c, int x, int y, int r) {
	Sprite* s = new Sprite;
	
    //if not generating player character (@)
	if(c != '@'){
	    //get monster from monster factory
		s = dungeon->mf->GetMon(dungeon);
		
        //if random placement or monster is tunnel-capable
		if(r > 0 || s->s.tu == true) {
			int t = 0;
			RP: ;
            //pick random room
        	int r_id = rand() % dungeon->num_rooms;
            //calculate random position within room
			x = (rand() % dungeon->r[r_id].w) + getPosX(dungeon->r[r_id].top_left);
			y = (rand() % dungeon->r[r_id].h) + getPosY(dungeon->r[r_id].top_left);

            //if not PC and multiple rooms exist
			if(getSpriteC(s) != '@' && dungeon->num_rooms > 1) {
				setSpritePX(s, x);
				setSpritePY(s, y);

                //check if spawned with PC
				Bool w_pc = FALSE;
				with_pc(dungeon, s, &w_pc);
                //retry if too close to PC (max 8 attempts)
				if(w_pc == TRUE && t < 8) {
					t++;
					goto RP;
				}
			}
		} else {
            //handle out-of-bounds coordinates for non-random placement
			if(x < 0 || x > dungeon->w) {
				x = (rand() % (dungeon->w-2)) + 1;
        	}
			if(y < 0 || y > dungeon->h) {
				y = (rand() % (dungeon->h-2)) + 1;
			}
		}
	} else {
        //generate player character
		s = (Sprite *)initPC(dungeon);
	}
		
	return s;
}

/* function to check win condition, any alive monsters */
Bool check_monsters_alive(Dungeon * dungeon) {
	for(int i = 0; i < dungeon->ns; i++) {
		if(getSpriteAA(dungeon->ss, i) == TRUE && i != 0)
			return TRUE;
	}
	return FALSE;
}