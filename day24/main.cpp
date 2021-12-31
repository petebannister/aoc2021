#include "utils.h"

using namespace std;


struct ModelNumber {
    uint8_t a[14];
    ModelNumber(uint64_t i = 0)
    {
        auto x = 14;
        while (x--) {
            a[x] = i % 10;
            i /= 10;
        }
    }
    uint64_t value() const {
        uint64_t r = 0;
        auto x = 14;
        for (auto x : integers(14)) {
            r *= 10;
            r += a[x];
        }
        return r;
    }
    uint8_t operator[](size_t index) const {
        return a[index];
    }
};

//inp a - Read an input value and write it to variable a.
//add a b - Add the value of a to the value of b, then store the result in variable a.
//mul a b - Multiply the value of a by the value of b, then store the result in variable a.
//div a b - Divide the value of a by the value of b, truncate the result to an integer, then store the result in variable a. (Here, "truncate" means to round the value toward zero.)
//mod a b - Divide the value of a by the value of b, then store the remainder in variable a. (This is also called the modulo operation.)
//eql a b - If the value of a and b are equal, then store the value 1 in variable a.Otherwise, store the value 0 in variable a.

enum class Op
{
    lit, inp, add, mul, div, mod, eql,
    neq
};

// Inclusive range
struct Range {
    int64_t min = 0;
    int64_t max = 0;
    bool intersects(Range const& other) const {
        return ((other.min <= max) && (other.max >= min));
    }
    int64_t size() const {
        return (max - min) + 1;
    }
    bool operator==(Range const& rhs) const {
        return min == rhs.min && max == rhs.max;
    }
};


struct AstNode
{
    static list<AstNode> arena;
    Op op = Op::lit;
    int64_t value = 0;
    AstNode* a = nullptr;
    AstNode* b = nullptr;
    Range range;

    std::string toStringBinary(char const* o) const {
        return a->toString() + " " + o + " " + b->toString();
    }
    std::string toString() const {
        switch (op) {
        case Op::lit: return std::to_string(value);
        case Op::inp: return "M[" + std::to_string(value) + "]";
        case Op::add: return "(" + a->toString() + " + " + b->toString() + ")";
        case Op::mul: return toStringBinary("*");
        case Op::div: return toStringBinary("/");
        case Op::mod: return toStringBinary("%");
        case Op::eql: return toStringBinary("==");
        case Op::neq: return toStringBinary("!=");
        default:
            throw logic_error("tostring case");
        }
    }
    int64_t calc_i(ModelNumber const& m) const {
        switch (op) {
        case Op::lit: return value;
        case Op::inp: return m[value];
        case Op::add: return a->calc(m) + b->calc(m);
        case Op::mul: return a->calc(m) * b->calc(m);
        case Op::div: return a->calc(m) / b->calc(m);
        case Op::mod: return a->calc(m) % b->calc(m);
        case Op::eql: return (a->calc(m) == b->calc(m)) ? 1 : 0;
        case Op::neq: return (a->calc(m) != b->calc(m)) ? 1 : 0;
        default:
            throw logic_error("calc case");
        }
    }
    int64_t calc(ModelNumber const& m) const {
        auto v = calc_i(m);
        return v;
    }

