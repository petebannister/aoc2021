#include "utils.h"

using namespace std;

using World = vector<vector<char>>;

bool iterate(World& w, World& swp, bool dx) {
    bool changed = false;
    swp = w;
    auto height = w.size();
    auto width = w[0].size();

    for (auto y : integers(w.size())) {
        auto& curr = w[y];
        auto y2 = (y + 1) % height;
        for (auto x : integers(width)) {
            auto c = curr[x];
            if (dx) {
                if (c == '>') {
                    auto x2 = (x + 1) % width;
                    auto c2 = curr[x2];
                    if (c2 == '.') {
                        swp[y][x] = '.';
                        swp[y][x2] = c;
                        changed = true;
                    }
                }
            }
            else {
                if (c == 'v') {
                    auto c2 = w[y2][x];
                    if (c2 == '.') {
                        swp[y][x] = '.';
                        swp[y2][x] = c;
                        changed = true;
                    }
                }
            }
        }
    }
    w.swap(swp);
    return changed;
}
bool iterate(World& w, World& swp) {
    bool a = iterate(w, swp, true);
    bool b = iterate(w, swp, false);
    return a || b;
}

//-----------------------------------------------------------------------------
void solveFile(char const* fname) {
    TextFileIn f(fname);

    World world;
    World swp;

    for (auto line : f.lines()) {
        if (!line.empty()) {
            world.push_back(vector<char>(line.begin(), line.end()));
        }
    }


    uint32_t part1 = 1;
    while (iterate(world, swp)) {
        ++part1;
    }
    print(part1);
    uint32_t part2 = 0;
    print(part2);
}

//-----------------------------------------------------------------------------
void main() {
    print("example");
    solveFile("example.txt");
    print("input");
    solveFile("input.txt");
}
