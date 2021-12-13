#include "utils.h"

struct Origami
{
	std::set<Point> points;
	std::vector<Point> folds;
	
	void fold(Point f) {
		std::set<Point> folded;
		for (Point p : points) {
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
		Bounds bounds(points);
		bounds.print_f([&](Point p) {
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
	// Parse
	TextFileIn f(fname);
	Origami origami;
	for (auto line : f.lines()) {
		if (line.empty()) {
			break;
		}
		auto x = line.split(',').i32();
		auto y = line.i32();
		origami.points.insert({ x, y });
	}
	for (auto line : f.lines()) {		
		line = line.mid(11); // "fold along "
		auto d = line.split('=');
		auto fold = Point{ line.i32(), 0 };
		if (d.startsWith('y')) {
			fold.swapxy();
		}
		origami.folds.push_back(fold); 
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
