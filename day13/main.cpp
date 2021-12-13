#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <ctime>
#include <filesystem>
#include <iostream>
#include <unordered_set>
#include <deque>
#include <vector>

#include "utils.h"

struct Cave {
	std::string name;
	bool small = false;
	std::vector<Cave*> next;
};

using Caves = std::unordered_map<std::string, Cave>;
using Path = std::vector<Cave const*>;


bool CanVisit(Cave const* cave, Path const& path)
{
	if (cave->small) {
		// O(N) search here could be improved?
		for (auto* visited : path) {
			if (visited == cave) {
				return false;
			}
		}
	}
	return true;
}


uint32_t part1(Cave const* start, Cave const* end) {
	uint32_t count = 0;

	Path path;
	std::vector<int> index;
	path.push_back(start);
	index.push_back(-1);

	while (!path.empty()) {
		auto& i = index.back();
		auto& c = *path.back();
		++i;
		if (size_t(i) >= c.next.size()) {
			index.pop_back();
			path.pop_back();
		}
		else {
			auto* next = c.next[i];
			if (next == end) {
				++count;
			}
			else if (CanVisit(next, path)) {
				path.push_back(next);
				index.push_back(-1);
			}
		}
	}
	return count;
}

bool CanVisit2(Cave const* cave, Path const& path, Cave const* small_cave)
{
	if (cave->small) {
		if (cave == small_cave) {
			if (std::count(path.begin(), path.end(), cave) > 1) {
				return false;
			}
		}
		else {
			// O(N) search here could be improved?
			for (auto* visited : path) {
				if (visited == cave) {
					return false;
				}
			}
		}
	}
	return true;
}

std::string ToString(Path const& path) {
	std::string s;
	for (auto* c : path) {
		s += c->name;
		s += ".";
	}
	return s;
}

uint32_t part2(Cave const* start, Cave const* end, Caves const& caves) {
	uint32_t count = 0;

	std::vector<Cave const*> small_caves;
	for (auto& item : caves) {
		auto* c = &item.second;
		if (c->small && c != start && c != end) {
			small_caves.push_back(c);
		}
	}

	Path path;
	std::vector<int> index;
	std::set<std::string> distinct;

	for (auto* small_cave : small_caves) {
		path = { start };
		index = { -1 };
		while (!path.empty()) {
			auto& i = index.back();
			auto& c = *path.back();
			++i;
			if (size_t(i) >= c.next.size()) {
				index.pop_back();
				path.pop_back();
			}
			else {
				auto* next = c.next[i];
				if (next == end) {
					auto id = ToString(path);
					if (!contains(distinct, id)) {
						++count;
						distinct.insert(id);
					}
				}
				else if (CanVisit2(next, path, small_cave)) {
					path.push_back(next);
					index.push_back(-1);
				}
			}
		}
	}
	return count;
}

void solveFile(char const* fname) {
	// Load graph
	std::string line;
	std::ifstream fs(fname);
	if (!fs) {
		throw std::runtime_error(fname);
	}
	Caves caves;
	while (std::getline(fs, line)) {
		auto parts = split(line, "-");
		auto& a = caves[parts[0]];
		auto& b = caves[parts[1]];
		if (a.next.empty()) {
			a.name = parts[0];
			a.small = std::islower(unsigned(parts[0][0]));
		}
		if (b.next.empty()) {
			b.name = parts[1];
			b.small = std::islower(unsigned(parts[1][0]));
		}
		a.next.push_back(&b);
		b.next.push_back(&a);
	}
	auto* start = &(caves.find("start")->second);
	auto* end = &(caves.find("end")->second);
	print(part1(start, end));
	print(part2(start, end, caves));
}

void main() {

	auto cwd = std::filesystem::current_path();
	print("example");
	solveFile("example.txt");
	print("input");
	solveFile("input.txt");
}