    int64_t calc() const {
        return calc(0);
    }
    void updateRange() {
        switch (op) {
        case Op::lit: 
            range = { value, value };
            break;
        case Op::inp:
            range = { 1, 9 };
            break;
        case Op::add:
            range = a->range;
            range.min += b->range.min;
            range.max += b->range.max;
            break;
        case Op::mul:
        {
            auto ra = a->range.min * b->range.min;
            auto rb = a->range.min * b->range.max;
            auto rc = a->range.max * b->range.min;
            auto rd = a->range.max * b->range.max;
            range.min = min(min(ra, rb), min(rc, rd));
            range.max = max(max(ra, rb), max(rc, rd));
        }
            break;
        case Op::div:
            range = a->range;
            // what about possible zero div?
            if (b->range.min == 0) {
                range.min /= b->range.max;
                //range.max;
            }
            else {
                auto ra = a->range.min / b->range.min;
                auto rb = a->range.min / b->range.max;
                auto rc = a->range.max / b->range.min;
                auto rd = a->range.max / b->range.max;
                range.min = min(min(ra, rb), min(rc, rd));
                range.max = max(max(ra, rb), max(rc, rd));
            }
            break;
        case Op::mod:
            assert(b->op == Op::lit); // assumption about possible input
            if (a->range.max < b->value) {
                range = a->range;
            }
            else {
                range = { 0, b->value - 1 };
            }
            break;
        case Op::eql:
            if (a->range.intersects(b->range)) {
                if ((a->range.size() == 1) && (a->range == b->range)) {
                    range = { 1, 1 };
                }
                else {
                    range = { 0, 1 };
                }
            }
            else {
                range = { 0, 0 };
            }
            break;
        case Op::neq:
            if (a->range.intersects(b->range)) {
                if ((a->range.size() == 1) && (a->range == b->range)) {
                    range = { 0, 0 };
                }
                else {
                    range = { 0, 1 };
                }
            }
            else {
                range = { 1, 1 };
            }
            break;
        default:
            throw logic_error("updateRange case");
        }
    }
    void optimize();
    void setLiteral(int64_t v) {
        value = v;
        a = nullptr;
        b = nullptr;
        op = Op::lit;
        range = { v, v };
    }
    AstNode* refactorMod(int64_t modv) {
        if (op == Op::add) {
            auto* da = a->refactorMod(modv);
            auto* db = b->refactorMod(modv);
            if (da || db) {
                auto* n = alloc(op);
                n->a = (da) ? da : a;
                n->b = (db) ? db : b;
                return n;
            }
        }
        else if (op == Op::mul) {
            if (b->op == Op::lit) {
                if (b->value == modv) {
                    return zero();
                }
            }
        }
        return nullptr;
    }
    static AstNode* alloc(Op _op, int64_t _v = 0){
        arena.push_back({});
        auto& n = arena.back();
        n.op = _op;
        n.value = _v;
        n.range = { _v, _v };
        return &n;
    }

    static AstNode* zero() {
        static auto n = alloc(Op::lit, 0);
        return n;
    }

#if 0
    deque<ModelNumber> solutions(uint64_t expect, ModelNumber m) {
        deque<ModelNumber> r;
        switch (op) {
        case Op::neq:
            if (b->op == Op::inp) {
                if (expect == 0) {

                }
                else {

                }
            }
            break;
        case Op::add:
            if (a->op == Op::inp) {
                if (b->op == Op::lit) {
                    auto v = expect - b->value;
                    if (v >= 1 && v <= 9) {
                        m.a[a->value] = expect - b->value;
                        r.push_back(m);
                    }
                    return r;
                }
            }
            break;

        default:
            break;
        }
        //throw runtime_error("solutions: unhandled");
        return{};
    }
#endif
    
};

list<AstNode> AstNode::arena;

struct Stringizer
{
    map<AstNode*, char> node_vars;
    set<AstNode*> visited;
    vector<AstNode*> vars;
    char var = 'a';
    string result;
    void buildVars(AstNode* node) {
        if (contains(visited, node)) {
            if (node->op != Op::lit && node->op != Op::inp) {
                if (!contains(node_vars, node)) {
                    node_vars[node] = var++;
                    vars.push_back(node);
                }
            }
        }
        else {
            if (node->a) {
                buildVars(node->a);
            }
            if (node->b) {
                buildVars(node->b);
            }
        }
        visited.insert(node);
    }
    string toString(AstNode* node) {
        buildVars(node);
        string result;
        for (auto* var : vars) {
            result += node_vars[var];
            result += " = " + toString_i(var);
            result += "\t\t{" + to_string(var->range.min) + "..." + to_string(var->range.max) + "}" ;
            result += "\n";
        }
        result += toString_i(node);
        return result;
    }
    string binary(AstNode* node, char const* op) {
        string r="(";
        if (auto* av = map_value(node_vars, node->a)) {
            r += *av;
        }
        else {
            r += toString_i(node->a);
        }
        r += " ";
        r += op;
        r += " ";
        if (auto* bv = map_value(node_vars, node->b)) {
            r += *bv;
        }
        else {
            r += toString_i(node->b);
        }
        r += ")";
        return r;

    }
    string toString_i(AstNode* node) {
        switch (node->op) {
        case Op::lit: return node->toString();
        case Op::inp: return node->toString();
        case Op::add: return binary(node, "+");
        case Op::mul: return binary(node, "*");
        case Op::div: return binary(node, "/");
        case Op::mod: return binary(node, "%");
        case Op::eql: return binary(node, "==");
        case Op::neq: return binary(node, "!=");
        default:
            throw logic_error("toString_i case");
        }
    }
};

