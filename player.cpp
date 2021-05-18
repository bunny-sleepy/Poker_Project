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

double eval_win_rate(const uint8_t *card_pri, const uint8_t *card_pub, const uint8_t round);
double eval_win_rate(Game *game, MatchState *state);

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
  unsigned int pot = 0; // current total pot value
  for (uint8_t i = 0; i < game->numPlayers; ++i) {
    pot += state->state.spent[i];
  }

  // update public cards according to current cards
  if (round > 0) {
    for (uint8_t i = 0; i < round + 2; ++i) {
      card_pub[i] = state->state.boardCards[i];
    }
    num_pub = round + 2;
  }

  /* Define the probabilities of actions for the player */
  probs[a_fold] = 0.06;
  probs[ a_call ] = ( 1.0 - probs[ a_fold ] ) * 0.5;
  probs[ a_raise ] = ( 1.0 - probs[ a_fold ] ) * 0.5;

  /* build the set of valid actions */
  double p = 0.0;
  int a;
  for( a = 0; a < NUM_ACTION_TYPES; ++a ) {
    actionProbs[ a ] = 0.0;
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
    // TODO: check if raise size < min
    action.size = min + genrand_int32( rng ) % ( max - min + 1 );
  }
  else {
    action.size = 0;
  }

  return action;
}

// TODO: implement a function to evaluate the winning rate given the current information
double eval_win_rate(const uint8_t* card_pri, const uint8_t* card_pub, const uint8_t round) {
  uint8_t num_pub = (round > 0) ? (round + 2) : 0; // number of public cards
  Cardset cardset_pri, cardset_oppo;
  // addCardToCardset(&cardset_pri, card_pri[]
}

double eval_win_rate(Game *game, MatchState *state) {
  const uint8_t num_pub = (state->state.round > 0) ? (state->state.round + 2) : 0; // number of public cards
  Cardset cardset_pri, cardset_oppo;
  const uint8_t ID = state->viewingPlayer;
  bool card_use_flag[52] = {0};
  // the total number of guesses and wins
  size_t tot = 0, win = 0;
  uint8_t curr_card_num = num_pub;

  // init current info to the cardsets
  addCardToCardset(&cardset_pri, state->state.holeCards[ID][0] % 4, state->state.holeCards[ID][0] / 4);
  addCardToCardset(&cardset_pri, state->state.holeCards[ID][1] % 4, state->state.holeCards[ID][1] / 4);
  card_use_flag[state->state.holeCards[ID][0]] = 1;
  card_use_flag[state->state.holeCards[ID][1]] = 1;
  for (uint8_t i = 0; i < num_pub; ++i) {
    addCardToCardset(&cardset_pri, state->state.boardCards[i] % 4, state->state.holeCards[i] / 4);
    addCardToCardset(&cardset_oppo, state->state.boardCards[i] % 4, state->state.holeCards[i] / 4);
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
      while (true) {
        for (uint8_t )
      }
      card_use_flag[oppo2] = 0;
    }
    card_use_flag[oppo1] = 0;
  }
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


int main( int argc, char **argv )
{
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
