#include "utils.h"

using namespace std;


void solveFile(char const* fname) {
    TextFileIn f(fname);

    StringView line;
    f.readLine(line);
    line.split('=');
    auto xmin = line.split('.').parseInt32();
    line.pop_front();
    auto xmax = line.split(',').parseInt32();
    line.split('=');
    auto ymin = line.split('.').parseInt32();
    line.pop_front();
    auto ymax = line.split(',').parseInt32();
    
    int32_t bestx = 0;
    int32_t besty = 0;
    int32_t besth = 0;


    std::set<Point> distinct;

    for (auto init_vx : integers(xmax + 1)) {
        auto vx = init_vx;
        auto px = 0;
        uint32_t steps = 0;
        while (px <= xmax) {
            if (px >= xmin) {

                int32_t init_vy = ymin;
                while (1) {
                    int32_t top = 0;
                    int32_t vy = init_vy;
                    auto py = 0;
                    for (auto step : integers(steps)) {
                        py += vy;
                        vy -= 1;
                        amax(top, py);
                    }
                    while (py >= ymin) {
                        if (py >= ymin && py <= ymax) {
                            if (top > besth) {
                                besth = top;
                                bestx = init_vx;
                                bestx = init_vy;
                            }
                            distinct.insert({ init_vx, init_vy });
                        }
                        if (vx != 0) {
                            break;
                        }
                        else {
                            py += vy;
                            vy -= 1;
                            amax(top, py);
                            if (py < ymin) {
                                break;
                            }
                        }
                    }
                    // The Y position will always return to 0 exactly - so if 
                    // the velocity exceeds ymin then there will not be a hit today.
                    // 
                    if (init_vy > (-ymin + 1)) {
                        break;
                    }
                    ++init_vy;
                }
            }
            if (vx == 0) {
                break;
            }
            px += vx;
            // drag
            if (vx > 0) {
                vx -= 1;
            }
            ++steps;
        }
    }
    
    print(besth);
    print(distinct.size());
}

void main() {
    print("example");
    solveFile("example.txt");
    print("input");
    solveFile("input.txt");
}
