#include "dungeon_generator.h"
#include <stdlib.h>

/* function to randomly place an item in a dungeon room */
void placeitem(Dungeon * dungeon, Item it) {
    //select a random room from the dungeon
	int r_id = rand() % dungeon->num_rooms;

    //calculate random position within the selected room
	it.p.x = (rand() % dungeon->r[r_id].w) + getPosX(dungeon->r[r_id].top_left);
	it.p.y = (rand() % dungeon->r[r_id].h) + getPosY(dungeon->r[r_id].top_left);

    //check if there's space for more items in the dungeon
	if(dungeon->nit < dungeon->ms) {
        //add the item to the dungeon's item array
		dungeon->items[dungeon->nit] = it;
		
        //increment the item count
		dungeon->nit++;
	}
}