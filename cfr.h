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
#include <unordered_map>
#include <string>
#include <tuple>

#include "game.h"
#include "rng.h"
#include "net.h"
#include "evalHandTables.h"

class NLTH {
public:
  NLTH() {
    rng = new rng_state_t;
    round = 0;
    mIsEnd = false;
  }
  ~NLTH() = default;
  void step(uint8_t type, uint32_t raiseSize) {
    switch (type) {
    case a_fold:
      mIsEnd = true;
      break;
    case a_call:

    case a_raise:

    default:
      break;
    }
  }
  bool isEnd() const {
    return mIsEnd;
  }
  uint8_t currentPlayer() const;
  std::string infoSet() const;

private:
  static const uint8_t numPlayers = 2;
  static const uint8_t numRounds = 4;
  static const uint32_t SB = 50;
  static const uint32_t BB = 100;
  std::string stage;
  uint8_t round;
  bool mIsEnd;
  rng_state_t *rng;
};

class Node {
public:
  Node(const uint8_t actionNum) {
    mActionNum = actionNum;
  }
  // get strategy at info node according to regret matching
  const double *strategy();

  void strategySum(const float *strategy, const float realizationWeight);

  // cfr of action
  double regretSum(const int action) const;

  // update regret
  void regretSum(const int action, const float value);

  // number of available actions
  uint8_t actionNum() const;
private:
  uint8_t mActionNum;
  double *mRegretSum;
  double *mStrategySum;
  double *mAverageStrategy;
};

class Trainer {
public:
  Trainer() {
    strategies = new std::unordered_map<std::string, Node *>[2];
    bool = new bool[2];
    folderPath = "./strategies";
  }
  ~Trainer() {
    delete[] strategies;
    delete[] update;
  }
  double externalSamplingCFR(Game *game, MatchState *state, uint_8 player_idx);
  void train(const size_t iterations);
  void writeStrategy(const size_t iterations) const;
private:
  // the strategies of each player
  std::unordered_map<std::string, Node *> *strategies;
  bool *update;
  std::string folderPath;
};

#endif // _CFR_H_