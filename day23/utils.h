#pragma once

#include <algorithm>
#include <cassert>
#include <deque>
#include <functional>
#include <fstream>
#include <iostream>
#include <iterator>
#include <list>
#include <limits>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <tuple>
#include <unordered_set>
#include <unordered_map>
#include <vector>

#include "utils_stringview.h"

static double const PI = 3.14159265358979323846264338327950288;

template <typename T>
void print(T v) {
    std::cout << v << std::endl;
}


//-----------------------------------------------------------------------------
inline std::string FileReadAll(char const* name) {
    std::ifstream fs(name, std::ios::binary);
    std::ostringstream ss;
    ss << fs.rdbuf();
    return ss.str();
}

class TextFileIn;
class TextFileLines;
class TextFileLineIterator;

//-----------------------------------------------------------------------------
class TextFileLines
{
    TextFileIn* f_;
public:
    TextFileLines(TextFileIn* f)
        : f_(f)
    {}
    TextFileLineIterator begin() const;
    TextFileLineIterator end() const;
};

//-----------------------------------------------------------------------------
class TextFileIn {
    std::ifstream f_;
    std::string line_;
public:
    TextFileIn(char const* fname)
        : f_(fname)
    {
        if (!f_) {
            throw std::runtime_error(std::string("Could not open ") + fname);
        }
    }
    void rewind() {
        f_.seekg(0);
    }
    bool readLine(StringView& sv) {
        if (std::getline(f_, line_)) {
            sv = StringView(line_);
            return true;
        }
        sv = {};
        return false;
    }
    bool readLine(std::string& line) {
        if (std::getline(f_, line)) {
            return true;
        }
        line.clear();
        return false;
    }
    std::string readLine() {
        if (std::getline(f_, line_)) {
            return line_;
        }
        return {};
    }
    void skipLine() {
        std::getline(f_, line_);
    }

    TextFileLines lines() {
        return TextFileLines(this);
    }
};

//-----------------------------------------------------------------------------
class TextFileLineIterator
{
    TextFileIn* f_;
    StringView sv_;
public:
    TextFileLineIterator(TextFileIn* f)
        : f_(f)
    {
        next();
    }
    TextFileLineIterator()
        : f_(nullptr)
    {}
    bool operator==(TextFileLineIterator const& rhs) const {
        // Only really care about checking end condition today..
        return (f_ == nullptr) && (rhs.f_ == nullptr);
    }
    bool operator!=(TextFileLineIterator const& rhs) const {
        // Only really care about checking end condition today..
        return !(*this == rhs);
    }
    StringView operator*() const {
        return sv_;
    }
    TextFileLineIterator& operator++() {
        next();
        return *this;
    }
    void next() {
        if (!f_->readLine(sv_)) {
            f_ = nullptr;
        }
    }
};

inline TextFileLineIterator TextFileLines::begin() const {
    return TextFileLineIterator(f_);
}
inline TextFileLineIterator TextFileLines::end() const {
    return TextFileLineIterator();
}

//-----------------------------------------------------------------------------
template <typename F, typename II>
inline void tokenize(F func, II i, II e, char delimiter, bool process_empty_ranges = false)
{
    II t = i;

    while (i != e)
    {
        if (*i == delimiter)
        {
            if (process_empty_ranges || (i != t)) {
                func(t, i);
            }
            ++i;
            t = i;

            // last character was a delimiter
            if (process_empty_ranges && (t == e)) {
                func(t, e);
            }
        }
        else {
            ++i;
        }
    }

    if (t != e) {
        func(t, e);
    }
}

//-----------------------------------------------------------------------------
template <typename F, typename II>
inline void tokenize(F func, II i, II e, char const* delimiters, bool process_empty_ranges = false)
{
    II t = i;

    char const* e_delimiters = delimiters;
    while (*e_delimiters != 0) {
        ++e_delimiters;
    }

    while (i != e)
    {
        if (std::find(delimiters, e_delimiters, *i) != e_delimiters)
        {
            if (process_empty_ranges || (i != t)) {
                func(t, i);
            }
            ++i;
            t = i;

            // last character was a delimiter
            if (process_empty_ranges && (t == e)) {
                func(t, e);
            }
        }
        else {
            ++i;
        }
    }

    if (t != e) {
        func(t, e);
    }
}

