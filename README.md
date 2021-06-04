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

To maintain the belief in a rational way, the last raise(call)’s value made by the opponent is calculated and compared with the pot size (represent by a fraction   $raise / pot$   ). Since we need an exponential discount factor then, $raise / pot$ is clipped when it exceeds $15$.

Then we enumerates the opponent’s hand and get the winning rate of his hand with the assumption that the remaining cards (mine or the public card) are uniformly distributed. This is done by Monte Carlo sampling. So the hand `{i, j}` will get a discount factor of `exp(alpha * win_rate * ratio * (1 + round))`. The hands with higher winning rate will be amplified. In this way, we get a plausible belief system.

### Winning Rate Evaluation



### Strategy Generation



## Code Usage

My final agent is in the `player.cpp` and `player.h` ONLY, the other agents or files are for test uses.

Also, I added some features to `game.h` and `game.cpp` file, and renamed `evalHandTables` to `evalHandTables.h`, so be careful when writing the makefile.