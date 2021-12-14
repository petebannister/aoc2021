#include "utils.h"

void solveFile(char const* fname) {
	// Parse
	TextFileIn f(fname);
	std::list<char> polymer;
	std::map<std::pair<char, char>, char> rules;
	StringView sv;
	f.readLine(sv);
	polymer.assign(sv.begin(), sv.end());
	f.readLine(sv);
	for (auto line : f.lines()) {
		auto a = line[0];
		auto b = line[1];
		auto v = line[6];
		rules[std::pair(a, b)] = v;
	}
	for (auto i : integers(10)) {
		auto i = polymer.begin();
		auto e = polymer.end();
		char last = *i;
		++i;
		while (i != e) {
			if (auto r = rules[std::pair(last, *i)]) {
				polymer.insert(i, r);
			}
			last = *i++;
		}
	}

	std::map<char, int> counts;
	for (auto c : polymer) {
		counts[c]++;
	}
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
	solveFile("example.txt");
	print("input");
	solveFile("input.txt");
}