//-----------------------------------------------------------------------------
inline std::deque<std::string>
split(std::string const& s, char delimiter) {
    std::deque<std::string> r;
    auto* beg = s.c_str();
    auto* end = beg + s.size();
    tokenize([&](char const* a, char const* b) {
        r.push_back(std::string(a, b));
        },
        beg, end, delimiter);
    return r;
}
inline std::deque<std::string>
split(std::string const& s, char const* delimiters) {
    std::deque<std::string> r;
    auto* beg = s.c_str();
    auto* end = beg + s.size();
    tokenize([&](char const* a, char const* b) {
        r.push_back(std::string(a, b));
        },
        beg, end, delimiters);
    return r;
}

template<typename T, typename A>
inline std::ostream& operator<<(std::ostream& o, std::vector<T, typename A> const& s) {
    o << "[";
    bool f = true;
    for (auto&& v : s) {
        if (f) { f = false; }
        else { o << ","; }
        o << v;
    }
    o << "]";
    return o;
}
template<typename T, typename A>
inline std::ostream& operator<<(std::ostream& o, std::deque<T, typename A> const& s) {
    o << "[";
    bool f = true;
    for (auto&& v : s) {
        if (f) { f = false; }
        else { o << ","; }
        o << v;
    }
    o << "]";
    return o;
}

template<typename T, typename C, typename A>
inline std::ostream& operator<<(std::ostream& o, std::set<T, C, typename A> const& s) {
    o << "{ ";
    bool f = true;
    for (auto&& v : s) {
        if (f) { f = false; }
        else { o << ", "; }
        o << v;
    }
    o << " }";
    return o;
}

template<typename K, typename T, typename C, typename A>
inline std::ostream& operator<<(std::ostream& o, std::map<K, T, C, typename A> const& s) {
    o << "{ ";
    bool f = true;
    for (auto&& v : s) {
        if (f) { f = false; }
        else { o << ", "; }
        o << v.first << ": " << v.second;
    }
    o << " }";
    return o;
}

inline std::ostream& args_out(std::ostream& os) {
    return os;
}
template <typename Arg, typename...Args>
inline std::ostream& args_out(std::ostream& os, Arg&& arg, Args&&...args) {
    os << std::forward<Arg>(arg);
    args_out(os, std::forward<Args>(args)...);
    return os;
}

template <typename...Args>
inline void write(Args&&...args) {
    args_out(std::cout, std::forward<Args>(args)...);
}
template <typename...Args>
inline void writeln(Args&&...args) {
    write(std::forward<Args>(args)...);
    std::cout << std::endl;
}

template <typename...Args>
inline std::string args_to_string(Args&&...args) {
    std::ostringstream ss;
    args_out(ss, std::forward<Args>(args)...);
    return ss.str();
}

template <typename...Args>
inline void raise(Args&&...args) {
    throw std::runtime_error(args_to_string(std::forward<Args>(args)...));
}
template <typename T>
struct num_range {
    using self = num_range<T>;
    struct iterator {
        T v;
        iterator& operator++() {
            ++v;
            return *this;
        }
        T const& operator*() const {
            return v;
        }
        bool operator==(iterator const& r) const {
            return v == r.v;
        }
        bool operator!=(iterator const& r) const {
            return !(*this == r);
        }
    };
    T start_ = 0;
    T limit_ = 0;
    iterator begin() const {
        return iterator{ start_ };
    }
    iterator end() const {
        return iterator{ limit_ };
    }
    bool empty() const {
        return (start_ == limit_);
    }
    bool intersects(self const& other) const {
        return ((other.start_ < limit_) && (other.limit_ > start_));
    }
    bool contains(self const& other) const {
        return ((other.start_ >= start_) && (other.limit_ <= limit_));
    }
    T size() const {
        return (limit_ - start_);
    }
};
template <typename T>
inline num_range<T> integers(T limit) {
    return num_range<T>{0, limit};
}
template <typename T>
inline num_range<T> integers(T start, T limit) {
    return num_range<T>{start, limit};
}

template <typename S>
inline S intersect(S const& a, S const& b) {
    S r;
    std::set_intersection(a.begin(), a.end(), b.begin(), b.end(), std::inserter(r, r.begin()));
    return r;
}

