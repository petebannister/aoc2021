#include "utils.h"

using namespace std;

//inp a - Read an input value and write it to variable a.
//add a b - Add the value of a to the value of b, then store the result in variable a.
//mul a b - Multiply the value of a by the value of b, then store the result in variable a.
//div a b - Divide the value of a by the value of b, truncate the result to an integer, then store the result in variable a. (Here, "truncate" means to round the value toward zero.)
//mod a b - Divide the value of a by the value of b, then store the remainder in variable a. (This is also called the modulo operation.)
//eql a b - If the value of a and b are equal, then store the value 1 in variable a.Otherwise, store the value 0 in variable a.

struct ModelNumber {
    uint8_t v[14];
    ModelNumber(uint64_t i = 0) {
        auto x = 14;
        while (x--) {
            v[x] = i % 10;
            i /= 10;
        }
    }
};

struct Checker {
    using T = int32_t;
    ModelNumber num_;
    uint8_t ipos = 0;
    T w = {};
    T x = {};
    T y = {};
    T z = {};
    bool fail = false;

    void inp(T& r) {
        r = num_.v[ipos++];
        if (r == 0) {
            fail = true;
            //throw logic_error("inp");
        }
    }
    void add(T& a, T const& b) {
        a = a + b;
    }
    void mul(T& a, T const& b) {
        a = a * b;
    }
    void div(T& a, T const& b) {
        if (b == 0) {
            fail = true;
            //throw logic_error("div0");
        }
        else {
            a = a / b;
        }
    }
    void mod(T& a, T const& b) {
        if (a < 0) {

            fail = true;
            return;
            //throw logic_error("mod neg");
        }
        if (b == 0) {
            fail = true;
            return;
            //throw logic_error("mod div0");
        }
        a = a % b;
    }
    void eql(T& a, T const& b) {
        a = (a == b) ? 1 : 0;
    }
    T run(ModelNumber num) {
        num_ = num;
        return run();
    }
    T run() {
        fail = false;
        ipos = 0;
        w = {};
        x = {};
        y = {};
        z = {};
        // modified input so that the compiler can parse it
#include "input.h"
        return z;
    }
};

struct Solver {
    using T = vector<int32_t>;
    ModelNumber num_;
    uint8_t ipos = 0;
    T w = {};
    T x = {};
    T y = {};
    T z = {};
    T tmp;

    void inp(T& r) {
        r = { 1,2,3,4,5,6,7,8,9 };
    }
    void add(T& a, int32_t const& b) {
        for (auto& v : a) {
            v += b;
        }
    }
    void add(T& a, T const& b) {
        tmp.clear();
        for (auto& v : a) {
            for (auto& v2 : b) {
                tmp.push_back(v + v2);
            }
        }
        a.swap(tmp);
    }
    void mul(T& a, int32_t b) {
        for (auto& v : a) {
            v *= b;
        }
    }
    void mul(T& a, T const& b) {
        tmp.clear();
        for (auto& v : a) {
            for (auto& v2 : b) {
                tmp.push_back(v * v2);
            }
        }
        a.swap(tmp);
    }
    void div(T& a, T const& b) {
        tmp.clear();
        for (auto& v : a) {
            for (auto& v2 : b) {
                if (v2 != 0) {
                    tmp.push_back(v / v2);
                }
            }
        }
        a.swap(tmp);
    }
    void div(T& a, int32_t b) {
        if (b == 0) {
            a.clear();
        }
        else {
            for (auto& v : a) {
                v /= b;
            }
        }
    }
    void mod(T& a, T const& b) {
        tmp.clear();
        for (auto& v : a) {
            if (v >= 0) {
                for (auto& v2 : b) {
                    if (v2 != 0) {
                        tmp.push_back(v % v2);
                    }
                }
            }
        }
        a.swap(tmp);
    }
    void mod(T& a, int32_t b) {
        if (b == 0) {
            a.clear();
        }
        else {
            tmp.clear();
            for (auto& v : a) {
                if (v >= 0) {
                    tmp.push_back(v % b);
                }
            }
            a.swap(tmp);
        }
    }
    void eql(T& a, T const& b) {
        bool has0 = false;
        bool has1 = false;
        for (auto& v : a) {
            for (auto& v2 : b) {
                if (v == v2) {
                    has1 = true;
                }
                else {
                    has0 = true;
                }
            }
        }
        a.clear();
        if (has0) {
            a.push_back(0);
        }
        if (has1) {
            a.push_back(1);
        }
    }
    void eql(T& a, int32_t b) {
        bool has0 = false;
        bool has1 = false;
        for (auto& v : a) {
            if (v == b) {
                has1 = true;
            }
            else {
                has0 = true;
            }
        }
        a.clear();
        if (has0) {
            a.push_back(0);
        }
        if (has1) {
            a.push_back(1);
        }
    }
    T run(ModelNumber num) {
        num_ = num;
        return run();
    }
    T run() {
        ipos = 0;
        w = {0};
        x = {0};
        y = {0};
        z = {0};
        // modified input so that the compiler can parse it
#include "input.h"
        return z;
    }
};
//-----------------------------------------------------------------------------
void main() {

    // optimize out known constants
    // convert add to assign where appropriate
    // build an equation in form 0 = ...


    auto num = 99999999999999uLL;
    while (num > 11111111111111uLL) {
        try {
            Checker c;
            if (0 == c.run(num) && !c.fail) {
                break;
            }
        }
        catch (...) {}
        --num;
    }
    //auto z = c.run(13579246899999uLL);
    //Solver s;
    //auto z = s.run(13579246899999uLL);
    print(num);
}