void AstNode::optimize() {
    if (op == Op::lit) {
        updateRange();
        return;
    }
    if (op == Op::inp) {
        updateRange();
        return;
    }
    assert(!!a);
    assert(!!b);
    a->optimize();
    b->optimize();
    updateRange();

    if ((op != Op::lit) && (range.size() == 1)) {
        setLiteral(range.min);
        return;
    }

    if (op == Op::eql) {
        if (!a->range.intersects(b->range)) {
            // never equal
            setLiteral(0);
            return;
        }
        if ((b->op == Op::lit) && (a->op == Op::eql)) {
            if (b->value == 0) {
                *this = *a;
                op = Op::neq;
            }
        }
    }
    if (op == Op::mod) {
        // are there any terms that are multiplied by the modulus that could be optimized out?
        if (auto* da = a->refactorMod(b->value)) {
            a = da;
            a->updateRange();
        }
        else if (a->range.max < b->value) {
            *this = *a;
            updateRange();
            if (!a) {
                return;
            }
        }
    }
    if (a->op == Op::lit) {
        if (b->op == Op::lit) {
            setLiteral(calc());
        }
        else {
            if (op == Op::mul) {
                if (a->value == 0) {
                    setLiteral(0);
                }
                else if (a->value == 1) {
                    *this = *b;
                }
            }
            else if (op == Op::div) {
                if (a->value == 0) {
                    setLiteral(0);
                }
            }
            else if (op == Op::mod) {
                if (a->value == 0) {
                    setLiteral(0);
                }
            }
            else if (op == Op::add) {
                if (a->value == 0) {
                    *this = *b;
                }
            }
        }
    }
    else if (b->op == Op::lit) {
        if (op == Op::mul) {
            if (a->op == Op::div) {
                if (a->b->op == Op::lit) {
                    if (a->b->value == b->value) {
                        *this = *(a->a);
                        updateRange();
                    }
                }
            }
            if (b->value == 0) {
                setLiteral(0);
            }
            else if (b->value == 1) {
                *this = *a;
            }
        }
        else if (op == Op::div) {
            if (b->value == 1) {
                *this = *a;
            }
            else if (a->range.max < b->value) {
                *this = *AstNode::zero();
            }
            else if (a->op == Op::add) {
                if ((a->a->range.min > b->value) && (a->b->range.max < b->value)) { // Not quite correct - really need to know if a % 26 == 0 and b < 26
                    a = a->a;
                    updateRange();
                }
            }

            if (a->op == Op::mul && a->b->op == Op::lit) {
                if (a->b->value == b->value) {
                    *this = *a->a;
                }
            }
#if 0
            if (a->op == Op::add) {
                if (a->a->op == Op::mul && a->b->op == Op::mul) {
                    if (a->a->b->op == Op::lit && a->b->b->op == Op::lit) {
                        if (a->a->b->value == b->value && a->b->b->value == b->value) {
                            auto aa = a->a->a;
                            auto bb = a->b->a;
                            op = Op::add;
                            a = aa;
                            b = bb;
                            value = 0;
                            updateRange();
                        }
                    }
                }
            }
#endif
        }
        else if (op == Op::add) {
            if (a->op == Op::add && a->b->op == Op::lit) {
                auto v = b->value;
                auto* b2 = alloc(Op::lit, b->value + a->b->value);
                *this = *a;
                b = b2;
                updateRange();
            }
            else if (b->value == 0) {
                *this = *a;
            }
        }
    }
    updateRange();
}

bool IsLiteral(StringView const& s) {
    uint8_t ch = s.back();
    return isdigit(ch);
}

AstNode* Decompile(TextFileIn& file) {
    auto* zero = AstNode::zero();
    AstNode* areg[] = { zero, zero, zero, zero };
    int32_t inp_pos = 0;
    auto reg = [&](StringView const& v) -> AstNode*& {
        auto ch = v.front();
        assert(ch >= 'w' && ch <= 'z');
        return areg[v.front() - 'w'];
    };
    for (auto&& line : file.lines()) {
        StringView v(line);
        auto code = v.split(' ');
        auto a = v.split(' ');
        auto b = v.split(' ');
        auto mkop = [&](Op op) {
            auto* n = AstNode::alloc(op);
            n->a = reg(a);
            reg(a) = n;
            if (IsLiteral(b)) {
                n->b = AstNode::alloc(Op::lit, b.parseInt32());
            }
            else {
                n->b = reg(b);
            }
        };
        if (code == "inp") {
            auto* n = AstNode::alloc(Op::inp, inp_pos++);
            reg(a) = n;
        }
        else if (code == "add") {
            mkop(Op::add);
        }
        else if (code == "mul") {
            mkop(Op::mul);
        }
        else if (code == "div") {
            mkop(Op::div);
        }
        else if (code == "mod") {
            mkop(Op::mod);
        }
        else if (code == "eql") {
            mkop(Op::eql);
        }
        else {
            throw std::runtime_error("opcode");
        }
    }
    return reg("z"); // result is in z
}


