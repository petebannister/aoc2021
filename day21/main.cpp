#include "utils.h"

#include <future>

using namespace std;

struct SimpleDice {
    uint64_t a = 0u;
    uint64_t b = 0u;

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


using Positions = std::pair<uint8_t, uint8_t>;
using Scores = std::pair<uint8_t, uint8_t>;

struct Key {
    Scores scores; // remaining scores
    Positions pos;
    bool turn;
    uint32_t memo() const {
        uint32_t v = scores.first;
        v <<= 8;
        v += scores.second;
        v <<= 8;
        v += pos.first;
        v <<= 8;
        v += pos.second;
        if (turn) {
            v |= 0x80000000uL;
        }
        return v;
    }
    bool operator ==(Key const& rhs) const {
        return (memo() == rhs.memo());
    }
    bool operator !=(Key const& rhs) const {
        return (memo() != rhs.memo());
    }
};
using Wins = std::pair<uint64_t, uint64_t>;
using WinsMap = unordered_map<Key, Wins>;

namespace std {
    template <>
    struct hash<Key> {
        size_t operator()(Key const& k) const {
            return std::hash<uint32_t>()(k.memo());
        }
    };
}

struct DiracDice {

    Positions start;
    WinsMap map;

    // each of roll splits universe into 3 giving 27 outcomes per 3 rolls (3 * 3 * 3).
    static constexpr uint8_t OUTCOMES = 27;
    uint8_t lut[10][OUTCOMES];

    static uint8_t next(uint8_t pos, uint8_t dice) {
        return (pos + (dice)) % 10;
    }

    DiracDice() {
        for (auto p : integers(10)) {
            auto d = 0u;
            for (auto a : integers(3)) {
                for (auto b : integers(3)) {
                    for (auto c : integers(3)) {
                        auto roll = a + b + c + 3;
                        lut[p][d++] = (p + roll) % 10;
                    }
                }
            }
        }
    }

    Wins winsForRemainingScore(Scores scores, Positions pos, bool turn) {
        auto key = Key{ scores, pos, turn };
        auto i = map.find(key);
        if (i != map.end()) {
            return i->second;
        }
        Wins wins = { 0u, 0u };
        for (auto d : integers(OUTCOMES)) {
            // Array of players may have been better..
            if (turn) {
                // player2's turn
                auto p2 = lut[pos.second][d];
                auto s2 = p2 + 1;
                if (s2 >= scores.second) {
                    ++wins.second;
                }
                else {
                    // did not win
                    auto r = winsForRemainingScore({ scores.first, scores.second - s2 }, { pos.first, p2 }, !turn);
                    wins.first += r.first;
                    wins.second += r.second;
                }
            }
            else {
                // player 1
                auto p1 = lut[pos.first][d];
                auto s1 = p1 + 1;
                if (s1 >= scores.first) {
                    ++wins.first;
                }
                else {
                    auto r = winsForRemainingScore({ scores.first - s1, scores.second }, { p1, pos.second }, !turn);
                    wins.first += r.first;
                    wins.second += r.second;
                }
            }
        }
        map.insert(pair(key, wins));
        return wins;
    }

    Wins playUntil(uint8_t score) {
        return winsForRemainingScore({ score, score }, start, false);
    }
};


void solveFile(char const* fname) {
    TextFileIn f(fname);

    Positions start;

    StringView sv;
    f.readLine(sv);
    sv.split(':');
    sv.pop_front();
    start.first = sv.parseInt32() - 1;

    f.readLine(sv);
    sv.split(':');
    sv.pop_front();
    start.second = sv.parseInt32() - 1;

    SimpleDice simple;
    simple.a = start.first;
    simple.b = start.second;
    simple.playUntil(1000);
    print(simple.losingScore() * simple.rolls);

    DiracDice dirac;
    dirac.start = start;
    auto wins = dirac.playUntil(21);
    print(max(wins.first, wins.second));
}

void main() {
    print("example");
    solveFile("example.txt");
    print("input");
    solveFile("input.txt");
}
