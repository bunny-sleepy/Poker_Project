#include "player.h"

/* TODO: implement your own poker strategy in this function!
 * 
 * The function is called when it is the agent's turn to play!
 * All the information about the game and the current state is preprocessed
 * and stored in game and state.
 * 
 * */
Action act(Game *game, MatchState *state, rng_state_t *rng) {
  Action action;

  // TODO: substitute this simple random strategy with your own strategy

  double probs[ NUM_ACTION_TYPES ];
  double actionProbs[ NUM_ACTION_TYPES ];

  // TODO: check if there is other variables to be added
  uint8_t ID = state->viewingPlayer; // my player ID
  uint8_t num_pub = 0; // number of public cards
  const uint8_t num_pri = 2; // number of private cards
  uint8_t card_pri[2]; // my cards
  // initialize card_pri
  for (size_t i = 0; i < 2; ++i) {
    card_pri[i] = state->state.holeCards[ID][i];
  }
  uint8_t card_pub[5] = {0, 0, 0, 0, 0}; // public cards, first initialize to 0
  uint8_t round = state->state.round; // current round of game

  // update public cards according to current cards
  if (round > 0) {
    for (uint8_t i = 0; i < round + 2; ++i) {
      card_pub[i] = state->state.boardCards[i];
    }
    num_pub = round + 2;
  }
  updateBelief(state->state.prob_oppo, game, state, rng);
  // the win_rate
  // double win_rate = evalWinRateUniform(game, state);
  double win_rate_with_belief = evalWinRateWithBelief(state->state.prob_oppo, game, state, rng, 200000);
  /* Define the probabilities of actions for the player */
  ProbAct prob_act = evalStrategy(game, state, win_rate_with_belief, rng);
  probs[ a_fold ] = prob_act.prob_fold;
  probs[ a_call ] = prob_act.prob_call;
  probs[ a_raise ] = prob_act.prob_raise;

  /* build the set of valid actions */
  double p = 0.0;
  int a;
  for( a = 0; a < NUM_ACTION_TYPES; ++a ) {
    actionProbs[a] = 0.0;
  }

  /* consider fold */
  action.type = a_fold;
  action.size = 0;
  if( isValidAction( game, &(state->state), 0, &action ) ) {
    actionProbs[ a_fold ] = probs[ a_fold ];
    p += probs[ a_fold ];
  }

  /* consider call */
  action.type = a_call;
  action.size = 0;
  actionProbs[ a_call ] = probs[ a_call ];
  p += probs[ a_call ];

  /* consider raise */
  int32_t min, max;
  if( raiseIsValid( game, &(state->state), &min, &max ) ) {
    actionProbs[ a_raise ] = probs[ a_raise ];
    p += probs[ a_raise ];
  }

  /* normalise the probabilities  */
  assert( p > 0.0 );
  for( a = 0; a < NUM_ACTION_TYPES; ++a ) {

    actionProbs[ a ] /= p;
  }

  /* choose one of the valid actions at random */
  p = genrand_real2( rng );
  for( a = 0; a < NUM_ACTION_TYPES - 1; ++a ) {
    if( p <= actionProbs[ a ] ) {
      break;
    }
    p -= actionProbs[ a ];
  }
  action.type = (enum ActionType)a;
  if( a == a_raise ) {
    action.size = prob_act.raise_size;
    // in case of invalid raise
    if (action.size > max) {
      action.size = max;
    } else if (action.size < min) {
      action.size = min;
    }
  }
  else {
    action.size = 0;
  }

  return action;
}

