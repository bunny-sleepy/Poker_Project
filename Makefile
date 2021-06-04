CXX = g++

PROGRAMS = dealer example_player player test

all: $(PROGRAMS)

clean:
	rm -f $(PROGRAMS)

clean_log:
	rm -f ./logs/*

dealer: game.cpp game.h evalHandTables.h rng.cpp rng.h dealer.cpp net.cpp net.h
	$(CXX) -g -rdynamic -o $@ game.cpp rng.cpp dealer.cpp net.cpp

player: game.cpp game.h evalHandTables.h rng.cpp rng.h player.cpp player.h net.cpp net.h
	$(CXX) -g -rdynamic -o $@ game.cpp rng.cpp player.cpp net.cpp

example_player: game.cpp game.h evalHandTables.h rng.cpp rng.h example_player.cpp player.h net.cpp net.h
	$(CXX) -g -rdynamic -o $@ game.cpp rng.cpp example_player.cpp net.cpp

test: game.cpp game.h evalHandTables.h rng.cpp rng.h test.cpp player.h net.cpp net.h
	$(CXX) -g -rdynamic -o $@ game.cpp rng.cpp test.cpp net.cpp