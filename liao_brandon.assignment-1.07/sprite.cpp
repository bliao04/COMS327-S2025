#include "dungeon_generator.h"

/* create new sprite instance */
Sprite * initSprite() {
    return new Sprite;
}

/* initialize array of n sprites */
Sprite * initSprites(int n) {
    return new Sprite[n];
}

/* copy all attributes between sprites */
void copySprite(Sprite * to, Sprite * from) {
    to->p.x  =  from->p.x ;
    to->p.y  = from->p.y ;
    to->c    = from->c   ;
    to->s.in = from->s.in;
    to->s.te = from->s.te;
    to->s.tu = from->s.tu;
    to->s.eb = from->s.eb;
    to->s.s  = from->s.s ;
    to->t    = from->t   ;
    to->to.x = from->to.x;
    to->to.y = from->to.y;
    to->sn   = from->sn  ;
    to->pc.x = from->pc.x;
    to->pc.y = from->pc.y;
    to->a    = from->a   ;
}

/* copy sprite to array at index n */
void copyASprite(Sprite * to, int n, Sprite * from) {
    (to[n]).p.x  =  from->p.x ;
    (to[n]).p.y  = from->p.y ;
    (to[n]).c    = from->c   ;
    (to[n]).s.in = from->s.in;
    (to[n]).s.te = from->s.te;
    (to[n]).s.tu = from->s.tu;
    (to[n]).s.eb = from->s.eb;
    (to[n]).s.s  = from->s.s ;
    (to[n]).t    = from->t   ;
    (to[n]).to.x = from->to.x;
    (to[n]).to.y = from->to.y;
    (to[n]).sn   = from->sn  ;
    (to[n]).pc.x = from->pc.x;
    (to[n]).pc.y = from->pc.y;
    (to[n]).a    = from->a   ;
}

/* get sprite at index i in array */
Sprite * thisASprite(Sprite * arr, int i) {
    return arr[i].thisSprite();
}

/* return self reference */
Sprite * Sprite::thisSprite() {
    return this;
}

/* get x position from sprite */
int getSpritePX(Sprite * s) {
    return s->p.x;
}

/* get y position from sprite */
int getSpritePY(Sprite * s) {
    return s->p.y;
}

/* get display character from sprite */
char getSpriteC(Sprite * s) {
	return s->c;
}

/* get intelligence flag from sprite */
bool getSpriteSIn(Sprite * s) {
    return s->s.in;
}

/* get telepathy flag from sprite */
bool getSpriteSTe(Sprite * s) {
    return s->s.te;
}

/* get tunneling flag from sprite */
bool getSpriteSTu(Sprite * s) {
    return s->s.tu;
}

/* get erratic behavior flag from sprite */
bool getSpriteSEb(Sprite * s) {
    return s->s.eb;
}

/* get movement speed from sprite */
int getSpriteSS(Sprite * s) {
    return s->s.s;
}

/* get turn counter from sprite */
int getSpriteT(Sprite * s) {
    return s->t;
}

/* get target x position from sprite */
int getSpriteToX(Sprite * s) {
    return s->to.x;
}

/* get target y position from sprite */
int getSpriteToY(Sprite * s) {
    return s->to.y;
}

/* get sprite id number */
int getSpriteSn(Sprite * s) {
    return s->sn;
}

/* get last known pc x position */
int getSpritePcX(Sprite * s) {
    return s->pc.x;
}

/* get last known pc y position */
int getSpritePcY(Sprite * s) {
    return s->pc.y;
}

/* get alive status from sprite */
bool getSpriteA(Sprite * s) {
    return s->a;
}

/* get x position from sprite array */
int getSpriteAPX(Sprite * s, int i) {
    return s[i].p.x;
}

/* get y position from sprite array */
int getSpriteAPY(Sprite * s, int i) {
    return s[i].p.y;
}

/* get character from sprite array */
char getSpriteAC(Sprite * s, int i) {
    return s[i].c;
}

/* get intelligence flag from sprite array */
bool getSpriteASIn(Sprite * s, int i) {
    return s[i].c;
}

/* get telepathy flag from sprite array */
bool getSpriteASTe(Sprite * s, int i) {
    return s[i].s.te;
}

/* get tunneling flag from sprite array */
bool getSpriteASTu(Sprite * s, int i) {
    return s[i].s.tu;
}

/* get erratic behavior flag from sprite array */
bool getSpriteASEb(Sprite * s, int i) {
    return s[i].s.eb;
}

/* get movement speed from sprite array */
int getSpriteASS(Sprite * s, int i) {
    return s[i].s.s;
}

/* get turn counter from sprite array */
int getSpriteAT(Sprite * s, int i) {
    return s[i].t;
}

/* get target x from sprite array */
int getSpriteAToX(Sprite * s, int i) {
    return s[i].to.x;
}

