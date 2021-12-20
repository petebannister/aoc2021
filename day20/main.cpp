#include "utils.h"

#include <future>

using namespace std;

using Image = unordered_set<Point>;

static constexpr Point window[] = {
    {-1,-1},{0,-1},{1,-1},
    {-1, 0},{0, 0},{1, 0},
    {-1, 1},{0, 1},{1, 1},
};
struct Filter
{
    std::vector<bool> lookup;
    mutable Image pending;
    mutable  Image out;

    void iterate(Image& img, uint32_t n)
    {
        assert(lookup.front() != lookup.back());

        for (auto i : integers(n)) {
            step(img, (0 == (i & 1)));
        }
    }
    void step(Image& img, bool even) const
    {
        bool odd = !even;
        bool inverting =  lookup.front();

        pending.clear();
        out.clear();
        for (auto& p : img) {
            for (auto&& w : window) {
                pending.insert(p + w);
            }
        }
#if 0
        for (auto i : integers(10)) {
            for (auto& p : pending) {
                for (auto&& w : window) {
                    out.insert(p + w);
                }
            }
            pending.swap(out);
            out.clear();
        }
#endif

        for (auto& p : pending) {
            uint16_t i = 0u;
            for (auto&& w : window) {
                i <<= 1;
                if (contains(img, p + w)) {
                    i |= 1;
                }
            }
            if (inverting && odd) {
                // previous step swapped dark for light
                i = (~i) & 0x1FFu;
            }
            bool u = lookup[i]; // using 'auto' here is a reference due to vector bool retruning a proxy!!
            if (inverting) {
                if (even) {
                    u = !u; // insert dark
                }
            }
            if (u) {
                out.insert(p);
            }
        }
        img.swap(out);
    }
};

void render(Image const& img) {
    Bounds b;
    for (auto& p : img) {
        b.add(p);
    }
    b.print_f([&](Point const& p) {
        return contains(img, p) ? '#' : ' ';
        });
}

void solveFile(char const* fname) {
    TextFileIn f(fname);

    StringView sv;
    f.readLine(sv);

    Filter filter;
    filter.lookup.reserve(sv.size());
    for (auto ch : sv) {
        filter.lookup.push_back(ch == '#');
    }
    f.readLine(sv); // skip

    Image start;
    int32_t y = 0;
    for (auto line : f.lines()) {
        int32_t x = 0;
        for (auto ch : line) {
            if (ch == '#') {
                start.insert({ x, y });
            }
            ++x;
        }
        ++y;
    }
    Image img = start;
    filter.iterate(img, 2);

    Image img2 = start;
    filter.iterate(img2, 50);

    print(img.size());
    print(img2.size());
}

void main() {
    print("example");
    solveFile("example.txt");
    print("input");
    solveFile("input.txt");
}
