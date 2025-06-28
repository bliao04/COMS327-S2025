all: dungeon_generator

dungeon_generator: dungeon_generator.cpp heap.cpp monsters.cpp dungeon.cpp print.cpp sprite.cpp monster.cpp pc.cpp dice.cpp
	g++ -Wall -Werror -ggdb dungeon_generator.cpp heap.cpp monsters.cpp dungeon.cpp print.cpp sprite.cpp monster.cpp pc.cpp dice.cpp -lm -lncurses -o dungeon_generator

gpp: sprite.cpp monster.cpp pc.cpp
	g++ -c -Wall -Werror -ggdb sprite.cpp monster.cpp pc.cpp

clean:
	rm dungeon_generator