template<typename K, typename T, typename C, typename A>
inline std::set<K> keys(std::map<K, T, C, typename A> const& m) {
    std::set<K> s;
    for (auto&& v : m) {
        s.insert(v.first);
    }
    return s;
}
template <typename T, size_t N>
inline bool contains(T const(&c)[N], T const& val) {
    for (auto&& v : c) {
        if (v == val) {
            return true;
        }
    }
    return false;
}
template <typename T>
inline bool contains(std::vector<T> const& c, T const& val) {
    return c.end() != std::find(c.begin(), c.end(), val);
}
template <typename T>
inline bool contains(std::list<T> const& c, T const& val) {
    return c.end() != std::find(c.begin(), c.end(), val);
}
template <typename T>
inline bool contains(std::set<T> const& c, T const& val) {
    return c.end() != c.find(val);
}
template <typename T, typename H>
inline bool contains(std::unordered_set<T, H> const& c, T const& val) {
    return c.end() != c.find(val);
}
template <typename K, typename T>
inline bool contains(std::map<K, T> const& c, K const& key) {
    return c.end() != c.find(key);
}
template <typename K, typename T>
inline bool contains(std::unordered_map<K, T> const& c, K const& key) {
    return c.end() != c.find(key);
}

template <typename K, typename T>
inline T const* map_value(std::map<K, T> const& c, K const& key) {
    auto i = c.find(key);
    if (c.end() != i) {
        return &i->second;
    }
    return nullptr;
}
template <typename K, typename T>
inline T* map_value(std::map<K, T>& c, K const& key) {
    auto i = c.find(key);
    if (c.end() != i) {
        return &i->second;
    }
    return nullptr;
}
template <typename K, typename T, typename U>
inline T const map_value_or(std::map<K, T> const& c, K const& key, U const& value) {
    auto i = c.find(key);
    if (c.end() != i) {
        return i->second;
    }
    return value;
}

template <typename T>
inline T element(size_t index, std::initializer_list<T> const& elems) {
    if (index < elems.size()) {
        return elems.begin()[index];
    }
    throw std::runtime_error("element: out of bounds, " + index);
}

template <typename T>
inline T element_or(size_t index, T&& def, std::initializer_list<T> const& elems) {
    if (index < elems.size()) {
        return elems.begin()[index];
    }
    return def;
}


template <typename A, typename B>
inline bool equal(A& a, B& b) {
    if (a.size() != b.size()) {
        return false;
    }
    return std::equal(a.begin(), a.end(), b.begin(), b.end());
}

template <typename T, typename U>
T& amin(T& lhs, U const& rhs) {
    if (rhs < lhs) lhs = rhs;
    return lhs;
}
template <typename T, typename U>
T& amax(T& lhs, U const& rhs) {
    if (rhs > lhs) lhs = rhs;
    return lhs;
}

template <typename T>
inline T
gcd(T x, T y)
{
    T g;
    if (x <= y) {
        T r = y % x;
        if (r == 0) {
            g = x;
        }
        else {
            g = gcd(x, r);
        }
    }
    else {
        g = gcd(y, x);
    }
    return g;
}
template <typename T>
inline T sgn(T x) {
    return (x < 0) ? -1 : (x == 0) ? 0 : 1;
}

template <typename C>
inline void sort(C&& c) {
    std::sort(std::begin(c), std::end(c));
}
template <typename C, typename F>
inline void sort(C&& c, F&& f) {
    std::sort(std::begin(c), std::end(c), f);
}

struct first_less {
    template <typename T>
    bool operator()(T const& a, T const& b) const {
        return (a.first < b.first);
    }
};

struct first_greater {
    template <typename T>
    bool operator()(T const& a, T const& b) const {
        return (a.first > b.first);
    }
};

struct second_less {
    template <typename T>
    bool operator()(T const& a, T const& b) const {
        return (a.second < b.second);
    }
};

struct second_greater {
    template <typename T>
    bool operator()(T const& a, T const& b) const {
        return (a.second > b.second);
    }
};

template <typename C, typename T, typename F>
void insert_sorted(C&& c, T&& v, F&& f) {
    c.insert(std::lower_bound(std::begin(c), std::end(c), v, f), std::forward<T>(v));
}

template <typename C, typename T>
void insert_sorted(C&& c, T&& v) {
    insert_sorted(c, std::forward<T>(v), std::less<T>());
}

struct PointF {
    double x;
    double y;

