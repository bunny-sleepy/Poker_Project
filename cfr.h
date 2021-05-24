#ifndef _CFR_H_
#define _CFR_H_
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
#include <iostream>
#include <cmath>
#include "game.h"
#include "rng.h"
#include "net.h"
#include "evalHandTables.h"

double externalSamplingCFR(Game *game, MatchState *state, uint_8 player_idx);
void train(const size_t iterations);
void writeStrategy(const size_t iterations);

#endif // _CFR_H_