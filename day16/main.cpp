#include "utils.h"

using namespace std;

using Bits = vector<bool>;
using Biterator = Bits::const_iterator;
Bits ParseBits(StringView s) {
    Bits bits;
    bits.reserve(s.size() * 4);
    auto bs = s.take(1);
    while (!bs.empty()) {
        auto b = bs.parseNibbleHex();
        bs = s.take(1);
        bits.push_back((b >> 3) & 1);
        bits.push_back((b >> 2) & 1);
        bits.push_back((b >> 1) & 1);
        bits.push_back((b >> 0) & 1);
    }
    return bits;
}

struct Packet
{
    std::vector<uint8_t> bytes;

    static constexpr uint8_t ID_LITERAL = 4u;

    uint8_t id;
    uint8_t version;
    std::vector<Packet> sub;
    uint64_t value = 0;

    uint16_t read(uint8_t n, Biterator& i) {
        uint16_t r = 0;
        while (n--) {
            r <<= 1;
            r += (*i) ? 1 : 0;
            ++i;
        }
        return r;
    }
    Biterator parse(Biterator i, Biterator e) {
        version = (uint8_t)read(3, i);
        id = (uint8_t)read(3, i);
        if (id == ID_LITERAL) {
            while (i != e) {
                auto surrogate = read(5, i);
                value <<= 4;
                value += surrogate & 0xF;
                if (0 == (surrogate & 0x10)) {
                    break;
                }
            }
        }
        else {
            auto type = read(1, i);
            if (type == 0) {
                auto len = read(15, i);
                auto e2 = i + len;
                while (i != e2) {
                    sub.push_back({});
                    i = sub.back().parse(i, e2);
                }
                i = e2;
            }
            else {
                auto count = read(11, i);
                sub.reserve(count);
                while (count--) {
                    sub.push_back({});
                    i = sub.back().parse(i, e);
                }
            }
        }
        return i;
    }
    uint32_t versionSum() const {
        uint32_t sum = version;
        for (auto&& s : sub) {
            sum += s.versionSum();
        }
        return sum;
    }
    uint64_t operatorSum() const {
        uint64_t r = 0;
        switch (id) {
        case 0: // sum
            for (auto&& p : sub) {
                r += p.operatorSum();
            }
            return r;
        case 1: // product
            r = 1;
            for (auto&& p : sub) {
                r *= p.operatorSum();
            }
            return r;
        case 2: // min
            r = sub[0].operatorSum();
            for (auto&& p : sub) {
                amin(r, p.operatorSum());
            }
            return r;
        case 3: // max
            r = sub[0].operatorSum();
            for (auto&& p : sub) {
                amax(r, p.operatorSum());
            }
            return r;
        case 4: // literal
            return value;
        case 5: // greater
            r = (sub[0].operatorSum() > sub[1].operatorSum()) ? 1 : 0;
            return r;
        case 6: // less
            r = (sub[0].operatorSum() < sub[1].operatorSum()) ? 1 : 0;
            return r;
        case 7: // equal
            r = (sub[0].operatorSum() == sub[1].operatorSum()) ? 1 : 0;
            return r;
        }
        return r;
    }
};

void solveFile(char const* fname) {
    TextFileIn f(fname);
    for (auto line : f.lines()) {
        auto bits = ParseBits(line);
        Packet pkt;
        pkt.parse(bits.begin(), bits.end());
        print(pkt.versionSum());
        print(pkt.operatorSum());
    }
    // must be more than 654
}

void main() {
    print("example");
    solveFile("example.txt");
    print("input");
    solveFile("input.txt");
}