void updateBelief(double prob_oppo[52][52], Game *game, MatchState *state, rng_state_t *rng) {
  uint8_t ID = state->viewingPlayer;
  uint8_t round = state->state.round;
  uint8_t num_action = state->state.numActions[round];
  size_t raise = 0;
  unsigned int pot = 0; // current total pot value
  for (uint8_t i = 0; i < game->numPlayers; ++i) {
    pot += state->state.spent[i];
  }
  if (num_action == 0) { // I make first move in this round
    if (round == 0 || state->state.actingPlayer[round - 1][state->state.numActions[round - 1] - 1] == ID)
      raise = 0;
    else { // opponent calls last round
      if (state->state.action[round - 1][state->state.numActions[round - 1] - 2].type == a_call)
        raise = 0;
      else {
        size_t last_raise = game->blind[ID];
        raise = state->state.action[round - 1][state->state.numActions[round - 1] - 2].size;
        bool flag = false;
        for (int8_t j = state->state.numActions[round - 1] - 3; j >= 0; --j) {
          if (state->state.action[round - 1][j].type == a_raise) {
            last_raise = state->state.action[round - 1][j].size;
            flag = true;
            break;
          }
        }
        if (!flag) for (int8_t i = round - 2; i >= 0; --i) {
          if (flag) break; 
          for (int8_t j = state->state.numActions[i] - 1; j >= 0; --j) {
            if (state->state.action[i][j].type == a_raise) {
              last_raise = state->state.action[i][j].size;
              flag = true;
              break;
            }
          }
        }
        raise = raise - last_raise;
        pot -= raise;
        raise /= 2;
      }
    }
  } else if (num_action == 1) {
    if (state->state.action[round][0].type == a_call) {
      if (round == 0 || state->state.action[round - 1][state->state.numActions[round - 1] - 1].type == a_call)
        raise = 0;
      else {
        size_t last_raise;
        if (state->state.actingPlayer[round - 1][state->state.numActions[round - 1] - 1] == ID) {
          last_raise = game->blind[ID];
        } else {
          last_raise = 150 - game->blind[ID];
        }
        raise = state->state.action[round - 1][state->state.numActions[round - 1] - 1].size;
        bool flag = false;
        for (int8_t j = state->state.numActions[round - 1] - 2; j >= 0; --j) {
          if (state->state.action[round - 1][j].type == a_raise) {
            last_raise = state->state.action[round - 1][j].size;
            flag = true;
            break;
          }
        }
        if (!flag) for (int8_t i = round - 2; i >= 0; --i) {
          if (flag) break;
          for (int8_t j = state->state.numActions[i] - 1; j >= 0; --j) {
            if (state->state.action[i][j].type == a_raise) {
              last_raise = state->state.action[i][j].size;
              flag = true;
              break;
            }
          }
        }
        raise = raise - last_raise;
        pot -= raise;
        raise /= 2;
      }
    } else { // a_raise
      size_t last_raise = (game->blind[ID] == 100) ? 50 : 100;
      bool flag = false;
      if (round > 0) for (int8_t i = round - 1; i >= 0; --i) {
        if (flag) break;
        if (!flag) for (int8_t j = state->state.numActions[i] - 1; j >= 0; --j) {
          if (state->state.action[i][j].type == a_raise) {
            last_raise = state->state.action[i][j].size;
            flag = true;
            break;
          }
        }
      }
      raise = state->state.action[round][0].size - last_raise;
      pot -= raise;
    }
  } else {
    if (state->state.action[round][num_action - 1].type == a_call) {
      if (state->state.action[round][num_action - 2].type == a_call)
        raise = 0;
      else {
        raise = state->state.action[round][num_action - 2].size;
        size_t last_raise = game->blind[ID];
        bool flag = false;
        for (int8_t j = state->state.numActions[round] - 3; j >= 0; --j) {
          if (state->state.action[round][j].type == a_raise) {
            last_raise = state->state.action[round][j].size;
            flag = true;
            break;
          }
        }
        if (!flag) for (int8_t i = round - 1; i >= 0; --i) {
          if (flag) break;
          for (int8_t j = state->state.numActions[i] - 1; j >= 0; --j) {
            if (state->state.action[i][j].type == a_raise) {
              last_raise = state->state.action[i][j].size;
              flag = true;
              break;
            }
          }
        }
        raise = raise - last_raise;
        pot -= raise;
        raise /= 2;
      }
    } else { // a_raise
      size_t last_raise = (game->blind[ID] == 100) ? 50 : 100;
      bool flag = false;
      for (int8_t j = state->state.numActions[round] - 2; j >= 0; --j) {
        if (state->state.action[round][j].type == a_raise) {
          last_raise = state->state.action[round][j].size;
          flag = true;
          break;
        }
      }
      if (!flag) for (int8_t i = round - 1; i >= 0; --i) {
        if (flag) break;
        for (int8_t j = state->state.numActions[i] - 1; j >= 0; --j) {
          if (state->state.action[i][j].type == a_raise) {
            last_raise = state->state.action[i][j].size;
            flag = true;
            break;
          }
        }
      }
      raise = state->state.action[round][0].size - last_raise;
      pot -= raise;
    }
  }
  // update according to ratio, if ratio = 0, we should not modify, and prob discounts as ratio increases
  // here we choose exp(win_rate * ratio * alpha) as discount factor, then normalize
  double ratio = double(raise) / pot;
  if (ratio >= 15.0) ratio = 15.0;
  double alpha = 0.5;
  for (uint8_t i = 0; i < 52; ++i) {
    for (uint8_t j = 0; j < i; ++j) {
      uint8_t hand[2] = {i, j};
      double win_rate = evalWinRateWithHand(game, state, hand, rng, 10000);
      double discount_factor = exp(alpha * win_rate * ratio * (1 + round));
      prob_oppo[i][j] = prob_oppo[i][j] * discount_factor;
      prob_oppo[j][i] = prob_oppo[j][i] * discount_factor;
      if (prob_oppo[i][j] <= 0.0) {
        prob_oppo[i][j] = 0.0;
        prob_oppo[j][i] = 0.0;
      }
    }
  }
  double sum = 0.0;
  for (uint8_t i = 0; i < 52; ++i) {
    for (uint8_t j = 0; j < i; ++j) {
      sum = sum + prob_oppo[i][j];
    }
  }
  sum = sum * 2.0;
  for (uint8_t i = 0; i < 52; ++i) {
    for (uint8_t j = 0; j < i; ++j) {
      prob_oppo[i][j] = prob_oppo[i][j] / sum;
      prob_oppo[j][i] = prob_oppo[j][i] / sum;
      assert(prob_oppo[i][j] >= 0.0 && prob_oppo[j][i] >= 0.0);
    }
  }
}

