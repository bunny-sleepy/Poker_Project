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

// evaluates opponent's win rate
double eval_dist_oppo(double prob_oppo[52][52], Game *game, MatchState *state);

// evaluates uniform win rate
double eval_win_rate_uniform(Game *game, MatchState *state);

// evaluate win rate with opponent's action
double eval_win_rate_with_belief(double prob_oppo[52][52], Game *game, MatchState *state);

// generate strategy
ProbAct eval_strategy(Game *game, MatchState *state, double win_rate, rng_state_t *rng);

#endif // _PLAYER_H_