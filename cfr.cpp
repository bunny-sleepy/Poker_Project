#include "cfr.h"
#include <string>

double Trainer::externalSamplingCFR(Game *game, MatchState *state, uint8_t playerIdx) {
  if (game_done()) {
    return payoff();
  }
  std::string infoSet;
  const int actionNum = game->actionNum();
  const uint8_t player = state->state.actingPlayer;
  
}

void Trainer::train(const size_t iterations) {
  return;
}

void Trainer::writeStrategy(const size_t iterations) const {
  return;
}

// train the CFR model
int main() {
  
  return 0;
}