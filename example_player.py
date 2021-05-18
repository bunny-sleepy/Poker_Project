import sys
import socket
import random


def eprint(*args, **kwargs):
    print(*args, file=sys.stderr, **kwargs)


class GameState:
    def __init__(self, match_string):
        if match_string[:11] != "MATCHSTATE:":
            raise AssertionError("ERROR: message must starts with 'MATCHSTATE:'")
        match_string = match_string[11:]
        message = match_string.strip().split(":")

        # [<position>, <handNumber>, <betting>, <cards>]
        self.message = message
        # int, <position>
        self.viewingPlayer = int(message[0])
        # int, <handNumber>
        self.handId = int(message[1])
        # string, <betting>
        self.betting_string = message[2]
        # list, each element is a string of cards dealt in each round.
        # e.g. ['9hQd']: 9hQd are dealt as private cards in the first round
        #      ['5d5c', '8dAs8s', '4h']: 9hQd are dealt as private cards in the first round,
        #           8dAs8s are dealt in the second round, and 4h is dealt in the third round.
        # NOTICE: do not use this when the game is finished.
        self.cards = self._cal_card_sequence(message[3])

        # A nested list of shape
        # (number of rounds played, number of actions in that round).
        # Actions are "c", "f", "r300"(or any other valid betting size)
        self.betting_sequence = self._cal_betting_sequence(self.betting_string)
        # Current round. One of (0, 1, 2, 3).
        self.round = len(self.betting_sequence)

        # bool, whether the hand is finished
        self.finished = None
        # bool, whether the agent should act
        self.acting = None
        # list of two ints. e.g. [100, 50] means player at position 0 has spent
        # 100, and player at position 1 has spent 50.
        self.spent = None

        # Calculate self.finished, self.acting, and self.spent.
        self._simulate_game()

    @staticmethod
    def _cal_betting_sequence(betting_string):
        betting_sequence = list()
        for x in betting_string.split("/"):
            sequence = list()
            s = 0
            while s < len(x):
                if x[s] == 'c' or x[s] == 'f':
                    sequence.append(x[s])
                    s += 1
                else:
                    e = s + 1
                    while e < len(x) and \
                            (x[e] != 'c' and x[e] != 'f' and x[e] != 'r'):
                        e += 1
                    sequence.append(x[s:e])
                    s = e
            betting_sequence.append(sequence)
        return betting_sequence
    
    @staticmethod
    def _cal_card_sequence(cards_string):
        cards = cards_string.split("/")
        if cards[0][0] == "|":
            cards[0] = cards[0][1:]
        else:
            cards[0] = cards[0][:-1]
        return cards

    def _simulate_game(self):
        finished = False
        spent = [100, 50]   # big blind & small blind
        acting = 1
        for r, s in enumerate(self.betting_sequence):
            if r > 0:
                acting = 0
            acted = 0
            for a in s:
                if a[0] == 'f':
                    finished = True
                elif a[0] == 'c':
                    spent[acting] = spent[1-acting]
                    acted += 1
                    if acted >= 2 and r == 3:
                        finished = True
                else:
                    spent[acting] = int(a[1:])
                    acted += 1
                acting = 1 - acting

        if spent[self.viewingPlayer] == 20000:
            finished = True
        self.finished = finished
        self.acting = (not finished) and (acting == self.viewingPlayer)
        self.spent = spent

    def is_fold_valid(self) -> bool:
        """Check whether fold is a valid action in current state.

        :return: True if fold is valid, False otherwise.
        """
        if self.spent[0] == self.spent[1]:
            return False
        return True

    def is_raise_valid(self):
        """Check if raise if a valid action in current state. If valid, give
        min and max raise bets (the amount that can be raised TO, NOT raised
        BY).

        :return: None if raise is not valid, (min_bet, max_bet) otherwise.
        """
        if self.spent[1-self.viewingPlayer] == 20000:
            return None
        spent = [100, 50]
        acting = 1
        for r, s in enumerate(self.betting_sequence[:-1]):
            if r > 0:
                acting = 0
            for a in s:
                if a[0] == 'c':
                    spent[acting] = spent[1-acting]
                else:
                    spent[acting] = int(a[1:])
                acting = 1 - acting
        if len(self.betting_sequence) > 1:
            assert(spent[0] == spent[1])
        else:
            assert(spent == [100, 50])

        current_bet = spent[0]
        min_raise = 100
        for a in self.betting_sequence[-1]:
            if a[0] == 'r':
                min_raise = int(a[1:]) - current_bet
                current_bet = int(a[1:])
        return min(current_bet + min_raise, 20000), 20000


def act(state: GameState):
    a = random.randint(0, 3)
    if a == 1 and state.is_fold_valid():
        return "f"
    elif (a == 2 or a == 3) and state.is_raise_valid():
        min_bet, max_bet = state.is_raise_valid()
        if a == 2:
            return "r" + str(min_bet)
        else:
            return "r" + str(max_bet)
    else:
        return "c"


if __name__ == "__main__":
    if len(sys.argv) < 3:
        eprint("usage: player server port")
        sys.exit(1)

    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect((sys.argv[1], int(sys.argv[2])))

    if sock.send("VERSION:2.0.0\n".encode()) != 14:
        eprint("ERROR: could not send version to server")
        sys.exit(1)
    
    while True:
        lines = sock.recv(4096).decode()  # we may read multiple lines at a time
        if lines is None or len(lines) <= 0:
            break
        for line in lines.strip().split("\r\n"):
            game_state = GameState(line)
            if game_state.acting is True:
                action = act(game_state)
                ret = line + ":"
                ret += action
                ret += "\r\n"
                if sock.send(ret.encode()) != len(ret):
                    eprint("ERROR: could not send response to server")
                    sys.exit(1)

    sock.close()
    sys.exit(0)