int compareHands(uint8_t pri[2], uint8_t oppo[2], uint8_t pub[5]) {
  Cardset cardset_pri = emptyCardset(), cardset_oppo = emptyCardset();
  // add to private set
  addCardToCardset(&cardset_pri, pri[0] % 4, pri[0] / 4);
  addCardToCardset(&cardset_pri, pri[1] % 4, pri[1] / 4);
  addCardToCardset(&cardset_pri, pub[0] % 4, pub[0] / 4);
  addCardToCardset(&cardset_pri, pub[1] % 4, pub[1] / 4);
  addCardToCardset(&cardset_pri, pub[2] % 4, pub[2] / 4);
  addCardToCardset(&cardset_pri, pub[3] % 4, pub[3] / 4);
  addCardToCardset(&cardset_pri, pub[4] % 4, pub[4] / 4);
  // add to opponent's set
  addCardToCardset(&cardset_oppo, oppo[0] % 4, oppo[0] / 4);
  addCardToCardset(&cardset_oppo, oppo[1] % 4, oppo[1] / 4);
  addCardToCardset(&cardset_oppo, pub[0] % 4, pub[0] / 4);
  addCardToCardset(&cardset_oppo, pub[1] % 4, pub[1] / 4);
  addCardToCardset(&cardset_oppo, pub[2] % 4, pub[2] / 4);
  addCardToCardset(&cardset_oppo, pub[3] % 4, pub[3] / 4);
  addCardToCardset(&cardset_oppo, pub[4] % 4, pub[4] / 4);
  // compare
  int rank_pri = rankCardset(cardset_pri), rank_oppo = rankCardset(cardset_oppo);
  if (rank_pri == rank_oppo)
    return 0;
  else if (rank_pri < rank_oppo)
    return -1;
  else
    return 1;
}

// TODO: check pre_flop prob.
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

// TODO: check win rate
double evalWinRateWithHand(Game *game, MatchState *state, uint8_t hand[2], rng_state_t *rng, size_t times_mc) {
  const uint8_t num_pub = (state->state.round > 0) ? (state->state.round + 2) : 0; // number of public cards
  const uint8_t ID = state->viewingPlayer;
  // the total number of guesses and wins
  size_t win = 0;
  uint8_t curr_card_num = num_pub;
  // init current info to the cardsets
  uint8_t pri1 = hand[0], pri2 = hand[1];
  
  // pre-flop
  if (state->state.round == 0) {
    return evalWinRateUniform(game, state);
  }
  uint8_t oppo[2], pub[5];
  for (uint8_t i = 0; i < num_pub; ++i) {
    pub[i] = state->state.boardCards[i];
  }
  // Monte Carlo Sampling
  for (size_t time = 0; time < times_mc; time++) {
    bool used[52] = {0};
    used[hand[0]] = true;
    used[hand[1]] = true;
    for (uint8_t i = 0; i < num_pub; ++i) {
      used[pub[i]] = true;
    }
    // sample opponent's hand
    do {
      double rand_real = genrand_real2(rng);
      oppo[0] = uint8_t(52 * rand_real) % 52;
      rand_real = genrand_real2(rng);
      oppo[1] = uint8_t(52 * rand_real) % 52;
      if (oppo[0] == oppo[1]) continue;
    } while (used[oppo[0]] || used[oppo[1]]);

    // sample public cards
    used[oppo[0]] = true;
    used[oppo[1]] = true;
    for (uint8_t i = 0; i < 5 - num_pub; ++i) {
      size_t rand_card = (genrand_int32(rng) % 52);
      while (used[rand_card]) {
        rand_card = genrand_int32(rng) % 52;
      }
      pub[num_pub + i] = rand_card;
      used[rand_card] = true;
    }
    int result = compareHands(hand, oppo, pub);
    win += (result + 1);
  }
  return double(win) / times_mc;
}

