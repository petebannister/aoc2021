#include "utils.h"

using namespace std;

using Coord = int32_t;

struct Vec3 {
    union {
        struct {
            Coord x;
            Coord y;
            Coord z;
        };
        Coord v[3];
    };
    size_t hash() const {
        // Coords don't get that large - just join them into 64bit int abd use standard hash.
        uint64_t v =
            (uint64_t(x) << 24) +
            (uint64_t(y) << 48) +
            (uint64_t(z));
        return std::hash<uint64_t>()(v);
    }
    Vec3& operator+ (Vec3 const& rhs) const {
        return Vec3{ 
            x + rhs.x,
            y + rhs.y,
            z + rhs.z };
    }
    Vec3& operator- (Vec3 const& rhs) const {
        return Vec3{
            x - rhs.x,
            y - rhs.y,
            z - rhs.z };
    }
    Vec3& operator+= (Vec3 const& rhs) {
        x += rhs.x;
        y += rhs.y;
        z += rhs.z;
    }
    Vec3& operator-= (Vec3 const& rhs) {
        x -= rhs.x;
        y -= rhs.y;
        z -= rhs.z;
    }
    Coord& operator[](size_t i) {
        return v[i];
    }
    Coord const operator[](size_t i) const {
        return v[i];
    }
    auto fields() const {
        return std::tie(x, y, z);
    }
    bool operator==(Vec3 const& rhs) const {
        return fields() == rhs.fields();
    }
    bool operator<(Vec3 const& rhs) const {
        return fields() < rhs.fields();
    }
    bool operator!=(Vec3 const& rhs) const {
        return !(*this == rhs);
    }
};
struct Vec3Hash {
    std::size_t operator()(const Vec3& k) const {
        return k.hash();
    }
};

#if 0
struct Mat3 {
    Vec3 m[3];
    Vec3 operator*(Vec3 v) const {
        return {
            (m[0][0] * v.x) + (m[0][1] * v.y) + (m[0][2] * v.z),
            (m[1][0] * v.x) + (m[1][1] * v.y) + (m[1][2] * v.z),
            (m[2][0] * v.x) + (m[2][1] * v.y) + (m[2][2] * v.z),
        };
    }
};
#endif

// Only need 90deg rotations.. can avoid having a full matrix for that.
// Need 2 rotations to get all 24 possibilities.
struct Rotation
{
    static void r90(Coord& x, Coord& y) {
        auto t = x;
        x = -y;
        y = t;
    }
    static void r180(Coord& x, Coord& y) {
        x = -x;
        y = -y;
    }
    static void r270(Coord& x, Coord& y) {
        auto t = x;
        x = y;
        y = -t;
    }
    static void rot(uint8_t n, Coord& x, Coord& y) {
        switch (n) {
        case 0: break;
        case 1: r90(x, y); break;
        case 2: r180(x, y); break;
        case 3: r270(x, y); break;
        default:
            assert(false);
            break;
        }
    }

    uint8_t rx = 0;
    uint8_t ry = 0;
    uint8_t rz = 0;

    Vec3 operator*(Vec3 v) const {
        rot(rx, v.y, v.z);
        rot(ry, v.x, v.z);
        rot(rz, v.x, v.y);
        return v;
    }
};

std::vector<Rotation> BuildRotations() {
    std::vector<Rotation> all;
    std::unordered_set<Vec3, Vec3Hash> distinct;
    for (auto z = 0; z < 4; ++z) {
        for (auto y = 0; y < 4; ++y) {
            for (auto x = 0; x < 4; ++x) {
                Rotation r{ x, y, z };
                auto v = Vec3{ 1, 2, 3 };
                v = r * v;
                if (!contains(distinct, v)) {
                    distinct.insert(v);
                    all.push_back(r);
                }
            }
        }
    }
    return all;
}

static const std::vector<Rotation> rotations = BuildRotations();

struct Scan {
    std::unordered_set<Vec3, Vec3Hash> points;
};

void solveFile(char const* fname) {
    TextFileIn f(fname);

    std::vector<Scan> scans;
    scans.reserve(64);

    for (auto line : f.lines()) {
        if (line.startsWith("---")) {
            scans.push_back({});
        }
        else if (!line.empty()) {
            Vec3 p;
            p.x = line.split(',').parseInt32();
            p.y = line.split(',').parseInt32();
            p.z = line.parseInt32();
            scans.back().points.insert(p);
        }
    }
    print(0);
}

void main() {
    print("example");
    solveFile("example.txt");
    print("input");
    solveFile("input.txt");
}
