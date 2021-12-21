#include "utils.h"

#include <future>

using namespace std;

struct DiracDice {
    uint64_t a;
    uint64_t b;

    uint64_t rolls = 0u;
    uint64_t score_a = 0u;
    uint64_t score_b = 0u;

    // There must be an analytical solution to this... but not sure 
    // how the rules will change for part 2...
    // something along the lines of f(offset + 6n) % (10n) or 6(f(n)) % (10n)

    uint64_t roll() {
        return ++rolls;
    }

    uint64_t playOne(uint64_t& p) {
        p += roll() + roll() + roll();
        p %= 10;
        return p + 1;
    }

    void playUntil(uint64_t score) {
        bool player1 = true;
        while (winningScore() < score) {
            if (player1) {
                score_a += playOne(a);
            }
            else {
                score_b += playOne(b);
            }
            player1 = !player1;
        }
    }

    uint64_t losingScore() const {
        return min(score_a, score_b);
    }
    uint64_t winningScore() const {
        return max(score_a, score_b);
    }
};

void solveFile(char const* fname) {
    TextFileIn f(fname);

    DiracDice start;

    StringView sv;
    f.readLine(sv);
    sv.split(':');
    sv.pop_front();
    start.a = sv.parseInt32() - 1;

    f.readLine(sv);
    sv.split(':');
    sv.pop_front();
    start.b = sv.parseInt32() - 1;

    auto game = start;
    game.playUntil(1000);
    print(game.losingScore() * game.rolls);
    print(0);
}

void main() {
    print("example");
    solveFile("example.txt");
    print("input");
    solveFile("input.txt");
}
