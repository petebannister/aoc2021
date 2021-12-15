#include "utils.h"


using namespace std;
using Bond = pair<char, char>;
using Counts = map<char, uint64_t>;

struct Elem {
    uint8_t risk = 0;
    uint32_t cost = 0xFFFFFFFFu;
};

using World = vector<vector<Elem>>;
Point const dirs[] = {
    {-1, 0}, // l
    {0, -1}, // t
    {1, 0}, // r
    {0, 1}, // b
};

Elem* elem(World& w, Point p) {
    if (p.x < 0 || p.x >= int(w[0].size()) || (p.y < 0) || (p.y >= int(w.size()))) {
        return nullptr;
    }
    return &w[p.y][p.x];
}

uint32_t solveWorld(World& world) 
{
    auto start = Point{ 0, 0 };
    auto end = Point{ int(world[0].size()) - 1, int(world.size()) - 1 };
    auto* pstart = elem(world, start);

    // Might be possible to speed this up with a priority queue
    std::deque<Point> q = { end };
    elem(world, q.back())->cost = 0;

    // Get the cost of any path as a starting point for the cost of the start node.:
    pstart->cost = 0;
    for (auto& v : world[0]) {
        pstart->cost += v.risk;
    }
    for (auto& line : world) {
        pstart->cost += line.back().risk;
    }

    while (!q.empty()) {
        auto p = q.front();
        q.pop_front();
        auto* c = elem(world, p);
        auto dp = (end - p);

        if (c->cost < pstart->cost) {
            for (auto d : dirs) {
                auto s = p + d;
                if (auto* e = elem(world, s)) {
                    auto new_cost = c->cost + e->risk;
                    if (new_cost < pstart->cost) {
                        if (e->cost > new_cost) {
                            e->cost = new_cost;
                            q.push_back(s);
                        }
                    }
                }
            }
        }
    }

    uint32_t r = 0;
    auto pos = start;
    while (pos != end) {
        auto* c = elem(world, pos);
        auto cost = c->cost;
        auto new_pos = pos;
        for (auto d : dirs) {
            auto s = pos + d;
            if (auto* e = elem(world, s)) {
                if (e->cost < cost) {
                    cost = e->cost;
                    new_pos = s;
                }
            }
        }
        pos = new_pos;
        r += elem(world, new_pos)->risk;
    }
    return r;
}

void solveFile(char const* fname) {
    TextFileIn f(fname);
    World world;
    for (auto line : f.lines()) {
        world.push_back({});
        for (auto ch : line) {
            world.back().push_back(Elem{ uint8_t(ch - '0'), 0xFFFFFFFFu});
        }
    }
    World big_world = world;
    big_world.resize(big_world.size() * 5);
    for (auto& row : big_world) {
        row.resize(world[0].size() * 5);
    }
    for (auto n : integers(5)) {
        if (n > 0) {
            for (auto y : integers(world.size())) {
                for (auto x : integers(world[0].size())) {
                    auto e = world[y][x];
                    e.risk = (e.risk + n);
                    if (e.risk > 9) {
                        e.risk -= 9;
                    }
                    big_world[y][x + n * world[0].size()] = e;
                }
            }
        }
    }
    for (auto y : integers(world.size())) {
        for (auto x : integers(big_world[0].size())) {
            for (auto n : integers(5)) {
                if (n > 0) {
                    auto e = big_world[y][x];
                    e.risk = (e.risk + n);
                    if (e.risk > 9) {
                        e.risk -= 9;
                    }
                    big_world[y + world.size() * n][x] = e;
                }
            }
        }
    }

    print(solveWorld(world));
    print(solveWorld(big_world));
}

void main() {
    print("example");
    solveFile("example.txt");
    print("input");
    solveFile("input.txt");
}
