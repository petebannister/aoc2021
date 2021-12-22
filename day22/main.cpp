#include "utils.h"

#include <future>

using namespace std;

using Voxels = std::unordered_set<Vec3>;
using Axis = num_range<int32_t>;

struct Cube {
    using R = Axis;
    R r[3];

    size_t hash(R const& rr) const {
        auto const h = std::hash<uint64_t>();
        uint64_t v = rr.start_;
        v <<= 32;
        v += rr.limit_;
        return h(v);
    }
    size_t hash() const {
        auto const h = std::hash<uint64_t>();
        return h(hash(r[0])) ^ h(hash(r[1])) ^ h(hash(r[2]));
    }

    auto begin() {
        return std::begin(r);
    }
    auto end() {
        return std::end(r);
    }
    auto begin() const {
        return std::begin(r);
    }
    auto end() const {
        return std::end(r);
    }
    R& operator[](size_t i) {
        return r[i];
    }
    R const& operator[](size_t i) const {
        return r[i];
    }

    bool intersects(Cube const& other) const {
        return (
            r[0].intersects(other[0]) &&
            r[1].intersects(other[1]) &&
            r[2].intersects(other[2]));
    }
    uint64_t size() const {
        return uint64_t(r[0].size()) * uint64_t(r[1].size()) * uint64_t(r[2].size());
    }
};
namespace std {
    template <>
    struct hash<Cube> {
        std::size_t operator()(const Cube& k) const {
            return k.hash();
        }
    };
}

// Could use an Octree, looks like a simple list will probably suffice
struct Cubes
{
    std::list<Cube> cubes_;

    void insert(Cube const& cube) {

    }
    void erase(Cube const& cube) {

    }
    uint64_t size() const {
        uint64_t r = 0;
        for (auto&& c : cubes_) {
            r += c.size();
        }
        return r;
    }
};



void solveFile(char const* fname) {
    TextFileIn f(fname);

    Voxels main_core;

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
            for (auto x : cube[0]) {
                for (auto y : cube[1]) {
                    for (auto z : cube[2]) {
                        if (on) {
                            main_core.insert(Vec3{ x,y,z });
                        }
                        else {
                            main_core.erase(Vec3{ x,y,z });
                        }
                    }
                }
            }
        }
        // TODO other massive cubes not needed for part 1.
    }


    // example: 590784
    // input: 615869
    print(main_core.size());
}

void main() {
    print("example");
    solveFile("example.txt");
    print("example2");
    solveFile("example2.txt");
    print("input");
    solveFile("input.txt");
}
