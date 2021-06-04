# Poker_Project
## Methodology

### Main Idea

In the design, the main idea is to guess the opponent’s hand using a belief system. Intuitively, the opponent would raise or call at some game stage if his hand has greater chances of winning. Therefore, to build a belief system of the probability distribution of opponent’s hand according to opponent’s actions is promising.

After having a belief over opponent’s hand, the agent should be able to solve the winning rate at a certain stage with respect to the belief. This thought is similar to the one that is used in PBE evaluation: find a rational response when the agent has a belief.

The last step is to generate a mixed behavioral strategy given the winning rate calculated by the agent. The idea is that the agent will definite fold when the winning rate is too low. Also, the expectation his raise rate and raise will increase (weak) exponentially w.r.t his winning rate. 

The following sections will explain each idea in greater detail.

### Belief System

A table is stored in `game.h` to store the probability distribution and is updated each time the agent is to make an action. The table looks like this:

```c++
double prob_oppo[52][52];
```

And it is maintained in this function:

```c++
void updateBelief();
```

To maintain the belief in a rational way, the last raise(call)’s value made by the opponent is calculated and compared with the pot size (represent by a fraction   $raise / pot$   ). Since we need an exponential discount factor then, $raise / pot$ is clipped when it exceeds $15$. The reason to use $pot$ as reference is that in some papers (including some poker strategy websites) uses pot as a main metric of an action’s aggressiveness.

Then we enumerates the opponent’s hand and get the winning rate of his hand with the assumption that the remaining cards (mine or the public card) are uniformly distributed. This is done by Monte Carlo sampling. So the hand `{i, j}` will get a discount factor of `exp(alpha * win_rate * ratio * (1 + round))`. The hands with higher winning rate will be amplified. In this way, we get a plausible belief system.

### Winning Rate Evaluation

With the belief system, the next step is to evaluate the winning rate. The methodology is to use Monte Carlo method, since it will take too long a time to solve the actual probability through exhaustive search. The function is given by:

```c++
double evalWinRateWithBelief();
```

In this function, we just specifies the number of Monte Carlo rounds, and in each round we sample opponent’s hand according to the maintained distribution and sample the other public cards uniformly. (order of sampling does not matter). 

### Strategy Generation

The strategy is based on the winning rate, and is divided into two parts: pre-flop and post-flop strategies.

```c++
typedef struct {
  double prob_fold = 0.0;
  double prob_call = 0.0;
  double prob_raise = 0.0;
  double raise_size;
} ProbAct;

ProbAct evalStrategy();
```

The pre-flip strategy is to make moves according to the winning rate of start hand, and the winning rate is efficiently stored in an array table. Specifically, in the pre-flop round (round 0) the agent folds with $100\%$ when the winning rate is below $0.5$, and the rate of raising increases with the winning rate. In the post-flop strategies, the agent folds with $100%$ when the winning rate is below $0.42$, and and the rate of raising increases with the winning rate.

## Code Usage

My final agent is in the `player.cpp` and `player.h` **ONLY**, the other agents or files are for test uses.

Also, I added some features to `game.h` and `game.cpp` file, and renamed `evalHandTables` to `evalHandTables.h`, so be careful when writing the makefile.

## Acknowledgements

The author would like to thank Li Zhouzi for providing thoughts on the belief system.