// TODO: check correctness
double evalWinRateWithBelief(double prob_oppo[52][52], Game *game, MatchState *state, rng_state_t *rng, size_t times_mc) {
  const uint8_t num_pub = (state->state.round > 0) ? (state->state.round + 2) : 0; // number of public cards
  const uint8_t ID = state->viewingPlayer;
  // preflop
  if (num_pub == 0) {
    return evalWinRateUniform(game, state);
  }
  size_t win = 0; // tot is times_mc
  uint8_t pri[2], pub[5], oppo[2];
  // initialize cards
  pri[0] = state->state.holeCards[ID][0], pri[1] = state->state.holeCards[ID][1];
  for (uint8_t i = 0; i < num_pub; ++i) {
    pub[i] = state->state.boardCards[i];
  }
  // Monte Carlo Sampling
  for (size_t time = 0; time < times_mc; time++) {
    // sample opponent's hand
    bool used[52] = {0};
    used[pri[0]] = true;
    used[pri[1]] = true;
    for (uint8_t i = 0; i < num_pub; ++i) {
      used[pub[i]] = true;
    }
    bool flag = false;
    do {
      double rand_real = genrand_real2(rng);
      flag = false;
      for (uint8_t i = 0; i < 52; ++i) {
        if (flag) break;
        for (uint8_t j = 0; j < i; ++j) {
          double prob = prob_oppo[i][j] + prob_oppo[j][i];
          if (rand_real <= prob) {
            oppo[0] = i;
            oppo[1] = j;
            flag = true;
            break;
          }
          rand_real -= prob;
        }
      }
    } while (!flag || (used[oppo[0]] || used[oppo[1]]));

    // sample public cards
    used[oppo[0]] = true;
    used[oppo[1]] = true;
    for (uint8_t i = 0; i < 5 - num_pub; ++i) {
      size_t rand_card = (genrand_int32(rng) % 52);
      while (used[rand_card]) {
        rand_card = genrand_int32(rng) % 52;
      }
      pub[num_pub + i] = rand_card;
      used[rand_card] = true;
    }
    int result = compareHands(pri, oppo, pub);
    win += (result + 1);
  }
  return double(win) / (2 * times_mc);
}

