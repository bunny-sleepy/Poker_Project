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

// update belief
void updateBelief(double prob_oppo[52][52], Game *game, MatchState *state, rng_state_t *rng);

// evaluate win rate with hand
double evalWinRateWithHand(Game *game, MatchState *state, uint8_t hand[2], rng_state_t *rng, size_t times_mc);

// TODO: check eval win rate
double evalWinRateUniform(Game *game, MatchState *state) {
  double pre_flop_win_rate[13][13] = {{0.85, 0.68, 0.67, 0.66, 0.66, 0.64, 0.63, 0.63, 0.62, 0.62, 0.61, 0.60, 0.59},
                                    {0.66, 0.83, 0.64, 0.64, 0.63, 0.61, 0.60, 0.59, 0.58, 0.58, 0.57, 0.56, 0.55},
                                    {0.65, 0.62, 0.80, 0.61, 0.61, 0.59, 0.58, 0.56, 0.55, 0.55, 0.54, 0.53, 0.52},
                                    {0.65, 0.62, 0.59, 0.78, 0.59, 0.57, 0.56, 0.54, 0.53, 0.52, 0.51, 0.50, 0.50},
                                    {0.64, 0.61, 0.59, 0.57, 0.75, 0.56, 0.54, 0.53, 0.51, 0.49, 0.49, 0.48, 0.47},
                                    {0.62, 0.59, 0.57, 0.55, 0.53, 0.72, 0.53, 0.51, 0.50, 0.48, 0.46, 0.46, 0.45},
                                    {0.61, 0.58, 0.55, 0.53, 0.52, 0.50, 0.69, 0.50, 0.49, 0.47, 0.45, 0.43, 0.43},
                                    {0.60, 0.57, 0.54, 0.52, 0.50, 0.48, 0.47, 0.67, 0.48, 0.46, 0.45, 0.43, 0.41},
                                    {0.59, 0.56, 0.53, 0.50, 0.48, 0.47, 0.46, 0.45, 0.64, 0.46, 0.44, 0.42, 0.40},
                                    {0.60, 0.55, 0.52, 0.49, 0.47, 0.45, 0.44, 0.43, 0.43, 0.61, 0.44, 0.43, 0.41},
                                    {0.59, 0.54, 0.51, 0.48, 0.46, 0.43, 0.42, 0.41, 0.41, 0.41, 0.58, 0.42, 0.40},
                                    {0.58, 0.54, 0.50, 0.48, 0.45, 0.43, 0.40, 0.39, 0.39, 0.39, 0.38, 0.55, 0.39},
                                    {0.57, 0.53, 0.49, 0.47, 0.44, 0.42, 0.40, 0.37, 0.37, 0.37, 0.36, 0.35, 0.51}};
  const uint8_t num_pub = (state->state.round > 0) ? (state->state.round + 2) : 0; // number of public cards
  const uint8_t ID = state->viewingPlayer;
  bool card_use_flag[52] = {0};
  // the total number of guesses and wins
  long long unsigned tot = 0, win = 0;
  uint8_t curr_card_num = num_pub;
  // init current info to the cardsets
  uint8_t pri1 = state->state.holeCards[ID][0], pri2 = state->state.holeCards[ID][1];
  card_use_flag[pri1] = 1;
  card_use_flag[pri2] = 1;
  
  // pre-flop
  if (state->state.round == 0) {
    if (pri1 / 4 == pri2 / 4) {
      return pre_flop_win_rate[12 - pri1 / 4][12 - pri1 / 4];
    } else if (pri1 % 4 == pri2 % 4) {
      if (pri1 / 4 < pri2 / 4) {
        return pre_flop_win_rate[12 - pri2 / 4][12 - pri1 / 4];
      } else {
        return pre_flop_win_rate[12 - pri1 / 4][12 - pri2 / 4];
      }
    } else {
      if (pri1 / 4 > pri2 / 4) {
        return pre_flop_win_rate[12 - pri2 / 4][12 - pri1 / 4];
      } else {
        return pre_flop_win_rate[12 - pri1 / 4][12 - pri2 / 4];
      }
    }
  }
  
  // not pre-flop
  for (uint8_t i = 0; i < num_pub; ++i) {
    card_use_flag[state->state.boardCards[i]] = 1;
  }
  // make guesses
  for (uint8_t oppo1 = 0; oppo1 < 52; ++oppo1) {
    if (card_use_flag[oppo1]) continue;
    card_use_flag[oppo1] = 1;
    for (uint8_t oppo2 = 0; oppo2 < 52; ++oppo2) {
      if (card_use_flag[oppo2]) continue;
      card_use_flag[oppo2] = 1;
      // dfs for the remaining public cards
      if (state->state.round <= 2) for (uint8_t pub1 = 0; pub1 < 52; ++pub1) {
        if (card_use_flag[pub1]) continue;
        card_use_flag[pub1] = 1;
        if (state->state.round <= 1) for (uint8_t pub2 = 0; pub2 < 52; ++pub2) {
          if (card_use_flag[pub2]) continue;
          tot++;
          card_use_flag[pub2] = 1;
          // init current info to the cardsets
          Cardset cardset_pri = emptyCardset(), cardset_oppo = emptyCardset();
          addCardToCardset(&cardset_pri, pri1 % 4, pri1 / 4);
          addCardToCardset(&cardset_pri, pri2 % 4, pri2 / 4);
          for (uint8_t i = 0; i < num_pub; ++i) {
            addCardToCardset(&cardset_pri, state->state.boardCards[i] % 4, state->state.boardCards[i] / 4);
            addCardToCardset(&cardset_oppo, state->state.boardCards[i] % 4, state->state.boardCards[i] / 4);
          }
          addCardToCardset(&cardset_pri, pub1 % 4, pub1 / 4);
          addCardToCardset(&cardset_pri, pub2 % 4, pub2 / 4);
          addCardToCardset(&cardset_oppo, pub1 % 4, pub1 / 4);
          addCardToCardset(&cardset_oppo, pub2 % 4, pub2 / 4);
          addCardToCardset(&cardset_oppo, oppo1 % 4, oppo1 / 4);
          addCardToCardset(&cardset_oppo, oppo2 % 4, oppo2 / 4);
          // TODO: check win
          win += (rankCardset(cardset_oppo) <= rankCardset(cardset_pri)) ? 1 : 0;

          card_use_flag[pub2] = 0;
        } else {
          tot++;
          // init current info to the cardsets
          Cardset cardset_pri = emptyCardset(), cardset_oppo = emptyCardset();
          addCardToCardset(&cardset_pri, pri1 % 4, pri1 / 4);
          addCardToCardset(&cardset_pri, pri2 % 4, pri2 / 4);
          for (uint8_t i = 0; i < num_pub; ++i) {
            addCardToCardset(&cardset_pri, state->state.boardCards[i] % 4, state->state.boardCards[i] / 4);
            addCardToCardset(&cardset_oppo, state->state.boardCards[i] % 4, state->state.boardCards[i] / 4);
          }
          addCardToCardset(&cardset_pri, pub1 % 4, pub1 / 4);
          addCardToCardset(&cardset_oppo, pub1 % 4, pub1 / 4);
          addCardToCardset(&cardset_oppo, oppo1 % 4, oppo1 / 4);
          addCardToCardset(&cardset_oppo, oppo2 % 4, oppo2 / 4);
          // TODO: check win
          win += (rankCardset(cardset_oppo) <= rankCardset(cardset_pri)) ? 1 : 0;
        }
        card_use_flag[pub1] = 0;
      } else {
        tot++;
        // init current info to the cardsets
        Cardset cardset_pri = emptyCardset(), cardset_oppo = emptyCardset();
        addCardToCardset(&cardset_pri, pri1 % 4, pri1 / 4);
        addCardToCardset(&cardset_pri, pri2 % 4, pri2 / 4);
        for (uint8_t i = 0; i < num_pub; ++i) {
          addCardToCardset(&cardset_pri, state->state.boardCards[i] % 4, state->state.boardCards[i] / 4);
          addCardToCardset(&cardset_oppo, state->state.boardCards[i] % 4, state->state.boardCards[i] / 4);
        }
        addCardToCardset(&cardset_oppo, oppo1 % 4, oppo1 / 4);
        addCardToCardset(&cardset_oppo, oppo2 % 4, oppo2 / 4);
        // TODO: check win
        win += (rankCardset(cardset_oppo) <= rankCardset(cardset_pri)) ? 1 : 0;
      }
      card_use_flag[oppo2] = 0;
    }
    card_use_flag[oppo1] = 0;
  }
  return (tot == 0.0) ? 0.0 : double(win) / double(tot);
}

// evaluate win rate with opponent's action
double evalWinRateWithBelief(double prob_oppo[52][52], Game *game, MatchState *state, rng_state_t *rng, size_t times_mc);

// generate strategy
ProbAct evalStrategy(Game *game, MatchState *state, double win_rate, rng_state_t *rng);

// compare 2 cardsets
// return 0 if equal, 1 if pri > oppo, -1 if pri < oppo
int compareHands(uint8_t pri[2], uint8_t oppo[2], uint8_t pub[5]);

#endif // _PLAYER_H_