/* get target y from sprite array */
int getSpriteAToY(Sprite * s, int i) {
    return s[i].to.y;
}

/* get sprite id from array */
int getSpriteASn(Sprite * s, int i) {
    return s[i].sn;
}

/* get pc x from sprite array */
int getSpriteAPcX(Sprite * s, int i) {
    return s[i].pc.x;
}

/* get pc y from sprite array */
int getSpriteAPcY(Sprite * s, int i) {
    return s[i].pc.y;
}

/* get alive status from sprite array */
bool getSpriteAA(Sprite * s, int i) {
    return s[i].a;
}

/* set x position for sprite */
void setSpritePX(Sprite * s, int n) {
    s->p.x = n;
}

/* set y position for sprite */
void setSpritePY(Sprite * s, int n) {
    s->p.y = n;
}

/* set display character for sprite */
void setSpriteC(Sprite * s, char c) {
	s->c = c;
}

/* set intelligence flag for sprite */
void setSpriteSIn(Sprite * s, bool b) {
    s->s.in = b;
}

/* set telepathy flag for sprite */
void setSpriteSTe(Sprite * s, bool b) {
    s->s.te = b;
}

/* set tunneling flag for sprite */
void setSpriteSTu(Sprite * s, bool b) {
    s->s.tu = b;
}

/* set erratic behavior flag for sprite */
void setSpriteSEb(Sprite * s, bool b) {
    s->s.eb = b;
}

/* set movement speed for sprite */
void setSpriteSS(Sprite * s, int n) {
    s->s.s = n;
}

/* set turn counter for sprite */
void setSpriteT(Sprite * s, int n) {
    s->t = n;
}

/* set target x position for sprite */
void setSpriteToX(Sprite * s, int n) {
    s->to.x = n;
}

/* set target y position for sprite */
void setSpriteToY(Sprite * s, int n) {
    s->to.y = n;
}

/* set sprite id number */
void setSpriteSn(Sprite * s, int n) {
    s->sn = n;
}

/* set last known pc x position */
void setSpritePcX(Sprite * s, int n) {
    s->pc.x = n;
}

/* set last known pc y position */
void setSpritePcY(Sprite * s, int n) {
    s->pc.y = n;
}

/* set alive status for sprite */
void setSpriteA(Sprite * s, bool b) {
    s->a = b;
}

/* set x position in sprite array */
void setSpriteAPX(Sprite * s, int i, int n) {
    s[i].p.x = n;
}

/* set y position in sprite array */
void setSpriteAPY(Sprite * s, int i, int n) {
    s[i].p.y = n;
}

/* set character in sprite array */
void setSpriteAC(Sprite * s, int i, char c) {
    s[i].c = c;
}

/* set intelligence flag in sprite array */
void setSpriteASIn(Sprite * s, int i, bool b) {
    s[i].s.in = b;
}

/* set telepathy flag in sprite array */
void setSpriteASTe(Sprite * s, int i, bool b) {
    s[i].s.te = b;
}

/* set tunneling flag in sprite array */
void setSpriteASTu(Sprite * s, int i, bool b) {
    s[i].s.tu = b;
}

/* set erratic behavior flag in sprite array */
void setSpriteASEb(Sprite * s, int i, bool b) {
    s[i].s.eb = b;
}

/* set movement speed in sprite array */
void setSpriteASS(Sprite * s, int i, int n) {
    s[i].s.s = n;
}

/* set turn counter in sprite array */
void setSpriteAT(Sprite * s, int i, int n) {
    s[i].t = n;
}

/* set target x in sprite array */
void setSpriteAToX(Sprite * s, int i, int n) {
    s[i].to.x = n;
}

/* set target y in sprite array */
void setSpriteAToY(Sprite * s, int i, int n) {
    s[i].to.y = n;
}

/* set sprite id in array */
void setSpriteASn(Sprite * s, int i, int n) {
    s[i].sn = n;
}

/* set pc x in sprite array */
void setSpriteAPcX(Sprite * s, int i, int n) {
    s[i].pc.x = n;
}

/* set pc y in sprite array */
void setSpriteAPcY(Sprite * s, int i, int n) {
    s[i].pc.y = n;
}

/* set alive status in sprite array */
void setSpriteAA(Sprite * s, int i, bool b) {
    s[i].a = b;
}

/* get x coordinate from position */
int getPosX(Position * p) {
    return p->x;
}

/* get y coordinate from position */
int getPosY(Position * p) {
    return p->y;
}

/* set x coordinate in position */
void setPosX(Position * p, int n) {
    p->x = n;
}

/* set y coordinate in position */
void setPosY(Position * p, int n) {
    p->y = n;
}

/* initialize new position object */
Position * initPos(void) {
	return new Position;
}