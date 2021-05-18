CXX = g++

PROGRAMS = dealer example_player

all: $(PROGRAMS)

clean:
	rm -f $(PROGRAMS)

clean_log:
	rm -f ./logs/*

dealer: game.cpp game.h evalHandTables.h rng.cpp rng.h dealer.cpp net.cpp net.h
	$(CXX) -o $@ game.cpp rng.cpp dealer.cpp net.cpp

example_player: game.cpp game.h evalHandTables.h rng.cpp rng.h example_player.cpp net.cpp net.h
	$(CXX) -o $@ game.cpp rng.cpp example_player.cpp net.cpp
