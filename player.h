#ifndef _PLAYER_H_
#define _PLAYER_H_

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <getopt.h>
#include <cmath>
#include "game.h"
#include "rng.h"
#include "net.h"
#include "evalHandTables.h"

typedef struct {
  double prob_fold = 0.0;
  double prob_call = 0.0;
  double prob_raise = 0.0;
  double raise_size;
} ProbAct;

// updates belief
double updateBelief(double prob_oppo[52][52], Game *game, MatchState *state);

// evaluates uniform win rate
double evalWinRateUniform(Game *game, MatchState *state);

// evaluate win rate with opponent's action
double evalWinRateWithBelief(double prob_oppo[52][52], Game *game, MatchState *state, rng_state_t *rng, size_t times_mc);

// generate strategy
ProbAct evalStrategy(Game *game, MatchState *state, double win_rate, rng_state_t *rng);

// compare 2 cardsets
// return 0 if equal, 1 if pri > oppo, -1 if pri < oppo
int compareHands(uint8_t pri[2], uint8_t oppo[2], uint8_t pub[5]);

#endif // _PLAYER_H_