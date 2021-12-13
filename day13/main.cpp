

#include "utils.h"

struct Origami
{
	std::set<point> points;
	std::vector<point> folds;

	void add(point p) {
		points.insert(p);
	}

	void fold(point f) {
		std::set<point> folded;
		for (point p : points) {
			if (f.x != 0) {
				if (p.x > f.x) {
					p.x = 2 * f.x - p.x;
				}
			}
			else {
				if (p.y > f.y) {
					p.y = 2 * f.y - p.y;
				}
			}
			folded.insert(p);
		}
		points.swap(folded);
	}

	void print() {
		bounds b;
		for (auto&& p : points) add(p);
		b.print_f([&](point p) {
			return contains(points, p) ? '#' : ' '; });
	}
};


uint32_t part1(Origami& origami) 
{
	origami.fold(origami.folds[0]);
	return (uint32_t)origami.points.size();
}

void part2(Origami& origami)
{
	// We can repeat the first fold even though part1 did it
	for (auto fold : origami.folds) {
		origami.fold(fold);
	}
	origami.print();
}

void solveFile(char const* fname) {
	// Load graph
	std::string line;
	std::ifstream fs(fname);
	if (!fs) {
		throw std::runtime_error(fname);
	}
	Origami origami;
	while (std::getline(fs, line)) {
		if (line.empty()) {
			break;
		}
		auto parts = split(line, ",");
		origami.add({ atoi(parts[0].c_str()), atoi(parts[1].c_str()) });
	}
	while (std::getline(fs, line)) {
		auto fold_along = "fold along";
		line.erase(0, 11);
		auto parts = split(line, "=");
		if (parts[0][0] == 'x') {
			origami.folds.push_back({ atoi(parts[1].c_str()), 0 });
		}
		else { 
			origami.folds.push_back({ 0, atoi(parts[1].c_str()) }); 
		}		
	}

	print(part1(origami));
	part2(origami);
}

void main() {
	print("example");
	solveFile("example.txt");
	print("input");
	solveFile("input.txt");
}
