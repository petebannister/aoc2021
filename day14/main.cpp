#include "utils.h"

using namespace std;
using Bond = pair<char, char>;
using Counts = map<char, uint64_t>;

struct Reaction
{
    vector<char> polymer;
    map<Bond, char> rules;
    map<pair<Bond, uint16_t>, Counts> memo;

    void merge(Counts& c, Counts const& rhs) {
        for (auto [key, value] : rhs) {
            c[key] += value;
        }
    }

    Counts const& polymerize(Bond bond, uint32_t n) {
        auto& c = memo[{bond, n}];
        if (c.empty()) {
            auto r = rules[bond];
            if (n == 0) {
                c[r] = 1;
            }
            else {
                c = polymerize({ bond.first, r }, n - 1);
                merge(c, polymerize({ r, bond.second }, n - 1));
                ++c[r];
            }
        }
        return c;
    }

    Counts iterate(uint32_t n) {
        Counts counts;
        for (auto e : polymer) {
            ++counts[e];
        }
        for (auto i : integers(polymer.size() - 1)) {
            auto const& c = polymerize(Bond(polymer[i], polymer[i + 1]), n - 1);
            merge(counts, c);
        }
        return counts;
    }
};

void solveFile(char const* fname, uint32_t iterations) {
    TextFileIn f(fname);
    StringView sv;
    f.readLine(sv);
    Reaction reaction;
    reaction.polymer.assign(sv.begin(), sv.end());
    f.readLine(sv);
    for (auto line : f.lines()) {
        reaction.rules[{line[0], line[1]}] = line[6];
    }
    auto counts = reaction.iterate(iterations);
    auto nmin = counts.begin()->second;
    auto nmax = nmin;
    for (auto [elem, count] : counts) {
        amin(nmin, count);
        amax(nmax, count);
    }
    print(nmax - nmin);
}

void main() {
    print("example");
    solveFile("example.txt", 10);
    solveFile("example.txt", 40);
    print("input");
    solveFile("input.txt", 10);
    solveFile("input.txt", 40);
}
