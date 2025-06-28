#include "dungeon_generator.h"
#include <stdlib.h>

std::set<std::string> ObjFac::created_artifacts; //tracks all artifacts that have been generated
std::set<std::string> ObjFac::obtained_artifacts; //tracks artifacts obtained by player

ObjFac::ObjFac(void) {
	id = NULL; //initialize item definitions to null
	dn = 0; //initialize definition count to 0
}

ObjFac::ObjFac(int n, ItemTemp * defs) {
	id = defs; //set item definitions
	dn = n; //set definition count
}

Item ObjFac::GetObj() {
    int attempts = 0;
    while (attempts++ < 1000) {
        //pick random item
        int n = rand() % dn;
        ItemTemp it = id[n];
        
        //check artifact status
        if (it.art && 
           (created_artifacts.count(it.n) || obtained_artifacts.count(it.n))) 
            continue;

        //rarity check
        if ((rand() % 100) >= it.rrty) continue;

        //create item
        Item ni;
        if (it.art) created_artifacts.insert(it.n);

        Position p0;
        p0.x = 0;
        p0.y = 0;
        
        ni.n 	= it.n;
        ni.desc = it.desc;
        ni.dl	= it.dl;
        ni.t	= it.t;
        ni.e	= it.e;
        ni.c	= it.c;
        ni.d	= it.d;
        ni.hib	= it.hib->roll();
        ni.dob	= it.dob->roll();
        ni.deb	= it.deb->roll();
        ni.w	= it.w->roll();
        ni.spb	= it.spb->roll();
        ni.sa	= it.sa->roll();
        ni.v	= it.v->roll();
        ni.s	= it.s;
        ni.p 	= p0;
        
        return ni;

    }
    return Item();
}

void ObjFac::MarkArtifactObtained(const std::string& name) {
    obtained_artifacts.insert(name); //mark artifact as obtained by player
}