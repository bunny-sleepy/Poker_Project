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
  int ID = state->viewingPlayer;
  int n = 0; // number of public cards.
  int N = 100000;
  double prob_oppo[52][52];

  int my_card[2];
  int pub[5] = {0};
  int rnd = state->state.round;
  // cout<<"round: "<<rnd<<endl;
  // cout<<"minnolimit: "<<state->state.minNoLimitRaiseTo<<endl;
  for (int i = 0; i<2; i++){
    int t = state->state.holeCards[ID][i];
    my_card[i] = 13 * (t % 4) + t/4;
  }
  if(rnd>0){
    for(int i = 0; i<2+rnd; i++){
      int t = state->state.boardCards[i];
      pub[i] = 13*(t % 4) + t/4;
    }
    n = rnd + 2;
  }
  for (int i = 0; i<52;i++){
    for (int j = 0; j<52; j++){
      prob_oppo[i][j] = state->state.prob_oppo[i][j];
    }
  }
  bool flag = 0;//for debug
  double rate;
//   rate = get_rate_with_prob(my_card,prob_oppo,pub,n,N);
  rate = evalWinRateUniform(game, state);
  // cout<< "rate: "<<rate<<endl;
  int size, least_raise;
  least_raise = state->state.minNoLimitRaiseTo;
  if (rate<0.70){
      probs[a_fold] = 1;
      probs[a_raise] = 0;
      probs[a_call] = 0;
  }
  else{
      probs[a_fold] = 0;
      probs[a_raise] = 1;
      probs[a_call] = 0;
      if(rnd == 0)
        size = 1000;
      if(rnd == 1)
        size = 5000;
      if(rnd >=2)
        size = 20000;
  }
  /* Define the probabilities of actions for the player */
//   probs[ a_fold ] = 0.06;
//   probs[ a_call ] = ( 1.0 - probs[ a_fold ] ) * 0.5;
//   probs[ a_raise ] = ( 1.0 - probs[ a_fold ] ) * 0.5;
  // choose_action(rate, probs, &size, bet, rng)

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
    // cout<<"can raise"<<endl;
  }
  // cout<<"min: "<<min <<" max: "<<max<<endl;

  /* normalise the probabilities  */
  if(p==0){
    actionProbs[a_call] = 1;
    p = 1;
    // flag = 1;
  }
  if(flag){
    // cout<<"WARNING: p == 0!"<<endl;
    for (int i = 0; i<2; i++){
      // cout<<"my card: "<<my_card[i]<<" ";
    }
    if(rnd>0){
      for(int i = 0; i<2+rnd; i++){
        // cout<<"pub i:"<<pub[i]<<" ";
      }
    }
    // cout<< "rate: "<<rate<<endl;
    // cout<<"n: "<<n<<endl;
    // cout<<p<<endl;
    // cout<<"probs: "<<probs[a_fold]<<", "<<probs[a_call]<<", "<<probs[a_raise]<<endl;
  }
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
    if(size<min){
    //   cout<<"ERROR: ------------------raise value too small--------------------------";
        size = min;
    }
    else{
      if(size>max){
        size = max;
      }
    }
    action.size = size;
  }
  else {
    action.size = 0;
  }
  return action;
}


/* Anything related with socket is handled below. */
/* If you are not interested with protocol details, you can safely skip these. */

int step(int len, char line[], Game *game, MatchState *state, rng_state_t *rng) {
  /* add a colon (guaranteed to fit because we read a new-line in fgets) */
  line[ len ] = ':';
  ++len;

  Action action = act(game, state, rng);
  // cout<<action.type<<" "<<action.size<<endl;
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