// TODO: check strategy
ProbAct evalStrategy(Game *game, MatchState *state, double win_rate, rng_state_t *rng) {
  ProbAct prob_act;
  unsigned int pot = 0; // current total pot value
  for (uint8_t i = 0; i < game->numPlayers; ++i) {
    pot += state->state.spent[i];
  }
  // pre-flop strategy
  if (state->state.round == 0) {
    // TODO: adjust the values
    const double fold_threshold_pre_flop = 0.52;
    const double raise_threshold_pre_flop = 0.65;
    // fold if win rate too low
    if (win_rate <= fold_threshold_pre_flop) {
      Action action;
      action.type = a_fold;
      action.size = 0;
      if (isValidAction(game, &(state->state), 0, &action)) {
        // TODO: adjust according to opponent's bet
        prob_act.prob_call = 0.05;
        prob_act.prob_fold = 0.95;
        prob_act.prob_raise = 0.0;
      } else { // call if not able to fold
        prob_act.prob_call = 1.0;
        prob_act.prob_fold = 0.0;
        prob_act.prob_raise = 0.0;
      }
    } else if (win_rate > raise_threshold_pre_flop) { // place a raise if we have high win prob
      // consider raise
      int32_t min, max;
      if (raiseIsValid(game, &(state->state), &min, &max)) {
        prob_act.prob_call = 0.0;
        prob_act.prob_fold = 0.0;
        prob_act.prob_raise = 1.0;
        // choose raise value w.r.t pot and win rate at random (uniform)
        double rand_num = genrand_real2(rng);
        // TODO: adjust raise size
        prob_act.raise_size = ((min + int32_t(rand_num * pot * exp(win_rate) * 3)) < max) ? (min + int32_t(rand_num * pot * exp(win_rate) * 3)) : max;
      } else {
        prob_act.prob_call = 1.0;
        prob_act.prob_fold = 0.0;
        prob_act.prob_raise = 0.0;
      }
    } else { // intermediate values
      int32_t min, max;
      if (raiseIsValid(game, &(state->state), &min, &max)) {
        Action action;
        action.type = a_fold;
        action.size = 0;
        if (isValidAction(game, &(state->state), 0, &action)) { // fold is valid
          prob_act.prob_fold = 1 / (1 + 2 * exp(2 * win_rate));
          prob_act.prob_raise = 2 * exp(2 * win_rate) * win_rate / (1 + 2 * exp(2 * win_rate));
          prob_act.prob_call = 1.0 - prob_act.prob_fold - prob_act.prob_raise;
        } else {
          prob_act.prob_fold = 0;
          prob_act.prob_raise = 2 * exp(win_rate) * win_rate / (1 + 2 * exp(win_rate)) + 1 / (1 + 2 * exp(win_rate));
          prob_act.prob_call = 1.0 - prob_act.prob_raise;
        }
        // choose raise value w.r.t pot and win rate at random (uniform)
        double rand_num =  genrand_real2(rng);
        // TODO: adjust raise size
        prob_act.raise_size = (min + int32_t(rand_num * pot * exp(win_rate) * 2) < max) ? (min + int32_t(rand_num * pot * exp(win_rate) * 2)) : max;
      } else {
        Action action;
        action.type = a_fold;
        action.size = 0;
        if (isValidAction(game, &(state->state), 0, &action)) { // fold is valid
          prob_act.prob_call = (win_rate + 1.0) / 2;
          prob_act.prob_fold = (1.0 - win_rate) / 2;
          prob_act.prob_raise = 0.0;
        } else {
          prob_act.prob_call = 1.0;
          prob_act.prob_fold = 0.0;
          prob_act.prob_raise = 0.0;
        }
      }
    }
  } else { // after flop
    int32_t min, max;
    if (raiseIsValid(game, &(state->state), &min, &max)) {
      Action action;
      action.type = a_fold;
      action.size = 0;
      if (isValidAction(game, &(state->state), 0, &action)) { // fold is valid
        prob_act.prob_fold = 1 / (1 + 2 * exp(5 * win_rate));
        prob_act.prob_raise = 2 * exp(5 * win_rate) * win_rate / (1 + 2 * exp(5 * win_rate));
        prob_act.prob_call = 1.0 - prob_act.prob_fold - prob_act.prob_raise;
        if (win_rate < 0.42) {
          prob_act.prob_call = 0.0;
          prob_act.prob_fold = 1.0;
          prob_act.prob_raise = 0.0;
        }
      } else {
        prob_act.prob_fold = 0.0;
        prob_act.prob_raise = exp(win_rate - 2.0);
        prob_act.prob_call = 1.0 - prob_act.prob_raise;
      }
      if (win_rate >= 0.78) {
        prob_act.prob_call = 0.2;
        prob_act.prob_fold = 0.0;
        prob_act.prob_raise = 0.8;
      }
      // choose raise value w.r.t pot and win rate at random (uniform)
      double rand_num =  genrand_real2(rng);
      // TODO: adjust raise size
      prob_act.raise_size = (min + int32_t(rand_num * pot * exp(win_rate) * 3) < max) ? (min + int32_t(rand_num * pot * exp(win_rate) * 3)) : max;
    } else { // raise is not valid
      Action action;
      action.type = a_fold;
      action.size = 0;
      if (isValidAction(game, &(state->state), 0, &action)) { // fold is valid
        prob_act.prob_call = exp(win_rate - 1.0);
        prob_act.prob_fold = 1 - prob_act.prob_call;
        prob_act.prob_raise = 0.0;
        if (win_rate < 0.42) {
          prob_act.prob_call = 0.0;
          prob_act.prob_fold = 1.0;
          prob_act.prob_raise = 0.0;
        }
      } else {
        prob_act.prob_call = 1.0;
        prob_act.prob_fold = 0.0;
        prob_act.prob_raise = 0.0;
      }
    }
  }
  assert(prob_act.prob_call + prob_act.prob_fold + prob_act.prob_raise - 1.0 > -1e-10);
  assert(prob_act.prob_call + prob_act.prob_fold + prob_act.prob_raise - 1.0 < 1e-10);
  return prob_act;
}

