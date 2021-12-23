#include "utils.h"

using namespace std;

static const uint32_t costs[] = { 1, 10, 100, 1000 }; // A, B, C, D

static const uint8_t TOTAL_POSITIONS = 19u;

//-----------------------------------------------------------------------------
template <uint8_t N>
struct Positions
{
    static constexpr uint8_t N2 = N * 2;
    union {
        uint8_t amphipod[8 * N];
        uint64_t value[1 * N]; // Whole state can be represented as 64bit int.
    };
    Positions()
    {
        memset(value, 0xFF, sizeof(value));
    }
    bool operator==(Positions const& rhs) const {
        return (0 == memcmp(value, rhs.value, sizeof(value)));
    }
    bool operator!=(Positions const& rhs) const {
        return !(*this == rhs);
    }
    size_t hash() const {
        size_t r = 0;
        for (auto i : integers(N)) {
            r ^= std::hash<uint64_t>()(value[i]);
        }
        return r;
        return r;
    }
    bool isComplete() const {
        for (auto i : integers(N * 8)) {
            if ((amphipod[i] / N2) != (i / N2)) {
                return false;
            }
        }
        return true;
    }
    static uint32_t stepCost(uint8_t amphi) {        
        return costs[amphi / N2];
    }
    static uint8_t homePos(uint8_t depth, uint8_t room) {
        return (room * N2) + depth;
    }
    static uint8_t hallPos(uint8_t pos) {
        return (8 * N) + pos;
    }
    static uint8_t doorPos(uint8_t room) {
        return (8 * N) + 2 + (room * uint8_t(2));
    }
    static bool isHallPos(uint8_t pos) {
        return (pos >= (8 * N));
    }
    static bool isDoorPos(uint8_t pos) {
        return
            (pos == ((8 * N) + 2)) ||
            (pos == ((8 * N) + 4)) ||
            (pos == ((8 * N) + 6)) ||
            (pos == ((8 * N) + 8));

    }
    void init(char type, uint8_t depth, uint8_t room) {
        uint8_t pod_index = (type - 'A') * N2;
        while (amphipod[pod_index] != 0xFF) {
            ++pod_index;
        }
        assert(pod_index < (1 + (type - 'A')) * N2);
        amphipod[pod_index] = homePos(depth, room);
    }
    bool isOccupied(uint8_t pos) const {
        return contains(amphipod, pos);
    }
    uint8_t getOccupier(uint8_t pos) const {
        uint8_t index = 0;
        for (auto& a : amphipod) {
            if (a == pos) {
                break;
            }
            ++index;
        }
        return index;
    }
    uint8_t validStepsBetween(uint8_t a, uint8_t b) const {
        if (a == b) {
            return 0;
        }
        if (isHallPos(a) == isHallPos(b)) {
            return 0; // not valid
        }
        if (isOccupied(b)) {
            return 0;
        }
        if (isHallPos(a)) {
            auto home = b / N2;
#if 0
            auto h0 = homePos(0, home);
            if (isOccupied(h0)) {
                return 0;
            }
            if (b == h0) {
                auto h1 = homePos(1, home);
                // There is only ever any point going into h0 if h1 is filled with an amphipod of the same type;
                if (home != (getOccupier(h1) / 2)) {
                    // room has not been emptied of other amphipod types yet.
                    return 0;
                }
            }
#endif
            auto door_pos = doorPos(home);
            auto dir = (door_pos < a) ? -1 : 1;
            auto p = a + dir;
            uint8_t steps = 1;
            for (; p != door_pos; p += dir) {
                if (isOccupied(p)) {
                    // blocked
                    return 0;
                }
                ++steps;
            }
            for (auto ih : integers(N2)) {
                ++steps;
                auto hp = homePos(ih, home);
                if (isOccupied(hp)) {
                    return 0;
                }
                if (b == hp) {
                    for (auto ih2 : integers(uint8_t(ih + 1), N2)) {
                        auto hp2 = homePos(ih2, home);
                        if (!isOccupied(hp2)) {
                            return 0;
                        }
                        if ((getOccupier(hp2) / N2) != home) {
                            return 0;
                        }
                    }
                    break;
                }
            }
            return steps;
        }
        else {
            if (isDoorPos(b)) {
                return 0; // cant stop at a door
            }
            auto home = a / N2;
            auto ih = a % N2;

            uint8_t steps = 1; // must do at least 1 move to get to door pos
            while (ih > 0) {
                --ih;
                ++steps;
                auto hp = homePos(ih, home);
                if (isOccupied(hp)) {
                    return 0;
                }
            }
            auto door_pos = doorPos(home);
            auto dir = (door_pos < b) ? 1 : -1;
            auto p = door_pos;
            for (; p != b; p += dir) {
                if (isOccupied(p)) {
                    // blocked
                    return 0;
                }
                ++steps;
            }
            return steps;
        }
    }