    bool operator== (PointF const& p) const {
        return (x == p.x) && (y == p.y);
    }
    bool operator!= (PointF const& p) const {
        return !(*this == p);
    }
    auto fields() const {
        return std::tie(x, y);
    }
    bool operator< (PointF const& p) const {
        return (fields() < p.fields());
    }
    PointF operator-(PointF const& rhs) const {
        return { x - rhs.x, y - rhs.y };
    }
    PointF operator+(PointF const& rhs) const {
        return { x + rhs.x, y + rhs.y };
    }
    PointF cw90() const {
        return PointF{ -y, x };
    }

    double sqlen() const {
        return (x * x + y * y);
    }
    double len() const {
        return sqrt(sqlen());
    }
};

struct Point {
    int x = 0;
    int y = 0;

    bool operator== (Point const& p) const {
        return (x == p.x) && (y == p.y);
    }
    bool operator!= (Point const& p) const {
        return !(*this == p);
    }
    auto fields() const {
        return std::tie(x, y);
    }
    bool operator< (Point const& p) const {
        return (fields() < p.fields());
    }
    Point operator-(Point const& rhs) const {
        return { x - rhs.x, y - rhs.y };
    }
    Point operator+(Point const& rhs) const {
        return { x + rhs.x, y + rhs.y };
    }
    Point& operator+=(Point const& rhs) {
        return (*this = *this + rhs);        
    }
    Point& operator-=(Point const& rhs) {
        return (*this = *this - rhs);
    }
    Point minimized() const {
        if (x == 0) {
            if (y == 0) {
                return *this;
            }
            return { 0, sgn(y) };
        }
        if (y == 0) {
            return { sgn(x), 0 };
        }
        auto p = *this;
        auto g = gcd(abs(x), abs(y));
        if (g != 0) {
            p.x /= g;
            p.y /= g;
        }
        return p;
    }
    Point cw90() const {
        return Point{ -y, x };
    }
    Point ccw90() const {
        return Point{ y, -x };
    }
    void swapxy() {
        std::swap(x, y);
    }

    double angle() const {
        auto d = minimized(); // for consistency
        d = d.cw90();
        auto a = atan2(d.y, d.x);
        if (a < 0) {
            return a + (3.14159265359 * 2);
        }
        return a;
    }
    int manhattan() const {
        return abs(x) + abs(y);
    }
};
using points = std::vector<Point>;

struct Bounds {
    num_range<int> x_;
    num_range<int> y_;

    Bounds() {}
    template <typename Collection>
    Bounds(Collection const& c) {
        for (Point const& p : c) {
            add(p);
        }
    }

    void add(Point const& p) {
        if (x_.empty()) {
            x_.start_ = p.x;
            x_.limit_ = p.x + 1;
            y_.start_ = p.y;
            y_.limit_ = p.y + 1;
            return;
        }
        amin(x_.start_, p.x);
        amin(y_.start_, p.y);
        amax(x_.limit_, p.x + 1);
        amax(y_.limit_, p.y + 1);
    }
    num_range<int> cols() const {
        return x_;
    }
    num_range<int> rows() const {
        return y_;
    }

    template <typename F>
    void print_f(F&& f) {
        for (auto y : y_) {
            for (auto x : x_) {
                write(f({ x,y }));
            }
            writeln();
        }
    }
};

template <typename T>
struct SparseBoard
{
    std::map<Point, T> m;
    Bounds bounds;
    T get_def(Point p, T def) {
        return map_value_or(m, p, def);
    }
    template <typename U>
    void put(Point p, U&& v) {
        m[p] = static_cast<T>(v);
        bounds.add(p);
    }
    template <typename F>
    void print_f(F && f) {
        bounds.print_f(std::forward<F>(f));
    }
    template <typename U>
    uint32_t count(U const& v) const {
        uint32_t n = 0;
        for (auto&& p : m) {
            if (p.second == v) {
                ++n;
            }
        }
        return n;
    }
};

struct Vec3 {
    union {
        struct {
            int32_t x;
            int32_t y;
            int32_t z;
        };
        int32_t v[3];
    };
    size_t hash() const {
        // Coords don't get that large - just join them into 64bit int abd use standard hash.
        auto const h = std::hash<uint64_t>();
        return h(x) ^ h(y) ^ h(z);
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
    int32_t& operator[](size_t i) {
        return v[i];
    }
    int32_t const operator[](size_t i) const {
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
namespace std {
    template <>
    struct hash<Vec3> {
        std::size_t operator()(const Vec3& k) const {
            return k.hash();
        }
    };
}
