#include "player.h"
#include <iostream>

using namespace std;

int main() {
  Cardset cardset_1 = emptyCardset(), cardset_2 = emptyCardset();
  addCardToCardset(&cardset_1, 0, 0);
  addCardToCardset(&cardset_1, 0, 6);
  addCardToCardset(&cardset_1, 0, 2);
  addCardToCardset(&cardset_1, 0, 3);
  addCardToCardset(&cardset_1, 0, 4);
  addCardToCardset(&cardset_1, 0, 5);
  addCardToCardset(&cardset_1, 0, 12);
  cardset_1 = emptyCardset();
  addCardToCardset(&cardset_2, 0, 7);
  addCardToCardset(&cardset_2, 0, 8);
  addCardToCardset(&cardset_2, 0, 9);
  addCardToCardset(&cardset_2, 0, 10);
  addCardToCardset(&cardset_2, 0, 11);
  addCardToCardset(&cardset_2, 0, 12);
  addCardToCardset(&cardset_2, 0, 6);
  // cout << rankCardset(cardset_1) << endl;
  cout << rankCardset(cardset_2) << endl;
  bool card_use_flag[52] = {0};
  uint32_t tot = 0, win = 0;
  uint8_t pri1 = 25, pri2 = 38;
  card_use_flag[pri1] = 1;
  card_use_flag[pri2] = 1;
  uint8_t round = 1;
  uint8_t num_pub = round + 2;
  uint8_t boardCards[5] = {12, 26, 37, 51, 6};
  for (uint8_t i = 0; i < 5; ++i) {
    card_use_flag[boardCards[i]] = 1;
  }

  for (uint8_t oppo1 = 0; oppo1 < 52; ++oppo1) {
    if (card_use_flag[oppo1]) continue;
    card_use_flag[oppo1] = 1;
    for (uint8_t oppo2 = 0; oppo2 < 52; ++oppo2) {
      if (card_use_flag[oppo2]) continue;
      card_use_flag[oppo2] = 1;
      // dfs for the remaining public cards
      if (round <= 2) for (uint8_t pub1 = 0; pub1 < 52; ++pub1) {
        if (card_use_flag[pub1]) continue;
        card_use_flag[pub1] = 1;
        if (round <= 1) for (uint8_t pub2 = 0; pub2 < 52; ++pub2) {
          if (card_use_flag[pub2]) continue;
          tot++;
          card_use_flag[pub2] = 1;
          // init current info to the cardsets
          Cardset cardset_pri = emptyCardset(), cardset_oppo = emptyCardset();
          addCardToCardset(&cardset_pri, pri1 % 4, pri1 / 4);
          addCardToCardset(&cardset_pri, pri2 % 4, pri2 / 4);
          for (uint8_t i = 0; i < num_pub; ++i) {
            addCardToCardset(&cardset_pri, boardCards[i] % 4, boardCards[i] / 4);
            addCardToCardset(&cardset_oppo, boardCards[i] % 4, boardCards[i] / 4);
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
            addCardToCardset(&cardset_pri, boardCards[i] % 4, boardCards[i] / 4);
            addCardToCardset(&cardset_oppo, boardCards[i] % 4, boardCards[i] / 4);
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
          addCardToCardset(&cardset_pri, boardCards[i] % 4, boardCards[i] / 4);
          addCardToCardset(&cardset_oppo, boardCards[i] % 4, boardCards[i] / 4);
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
  cout << win << endl << tot << endl;
  return 0;
}