    uint32_t moveCost(uint8_t amphi, uint8_t dest) const {
        auto current = amphipod[amphi];
        auto home = amphi / N2;

        if (isHallPos(current)) {
            bool is_own_home = false;
            for (auto& ih : integers(N2)) {
                auto p = homePos(ih, home);
                if (isOccupied(p)) {
                    if ((getOccupier(p) / N2) != home) {
                        // home occupied by another type of amphipod
                        return 0;
                    }
                }
                if (dest == p) {
                    is_own_home = true;
                }
            }
            if (!is_own_home) {
                return 0;
            }
        }
        else {
            // Is the current position already part of a solution
            auto ih = N2;
            while (ih--) {
                auto p = homePos(ih, home);
                if (current == p) {
                    return 0;
                }
                if ((getOccupier(p) / N2) != home) {
                    // home occupied by another type of amphipod - can move higher up pods out.
                    break;
                }
            }
        }

        uint32_t cost = validStepsBetween(current, dest);
        if (cost != 0) {
            auto sc = stepCost(amphi);
            return cost * sc;
        }
        return 0;
    }
    uint32_t doMove(uint8_t amphi, uint8_t dest) {
        auto cost = moveCost(amphi, dest);
        if (cost) {
            amphipod[amphi] = dest;
        }
        return cost;
    }

    void render() {
        char str[] =
            "#############\n"
            "#...........#\n"
            "###.#.#.#.###\n"
            "  #.#.#.#.#  \n"
            "  #########  \n";
        auto put = [&](char ch, uint8_t pos) {
            if (isHallPos(pos)) {
                str[15 + (pos - 8)] = ch;
            }
            else {
                auto i = 31;
                if (0 != (pos & 1)) {
                    i += 14;
                }
                str[i + ((pos / 2) * 2)] = ch;
            }
        };
        put('A', amphipod[0]);
        put('A', amphipod[1]);
        put('B', amphipod[2]);
        put('B', amphipod[3]);
        put('C', amphipod[4]);
        put('C', amphipod[5]);
        put('D', amphipod[6]);
        put('D', amphipod[7]);
        write(str);
    }
};

namespace std {
    template<>
    struct hash<Positions<1>> {
        size_t operator()(Positions<1> const& pos) const {
            return pos.hash();
        }
    };
    template<>
    struct hash<Positions<2>> {
        size_t operator()(Positions<2> const& pos) const {
            return pos.hash();
        }
    };
}

template <int N>
struct Solver
{
    std::unordered_map<Positions<N>, uint32_t> lookup;

    uint32_t solve(Positions<N> pos) {
        auto i = lookup.find(pos);
        if (i != lookup.end()) {
            return i->second;
        }
        //pos.render();
        if (pos.isComplete()) {
            //pos.render();
            lookup[pos] = 0;
            return 0;
        }
        uint32_t lowest = 0xFFFFFFFFuL;
        for (auto amphi : integers(uint8_t(N * 8))) {
            for (auto dest : integers(uint8_t(N * 8 + 11))) {
                Positions<N> new_pos = pos;
                auto cost = new_pos.doMove(amphi, dest);
                if (cost != 0) {
                    auto child = solve(new_pos);
                    if (child != 0xFFFFFFFFuL) { // dead end
                        cost += child;
                        amin(lowest, cost);
                    }
                }
            }
        }
        lookup[pos] = lowest;
        return lowest;
    }
};

//-----------------------------------------------------------------------------
void solveFile(char const* fname) {
    TextFileIn f(fname);

    Positions<1> start;
    Positions<2> start2;

    f.skipLine();
    f.skipLine();
    for (auto i : integers(2)) {
        auto L = f.readLine();
        start.init(L[3], i, 0);
        start.init(L[5], i, 1);
        start.init(L[7], i, 2);
        start.init(L[9], i, 3);

        if (i == 0) {
            start2.init(L[3], i, 0);
            start2.init(L[5], i, 1);
            start2.init(L[7], i, 2);
            start2.init(L[9], i, 3);
        }
        else if (i == 1) {
            start2.init('D', i, 0);
            start2.init('C', i, 1);
            start2.init('B', i, 2);
            start2.init('A', i, 3);

            start2.init('D', i + 1, 0);
            start2.init('B', i + 1, 1);
            start2.init('A', i + 1, 2);
            start2.init('C', i + 1, 3);

            start2.init(L[3], i + 2, 0);
            start2.init(L[5], i + 2, 1);
            start2.init(L[7], i + 2, 2);
            start2.init(L[9], i + 2, 3);
        }

    }

    Solver<1> sol1;
    Solver<2> sol2;

    uint32_t part1 = sol1.solve(start);
    print(part1);
    uint32_t part2 = sol2.solve(start2);
    print(part2);
}

//-----------------------------------------------------------------------------
void main() {
    print("example");
    solveFile("example.txt");
    print("input");
    solveFile("input.txt");
}
