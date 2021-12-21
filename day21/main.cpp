#include "utils.h"

#include <future>

using namespace std;

struct Dice {
    uint8_t a;
    uint8_t b;
};

void solveFile(char const* fname) {
    TextFileIn f(fname);

    Dice dice;

    StringView sv;
    f.readLine(sv);
    sv.split(':');
    sv.pop_front();
    dice.a = sv.parseInt32();

    f.readLine(sv);
    sv.split(':');
    sv.pop_front();
    dice.b = sv.parseInt32();


    print(0);
    print(0);
}

void main() {
    print("example");
    solveFile("example.txt");
    print("input");
    solveFile("input.txt");
}
