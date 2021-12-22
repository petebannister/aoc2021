#include "utils.h"

using namespace std;

//-----------------------------------------------------------------------------
using Axis = num_range<int32_t>;

//-----------------------------------------------------------------------------
struct Cube {
    Axis a_[3];
    auto begin() {
        return std::begin(a_);
    }
    auto end() {
        return std::end(a_);
    }
    auto begin() const {
        return std::begin(a_);
    }
    auto end() const {
        return std::end(a_);
    }
    Axis& operator[](size_t i) {
        return a_[i];
    }
    Axis const& operator[](size_t i) const {
        return a_[i];
    }
    bool intersects(Cube const& other) const {
        return (
            a_[0].intersects(other[0]) &&
            a_[1].intersects(other[1]) &&
            a_[2].intersects(other[2]));
    }
    bool contains(Cube const& other) const {
        return (
            a_[0].contains(other[0]) &&
            a_[1].contains(other[1]) &&
            a_[2].contains(other[2]));
    }
    uint64_t size() const {
        return uint64_t(a_[0].size()) * uint64_t(a_[1].size()) * uint64_t(a_[2].size());
    }
    bool empty() const {
        return (size() == 0u);
    }
    pair<Cube, Cube> split(uint8_t axis, int32_t pos) const {
        auto& a = a_[axis];
        if (pos > a.start_ && pos < a.limit_) {
            auto r1 = *this;
            auto r2 = *this;
            r1.a_[axis] = { a.start_, pos };
            r2.a_[axis] = { pos, a.limit_ };
            return { r1, r2 };
        }
        return { *this, {} };
    }
};
//-----------------------------------------------------------------------------
// Could use an Octree, looks like a linked list will probably suffice
struct Cubes
{
    list<Cube> cubes_;

    void insert(Cube const& cube) {
        erase(cube);
        cubes_.push_back(cube);
    }
    void erase(Cube const& cube) {
        auto i = cubes_.begin();
        auto e = cubes_.end();
        while (i != e) {
            auto& c = *i;
            if (c.intersects(cube)) {
                if (cube.contains(c)) {
                    auto t = i;
                    ++i;
                    cubes_.erase(t);
                }
                else { // split
                    Cubes sub;
                    auto t = i;
                    ++i;
                    sub.cubes_.splice(sub.cubes_.end(), cubes_, t);
                    sub.split(cube);
                    sub.cubes_.remove_if([&](auto&& s) { return cube.contains(s); });
                    cubes_.splice(i, sub.cubes_);
                }
            }
            else {
                ++i;
            }
        }
    }
    void split(Cube const& cube) {
        split(0, cube);
        split(1, cube);
        split(2, cube);
    }
    void split(uint8_t axis, Cube const& cube) {
        auto i = cubes_.begin();
        auto e = cubes_.end();
        while (i != e) {
            auto& c = *i;
            auto a = c.split(axis, cube[axis].start_);
            if (a.second.empty()) {
                auto b = a.first.split(axis, cube[axis].limit_);
                c = b.first;
                ++i;
                if (!b.second.empty()) {
                    i = cubes_.insert(i, b.second);
                    ++i;
                }
            }
            else {
                c = a.first;
                ++i;
                auto b = a.second.split(axis, cube[axis].limit_);
                i = cubes_.insert(i, b.first);
                ++i;
                if (!b.second.empty()) {
                    i = cubes_.insert(i, b.second);
                    ++i;
                }
            }
        }
    }
    uint64_t size() const {
        uint64_t tot = 0;
        for (auto&& c : cubes_) {
            tot += c.size();
        }
        return tot;
    }
};

//-----------------------------------------------------------------------------
void solveFile(char const* fname) {
    TextFileIn f(fname);

    Cubes main_core;
    Cubes all;

    Cube main_core_cube{
        Axis{-50, 51},
        Axis{-50, 51},
        Axis{-50, 51},
    };

    for (auto line : f.lines()) {
        bool on = line.startsWith("on");
        if (on) {
            line = line.mid(3);
        }
        else if (!line.empty()) {
            line = line.mid(4);
        }
        Cube cube;
        for (auto& a : cube) {
            line.split('=');
            a.start_ = line.split('.').parseInt32();
            line.pop_front();
            a.limit_ = 1 + line.split(',').parseInt32();
        }

        if (main_core_cube.intersects(cube)) {
            if (on) {
                main_core.insert(cube);
            }
            else {
                main_core.erase(cube);
            }
        }
        if (on) {
            all.insert(cube);
        }
        else {
            all.erase(cube);
        }
    }

    print(main_core.size());
    print(all.size());
}

//-----------------------------------------------------------------------------
void main() {
    print("example");
    solveFile("example.txt");
    print("example2");
    solveFile("example2.txt");
    print("input");
    solveFile("input.txt");
}
