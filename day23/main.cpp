#include "utils.h"

using namespace std;

static const uint32_t costs[] = { 1, 10, 100, 1000 }; // A, B, C, D

static const uint8_t TOTAL_POSITIONS = 19u;

//-----------------------------------------------------------------------------
struct Positions
{
    union {
        uint8_t amphipod[8];
        uint64_t value; // Whole state can be represented as 64bit int.
    };
    Positions()
        : value(~0uLL)
    {}
    bool operator==(Positions const& rhs) const {
        return (value == rhs.value);
    }
    bool operator!=(Positions const& rhs) const {
        return (value != rhs.value);
    }
    size_t hash() const {
        return std::hash<uint64_t>()(value);
    }
    bool isComplete() const {
        return
            ((amphipod[0] / 2) == 0) &&
            ((amphipod[1] / 2) == 0) &&
            ((amphipod[2] / 2) == 1) &&
            ((amphipod[3] / 2) == 1) &&
            ((amphipod[4] / 2) == 2) &&
            ((amphipod[5] / 2) == 2) &&
            ((amphipod[6] / 2) == 3) &&
            ((amphipod[7] / 2) == 3);
    }
    static uint32_t stepCost(uint8_t amphi) {        
        return costs[amphi / 2];
    }
    static uint8_t homePos(uint8_t depth, uint8_t room) {
        return (room * 2) + depth;
    }
    static uint8_t hallPos(uint8_t pos) {
        return 8 + pos;
    }
    static uint8_t doorPos(uint8_t room) {
        return 10 + (room * 2);
    }
    static bool isHallPos(uint8_t pos) {
        return (pos >= 8);
    }
    static bool isDoorPos(uint8_t pos) {
        return
            (pos == 10) ||
            (pos == 12) ||
            (pos == 14) ||
            (pos == 16);

    }
    void init(char type, uint8_t depth, uint8_t room) {
        uint8_t pod_index = (type - 'A') * 2;
        if (amphipod[pod_index] == 0xFF) {
            amphipod[pod_index] = homePos(depth, room);
        }
        else {
            ++pod_index;
            assert(amphipod[pod_index] == 0xFF);
            amphipod[pod_index] = homePos(depth, room);
        }
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
            auto home = b / 2;
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

            auto door_pos = doorPos(home);
            auto dir = (door_pos < a) ? -1 : 1;
            auto p = a + dir;
            uint8_t steps = 2; // for first step + door to first home pos.
            for (; p != door_pos; p += dir) {
                if (isOccupied(p)) {
                    // blocked
                    return 0;
                }
                ++steps;
            }
            if (b != h0) {
                ++steps;
            }
            return steps;
        }
        else {
            if (isDoorPos(b)) {
                return 0; // cant stop at a door
            }
            auto home = a / 2;
            auto door_pos = doorPos(home);
            auto h0 = homePos(0, home);
            uint8_t steps = 1; // must do at least 1 move to get to door pos
            if (a != h0) {
                if (isOccupied(h0)) {
                    return 0; // invalid move - blocked.
                }
                ++steps;
            }
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
        auto home = amphi / 2;

        auto h1 = homePos(1, home);
        if (current == h1) {
            // Already home in best pos
            return 0;
        }
        auto h0 = homePos(0, home);
        if (current == h0) {
            if ((getOccupier(h1) / 2) == home) {
                return 0;
            }
        }
        if (isHallPos(current)) {
            if ((dest != h0) && (dest != h1)) {
                // Destination is not desired 'home' room.
                return 0;
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
    struct hash<Positions> {
        size_t operator()(Positions const& pos) const {
            return pos.hash();
        }
    };
}

struct Solver
{
    std::unordered_map<Positions, uint32_t> lookup;

    uint32_t solve(Positions pos) {
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
        for (auto amphi : integers(uint8_t(8))) {
            for (auto dest : integers(uint8_t(TOTAL_POSITIONS))) {
                Positions new_pos = pos;
                auto cost = new_pos.doMove(amphi, dest);
                if (cost != 0) {
                    auto child = solve(new_pos);
                    if (child != 0xFFFFFFFFuL) { // dead end
                        cost += solve(new_pos);
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

    Positions start;

    f.skipLine();
    f.skipLine();
    for (auto i : integers(2)) {
        auto L = f.readLine();
        start.init(L[3], i, 0);
        start.init(L[5], i, 1);
        start.init(L[7], i, 2);
        start.init(L[9], i, 3);
    }

    Solver sol;


    uint32_t part1 = sol.solve(start);
    uint32_t part2 = 0;

    print(part1);
    print(part2);
}

//-----------------------------------------------------------------------------
void main() {
    print("example");
    solveFile("example.txt");
    print("input");
    solveFile("input.txt");
}
