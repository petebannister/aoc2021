#include "utils.h"

using namespace std;


struct Node
{
    uint32_t v = 0;
    Node* a = nullptr;
    Node* b = nullptr;
    Node* parent = nullptr;

    uint64_t magnitude() const {
        if (a) {
            return (a->magnitude() * 3) + (b->magnitude() * 2);
        }
        return v;
    }

    bool leaf() const {
        return a == nullptr;
    }
    uint32_t* rightLeaf() {
        if (leaf()) {
            return &v;
        }
        return b->rightLeaf();
    }
    uint32_t* leftLeaf() {
        if (leaf()) {
            return &v;
        }
        return a->leftLeaf();
    }
    uint32_t* left() {
        if (!parent) {
            return nullptr;
        }
        if (parent->b == this) {
            return parent->a->rightLeaf();
        }
        return parent->left();
    }
    uint32_t* right() {
        if (!parent) {
            return nullptr;
        }
        if (parent->a == this) {
            return parent->b->leftLeaf();
        }
        return parent->right();
    }
    // To explode a pair, the pair's left value is added to
    // the first regular number to the left of the exploding
    // pair (if any), and the pair's right value is added to 
    // the first regular number to the right of the exploding 
    // pair (if any). Exploding pairs will always consist of 
    // two regular numbers. Then, the entire exploding pair is
    // replaced with the regular number 0.
    bool explode_i(uint32_t d) {
        if (leaf()) {
            return false;
        }
        if (d >= 4) {
            assert(a->leaf());
            assert(b->leaf());
            if (auto* l = left()) {
                *l += a->v;
            }
            if (auto* r = right()) {
                *r += b->v;
            }
            v = 0;
            a = nullptr;
            b = nullptr;
            return true;
        }
        if (a && a->explode_i(d + 1)) {
            return true;
        }
        if (b && b->explode_i(d + 1)) {
            return true;
        }
        return false;
    }
    bool explode() {
        return explode_i(0);
    }
    void split_i() {
        a = new Node();
        b = new Node();
        a->v = v / 2;
        b->v = (v + 1) / 2;
        v = 0;
        a->parent = this;
        b->parent = this;
    }
    bool split() {
        if (leaf()) {
            if (v >= 10) {
                split_i();
                return true;
            }
            return false;
        }
        if (a->split()) {
            return true;
        }
        return b->split();
    }
    bool reduceStep() {
        if (explode()) {
            return true;
        }
        return split();
    }

    void reduce() {
        while (reduceStep()) {
            //print(toString());
            ;
        }
    }
    std::string toString() const {
        if (leaf()) {
            return std::to_string(v);
        }
        return "[" + a->toString() + "," + b->toString() + "]";
    }
};

Node* parse(StringView s) {
    std::vector<Node*> stack;
    Node* root = nullptr;
    if (s.startsWith('[')) {
        s.pop_front();
        root = new Node;
        stack.push_back(root);
    }
    else {
        return nullptr;
    }
    while (!s.empty()) {
        if (s.startsWith('[')) {
            s.pop_front();
            auto* n = new Node;
            auto* p = stack.back();
            n->parent = p;
            if (p->a) {
                p->b = n;
            }
            else {
                p->a = n;
            }
            stack.push_back(n);
        }
        else if (s.startsWith(']')) {
            s.pop_front();
            stack.pop_back();
        }
        else if (s.startsWith(',')) {
            s.pop_front();
            // skip.
        }
        else {
            auto e = s.find_first_of(",]");
            if (e == s.npos) {
                throw std::runtime_error("parse");
            }
            auto* n = new Node;
            n->v = s.left(e).parseInt32(10);
            s = s.mid(e);
            auto* p = stack.back();
            n->parent = p;
            if (p->a) {
                p->b = n;
            }
            else {
                p->a = n;
            }
        }
    }
    return root;
}

uint32_t additionSum(StringView a, StringView b)
{
    auto* na = parse(a);
    auto* nb = parse(b);

    auto* addition = new Node;
    addition->a = na;
    addition->b = nb;
    na->parent = addition;
    nb->parent = addition;
    addition->reduce();
    return addition->magnitude();
}

void solveFile(char const* fname) {
    TextFileIn f(fname);
    Node* addition = nullptr;
    vector<string> lines;
    for (auto line : f.lines()) {
        lines.push_back(line.toString());
        if (!addition) {
            addition = parse(line);
        }
        else {
            auto* a = addition;
            auto* b = parse(line);
            addition = new Node;
            addition->a = a;
            addition->b = b;
            a->parent = addition;
            b->parent = addition;
            addition->reduce();
            //print("");
        }
    }
    addition->reduce();
    auto s = addition->toString();
    print(addition->magnitude());

    uint32_t highest = 0;
    for (auto i : integers(lines.size())) {
        for (auto k : integers(lines.size())) {
            if (k != i) {
                amax(highest, additionSum(lines[i].c_str(), lines[k].c_str()));
                amax(highest, additionSum(lines[k].c_str(), lines[i].c_str()));
            }
        }
    }
    print(highest);

}

void main() {
    print("example");
    solveFile("example.txt");
    print("input");
    solveFile("input.txt");
}
