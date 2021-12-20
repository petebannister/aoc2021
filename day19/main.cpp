#include "utils.h"

#include <future>

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
    uint32_t manhattan() const {
        return abs(x) + abs(y) + abs(z);
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
    Vec3 operator- () const {
        return Vec3{
            -x,
            -y,
            -z };
    }
    Vec3& operator+= (Vec3 const& rhs) {
        x += rhs.x;
        y += rhs.y;
        z += rhs.z;
        return *this;
    }
    Vec3& operator-= (Vec3 const& rhs) {
        x -= rhs.x;
        y -= rhs.y;
        z -= rhs.z;
        return *this;
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

using Points = std::unordered_set<Vec3, Vec3Hash>;
struct Scan {
    Points points;
    vector<Vec3> vec;
};

using Scans = std::vector<Scan>;

struct Transform {
    uint32_t from;
    uint32_t to;
    Vec3 to_p;
    Vec3 from_p;
    Rotation rot;

    Vec3 operator*(Vec3 v) const {
        v -= from_p;
        v = rot * v;
        v += to_p;
        return v;
    }
};
using Matches = std::vector<Transform>;
bool MatchingPair(uint32_t to, uint32_t from, Scans const& scans, Transform& m)
{
    std::vector<Transform> result;
    auto& to_scan = scans[to];
    auto& from_scan = scans[from];
    for (auto& to_p : to_scan.points) {
        for (auto& from_p : from_scan.vec) {
            for (auto& rot : rotations) {
                int nmatch = 0;
                int rem = (int)from_scan.vec.size();
                for (Vec3 p : from_scan.vec) {
                    m.to_p = to_p;
                    m.from_p = from_p;
                    m.rot = rot;
                    p = m * p;
                    if (contains(to_scan.points, p)) {
                        ++nmatch;
                        if (nmatch >= 12) {
                            m.to = to;
                            m.from = from;
                            return true;
                        }
                    }
                    --rem;
                    if ((nmatch + rem) < 12) {
                        break;
                    }
                }
            }
        }
    }
    return false;
}


struct Joiner
{
    Points world;
    set<uint32_t> visited;
    deque<Transform> chain;
    vector<Vec3> scanners;
    void join(Scans const& scans) {
        world = scans[0].points;
        visited = { 0 };
        scanners.resize(scans.size());
        join(0, scans);
    }
    void join(uint32_t to, Scans const& scans) {
        vector<future<pair<bool, Transform>>> futures;
        vector<Transform> matches;
        auto& to_scan = scans[to];
        for (auto from : integers(scans.size())) {
            if (!contains(visited, uint32_t(from))) {
                futures.push_back(std::async(std::launch::async, [&scans,to,from] {

                    Transform m;
                    if (MatchingPair(to, from, scans, m)) {
                        return pair(true, m);
                    }
                    return pair(false, m);
                }));
            }
        }
        for (auto&& f : futures) {
            f.wait();
            auto [ok, trans] = f.get();
            if (ok) {
                matches.push_back(trans);
            }
        }
        for (auto&& m : matches) {
            visited.insert(m.from);
        }
        for (auto&& m : matches) {
            visited.insert(m.from);
            chain.push_front(m);
            insert(m.from, scans[m.from]);
            join(m.from, scans);
            chain.pop_front();
        }
    }
    void insert(uint32_t iscan, Scan const& scan) {
        Vec3 scanner = Vec3{ 0,0,0 };
        for (auto& m : chain) {
            scanner = m * scanner;
        }
        scanners[iscan] = scanner;

        for (Vec3 p : scan.points) {
            for (auto& m : chain) {
                p = m * p;
            }
            world.insert(p);
        }
    }

    uint32_t largestScannerDist() {
        uint32_t r = 0;
        for (auto i : integers(scanners.size())) {
            for (auto k : integers(i + 1, scanners.size())) {
                auto d = scanners[i] - scanners[k];
                amax(r, d.manhattan());
            }
        }
        return r;
    }
};

Points
BuildWorld(Scans const& scans) {
    Joiner j;
    j.join(scans);
    return j.world;
}

void solveFile(char const* fname) {
    TextFileIn f(fname);

    Scans scans;
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
            scans.back().vec.push_back(p);
        }
    }
    Joiner j;
    j.join(scans);
    print(j.world.size());
    print(j.largestScannerDist());
}

void main() {
    print("example");
    solveFile("example.txt");
    print("input");
    solveFile("input.txt");
}
