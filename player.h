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

double eval_win_rate(const uint8_t *card_pri, const uint8_t *card_pub, const uint8_t round);
double eval_win_rate(Game *game, MatchState *state);
ProbAct eval_strategy(Game *game, MatchState *state, double win_rate);

#endif // _PLAYER_H_