/* Anything related with socket is handled below. */
/* If you are not interested with protocol details, you can safely skip these. */

int step(int len, char line[], Game *game, MatchState *state, rng_state_t *rng) {
  /* add a colon (guaranteed to fit because we read a new-line in fgets) */
  line[ len ] = ':';
  ++len;

  Action action = act(game, state, rng);

  /* do the action! */
  assert( isValidAction( game, &(state->state), 0, &action ) );
  int r = printAction( game, &action, MAX_LINE_LEN - len - 2, &line[ len ] );
  if( r < 0 ) {
    fprintf( stderr, "ERROR: line too long after printing action\n" );
    exit( EXIT_FAILURE );
  }
  len += r;
  line[ len ] = '\r';
  ++len;
  line[ len ] = '\n';
  ++len;

  return len;
}

int main( int argc, char **argv ) {
  int sock, len;
  uint16_t port;
  Game *game;
  MatchState state;
  FILE *file, *toServer, *fromServer;
  struct timeval tv;
  rng_state_t rng;
  char line[ MAX_LINE_LEN ];

  /* we make some assumptions about the actions - check them here */
  assert( NUM_ACTION_TYPES == 3 );

  if( argc < 3 ) {

    fprintf( stderr, "usage: player server port\n" );
    exit( EXIT_FAILURE );
  }

  /* Initialize the player's random number state using time */
  gettimeofday( &tv, NULL );
  init_genrand( &rng, tv.tv_usec );

  /* get the game */
  file = fopen( "./holdem.nolimit.2p.reverse_blinds.game", "r" );
  if( file == NULL ) {

    fprintf( stderr, "ERROR: could not open game './holdem.nolimit.2p.reverse_blind.game'\n");
    exit( EXIT_FAILURE );
  }
  game = readGame( file );
  if( game == NULL ) {

    fprintf( stderr, "ERROR: could not read game './holdem.nolimit.2p.reverse_blind.game'\n");
    exit( EXIT_FAILURE );
  }
  fclose( file );

  /* connect to the dealer */
  if( sscanf( argv[ 2 ], "%"SCNu16, &port ) < 1 ) {

    fprintf( stderr, "ERROR: invalid port %s\n", argv[ 2 ] );
    exit( EXIT_FAILURE );
  }
  sock = connectTo( argv[ 1 ], port );
  if( sock < 0 ) {

    exit( EXIT_FAILURE );
  }
  toServer = fdopen( sock, "w" );
  fromServer = fdopen( sock, "r" );
  if( toServer == NULL || fromServer == NULL ) {

    fprintf( stderr, "ERROR: could not get socket streams\n" );
    exit( EXIT_FAILURE );
  }

  /* send version string to dealer */
  if( fprintf( toServer, "VERSION:%"PRIu32".%"PRIu32".%"PRIu32"\n",
	       VERSION_MAJOR, VERSION_MINOR, VERSION_REVISION ) != 14 ) {

    fprintf( stderr, "ERROR: could not get send version to server\n" );
    exit( EXIT_FAILURE );
  }
  fflush( toServer );

  /* play the game! */
  while( fgets( line, MAX_LINE_LEN, fromServer ) ) {

    /* ignore comments */
    if( line[ 0 ] == '#' || line[ 0 ] == ';' ) {
      continue;
    }

    len = readMatchState( line, game, &state );
    if( len < 0 ) {

      fprintf( stderr, "ERROR: could not read state %s", line );
      exit( EXIT_FAILURE );
    }

    if( stateFinished( &state.state ) ) {
      /* ignore the game over message */

      continue;
    }

    if( currentPlayer( game, &state.state ) != state.viewingPlayer ) {
      /* we're not acting */

      continue;
    }

    len = step(len, line, game, &state, &rng);

    if( fwrite( line, 1, len, toServer ) != len ) {

      fprintf( stderr, "ERROR: could not get send response to server\n" );
      exit( EXIT_FAILURE );
    }
    fflush( toServer );
  }

  return EXIT_SUCCESS;
}