struct Checker {
    using T = int64_t;
    ModelNumber num_;
    uint8_t ipos = 0;
    T w = {};
    T x = {};
    T y = {};
    T z = {};
    bool fail = false;

    void inp(T& r) {
        r = num_[ipos++];
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
            // throw logic_error("div0");
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

// Specific to the input.. TODO: read from the file.
int64_t D[] = { 1,  1,  1, 26,  1,  26,  1,  1,  1, 26, 26, 26, 26,  26 };
int64_t X[] = { 12, 10, 10, -6, 11, -12, 11, 12, 12, -2, -5, -4, -4, -12 };
int64_t A[] = { 6,  2, 13,  8, 13,   8,  3, 11, 10,  8, 14,  6,  8,   2 };

void iterate(ModelNumber const& m, uint8_t i, int64_t& z, vector<int>& st)
{
    auto w = m[i];
    bool b = (z % 26) + X[i];
    z /= D[i]; // pop?
    if (D[i] == 26) {
        st.pop_back();
    }
    if (b != w) {
        z *= 26;
        auto v = w + A[i];
        z += v;
        st.push_back(v);
    }
}

int64_t calc(ModelNumber m)
{
    vector<int> st;
    int64_t z = 0;
    for (auto i : integers(14)) {
        iterate(m, i, z, st);
    }
    return z;
}


ModelNumber MaxModelNumber()
{
    vector<uint8_t> stack;
    vector<pair<uint8_t, uint8_t>> pairs;
    for (auto i : integers(uint8_t(14))) {
        if (D[i] == 26) {
            pairs.push_back(pair(stack.back(), i));
            stack.pop_back();
        }
        else {
            stack.push_back(i);
        }
    }
    ModelNumber m;
    for (auto [a, b] : pairs) {
        // m[a] + A[a] + X[b] == m[b]
        auto x = A[a] + X[b];
        if (x < 0) {
            m.a[a] = 9;
            m.a[b] = 9 + x;
        }
        else {
            m.a[b] = 9;
            m.a[a] = 9 - x;
        }
    }
    return m;
}

ModelNumber MinModelNumber()
{
    vector<uint8_t> stack;
    vector<pair<uint8_t, uint8_t>> pairs;
    for (auto i : integers(uint8_t(14))) {
        if (D[i] == 26) {
            pairs.push_back(pair(stack.back(), i));
            stack.pop_back();
        }
        else {
            stack.push_back(i);
        }
    }
    ModelNumber m;
    for (auto [a, b] : pairs) {
        // m[a] + A[a] + X[b] == m[b]
        auto x = A[a] + X[b];
        if (x > 0) {
            m.a[a] = 1;
            m.a[b] = 1 + x;
        }
        else {
            m.a[b] = 1;
            m.a[a] = 1 - x;
        }
    }
    return m;
}


//-----------------------------------------------------------------------------
void solveFile(char const* fname) {
    TextFileIn f(fname);

    Checker checker;
#if 0
    auto* prog = Decompile(f);
    prog->optimize();

    Stringizer stringizer;
    print(stringizer.toString(prog));
    //auto chk1 = checker.run(13579246899999uLL);
    //auto chk1 = checker.run(11111111111111uLL);
    auto chk2 = checker.run(99999999999999uLL);
    auto chk3 =  prog->calc(99999999999999uLL);
    auto chk4 = prog->calc(62188832885682uLL);
    auto chk5 = calc(99999999999999uLL);
    //assert(chk1 == chk2);
#endif

    //auto sol = prog->solutions(0);
    uint64_t part1 = MaxModelNumber().value();
    auto chkp1 = checker.run(part1);
    //for (auto& s : sol) {
    //    amax(part1, s.value());
    //}
    print(part1);
    uint64_t part2 = MinModelNumber().value();
    auto chkp2 = checker.run(part2);
    print(part2);
}

//-----------------------------------------------------------------------------
void main() {
    //print("example");
    //solveFile("example.txt");
    print("input");
    solveFile("input.txt");
}

