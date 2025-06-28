#include "dungeon_generator.h"
#include <cstdio>
#include <stdlib.h>
#include <string>
#include <sstream>

/* function to roll a Dice and return the total result */
int	Dice::roll(void) {
	if(d == 0)
		//if die value is 0, treat it as no dice to roll
		return b;
	else
		//roll 'n' dice each with 'd' sides, generate random number for each die (1 to d), and add base value
		return b + n * ((rand() % d) + 1);
}

/* function that returns a string representation of the dice configuration */
std::string Dice::string(void) {
    return std::to_string(b) + "+" + std::to_string(n) + "d" + std::to_string(d);
}

/* constructor, initializes the Dice object with a base value, number of dice, and die value (sides per die) */
Dice::Dice(int base, int num, int die) {
	b = base;
	n = num;
	d = die;
}

/* constructor, initializes the Dice object with 0 as base values */
Dice::Dice(void) {
	b = 0;
	n = 0;
	d = 0;
}