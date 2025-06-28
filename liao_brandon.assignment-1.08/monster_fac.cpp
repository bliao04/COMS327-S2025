#include "dungeon_generator.h"
#include <stdlib.h>
#include <stdio.h>

std::set<std::string> MonFac::killed_uniques;

MonFac::MonFac(void) {
	md = NULL; //initialize monster definitions to null
	dn = 0; //initialize definition count to 0
}

MonFac::MonFac(int n, SpriteTemp * defs) {
	md = defs; //set monster definitions
	dn = n; //set definition count
}

Sprite* MonFac::GetMon(Dungeon *dungeon) {
    int attempts = 0;
    while (attempts++ < 1000) {
        //pick random monster
        int n = rand() % dn; 
        SpriteTemp s = md[n];
        
        //unique checks
        if (s.s.uq) {
            //global death check
            if (killed_uniques.count(s.n)) continue;
            
            //current dungeon existence check
            if (dungeon->current_uniques.count(s.n)) continue;
        }

        //rarity check
        if ((rand() % 100) >= s.s.rrty) continue;

        //create monster
        Sprite* ns = new Sprite();
        if (s.s.uq) dungeon->current_uniques.insert(s.n);
        Position p0;
        p0.x = 0;
        p0.y = 0;
        Position pn1;
        pn1.x = -1;
        pn1.y = -1;
        
        ns->p = p0;
        ns->c = s.c;
            
        ns->s.in = s.s.in;
        ns->s.te = s.s.te;
        ns->s.tu = s.s.tu;
        ns->s.eb = s.s.eb;
        ns->s.pa = s.s.pa;
        ns->s.s = s.s.s->roll();
        ns->s.a = s.s.a;
        ns->s.hp = s.s.hp->roll();
        ns->s.uq = s.s.uq;
        ns->s.boss = s.s.boss;
        ns->s.rrty = s.s.rrty;
        
        ns->t = 0;
        ns->to = p0;
        ns->sn = -1;
        ns->pc = pn1;
        ns->a = true;
        ns->n = s.n;
        ns->color = s.color;
        ns->desc = s.desc;
        ns->dl = s.dl;
        
        return ns;
    }
	
	return nullptr;
}

void MonFac::MarkUniqueDead(const std::string& name) {
    killed_uniques.insert(name); //mark unique monster as killed globally
}