#include "utils.h"

using namespace std;

using Bond = std::pair<char, char>;
using Counts = std::map<char, uint64_t>;
using Key = pair<Bond, uint16_t>;

struct Reaction
{
	std::vector<char> polymer;
	std::map<Bond, char> rules;
	map<Key, Counts> memo;

	void merge(Counts& c, Counts const& rhs) {
		for (auto [key, value] : rhs) {
			c[key] += value;
		}
	}

	Counts const& polymerize(Bond bond, uint32_t n) {
		auto key = Key(bond, n);
		auto& c = memo[key];
		if (c.empty()) {
			if (auto r = rules[bond]) {
				if (n == 0) {
					c[r] = 1;
				}
				else {
					c = polymerize(Bond(bond.first, r), n - 1);
					merge(c, polymerize(Bond(r, bond.second), n - 1));
					++c[r];
				}
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
	// Parse
	TextFileIn f(fname);

	StringView sv;
	f.readLine(sv);
	Reaction reaction;
	reaction.polymer.assign(sv.begin(), sv.end());
	f.readLine(sv);
	for (auto line : f.lines()) {
		auto a = line[0];
		auto b = line[1];
		auto v = line[6];
		reaction.rules[std::pair(a, b)] = v;
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
