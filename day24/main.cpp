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
    lit, inp, add, mul, div, mod, eql
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
        default:
            throw logic_error("tostring case");
        }
    }

    int64_t calc() const {
        switch (op) {
        case Op::lit: return value;
        case Op::inp: throw std::runtime_error("need input");
        case Op::add: return a->calc() + b->calc();
        case Op::mul: return a->calc() * b->calc();
        case Op::div: return a->calc() / b->calc();
        case Op::mod: return a->calc() % b->calc();
        case Op::eql: return (a->calc() == b->calc()) ? 1 : 0;
        default:
            throw logic_error("calc case");
        }
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
            range = a->range;
            range.min *= b->range.min;
            range.max *= b->range.max;
            break;
        case Op::div:
            range = a->range;
            // what about possible zero div?
            if (b->range.min == 0) {
                range.min /= b->range.max;
                //range.max;
            }
            else {
                range.min /= b->range.max;
                range.max /= b->range.min;
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
            range = { 0, 1 };
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
    if (op == Op::eql) {
        if (!a->range.intersects(b->range)) {
            // never equal
            setLiteral(0);
            return;
        }
    }
    if (op == Op::mod) {
        // are there any terms that are multiplied by the modulus that could be optimized out?
        if (auto* da = a->refactorMod(b->value)) {
            a = da;
            a->updateRange();
        }
        if (a->range.max < b->value) {
            *this = *a;
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
        }
        else if (op == Op::add) {
            if (a->op == Op::add && a->b->op == Op::lit) {
                a->b->value += b->value;
                *this = *a;
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


//-----------------------------------------------------------------------------
void solveFile(char const* fname) {
    TextFileIn f(fname);

    auto* prog = Decompile(f);
    prog->optimize();

    Stringizer stringizer;
    print(stringizer.toString(prog));

    uint32_t part1 = 0;
    print(part1);
    uint32_t part2 = 0;
    print(part2);
}

//-----------------------------------------------------------------------------
void main() {
    print("example");
    solveFile("example.txt");
    print("input");
    solveFile("input.txt");
}

