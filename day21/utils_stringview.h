#pragma once

#include <string>
#include <stdexcept>
#include <iosfwd>

class StringView {
protected:
    char const* a_ = nullptr;
    char const* b_ = nullptr;
public:
    static int const npos = -1;
    using self = StringView;

    StringView() {}
    StringView(char const* a, char const* b)
        : a_(a), b_(b)
    {
        if (!a || (b < a)) {
            throw std::domain_error("StringView: invalid range");
        }
    }
    StringView(char const* s)
        : a_(s)
        , b_(s + strlen(s))
    {
    }
    template<int size>
    StringView(char(&s)[size])
        : a_(s)
        , b_(s + (size - 1))
    {}
    explicit StringView(std::string const& s)
        : a_(s.data())
        , b_(s.data() + s.size())
    {
    }
    char const* begin() const {
        return a_;
    }
    char const* end() const {
        return b_;
    }
    size_t size() const {
        return (b_ - a_);
    }
    char operator[](size_t index) const {
        return a_[index];
    }
    bool empty() const {
        return (a_ == b_);
    }
    void clear() {
        a_ = b_ = nullptr;
    }
    char back() const {
        if (empty()) {
            return 0;
        }
        return b_[-1];
    }
    char front() const {
        if (empty()) {
            return 0;
        }
        return a_[0];
    }
    int compare(char const* s) const {
        return compare(self(s));
        //return strncmp(a_, s, size());
    }
    int compare(self const& other) const {
        auto n = (size() < other.size()) ? size() : other.size();
        int result = strncmp(a_, other.a_, n);
        if ((0 == result) && (size() != other.size())) {
            result = (size() < other.size()) ? -1 : 1;
        }
        return result;
    }
    bool operator==(std::string const& s) const {
        return equals(self(s));
    }
    friend bool operator==(std::string const& s, self const& rhs) {
        return rhs.equals(s);
    }
    bool operator==(char const* s) const {
        return equals(s);
    }
    friend bool operator==(char const* s, self const& rhs) {
        return rhs.equals(s);
    }
    bool operator!=(std::string const& s) const {
        return !equals(self(s));
    }
    friend bool operator!=(std::string const& s, self const& rhs) {
        return !rhs.equals(s);
    }
    bool operator!=(char const* s) const {
        return !equals(s);
    }
    friend bool operator!=(char const* s, self const& rhs) {
        return !rhs.equals(s);
    }
    bool operator<(self const& rhs) const {
        return (compare(rhs) < 0);
    }
    bool equals(char const* s) const {
        return (0 == compare(s));
    }
    bool equals(std::string const& s) const {
        return (0 == compare(self(s)));
    }
    bool equals(self const& s) const {
        if (size() == s.size()) {
            return (0 == compare(s));
        }
        return false;
    }
    bool isValid() const {
        return (a_ != nullptr);
    }
    bool isNull() const {
        return !a_;
    }
    bool contains(char const* loc) const {
        return (loc >= a_) && (loc < b_);
    }
    std::string toString() const {
        return std::string(a_, b_);
    }
    std::string& toString(std::string& s) const {
        s.assign(a_, b_);
        return s;
    }
    struct Convbuf {
        static size_t const COUNT = 64;
        char data[COUNT];
        Convbuf(self const& v) {
            size_t n = COUNT - 1;
            if (n > v.size()) {
                n = v.size();
            }
            memcpy(data, v.begin(), n * sizeof(char));
            data[n] = 0;
        }
        int detectRadix() const {
            if (data[0] == '0' && data[1] == 'x') {
                return 16;
            }
            return 10;
        }
    };
    bool maybeDouble() const {
        return (find_first_of(".eE") != npos);
    }

    struct shims {
        // Use overloading for type specializations - we need to 
        // select the appropriate standard library function based 
        // upon the concrete character type.
        static uint64_t toU64(char const* s, char** eptr, int radix) {
            return strtoull(s, eptr, radix);
        }
        static int64_t toI64(char const* s, char** eptr, int radix) {
            return strtoll(s, eptr, radix);
        }
        static double toF64(char const* s, char** eptr) {
            return strtod(s, eptr);
        }
    };
    uint64_t parseUInt64(int radix = 0, bool* ok = nullptr) const {
        Convbuf buffer(*this);
        // Do not have octal or binary in A2L
        if (radix == 0) {
            radix = buffer.detectRadix();
        }
        char* eptr = buffer.data;
        auto result = shims::toU64(buffer.data, &eptr, radix);
        if (ok) {
            *ok = eptr != buffer.data;
        }
        return result;
    }
    int64_t parseInt64(int radix = 0, bool* ok = nullptr) const {
        Convbuf buffer(*this);
        if (radix == 0) {
            radix = buffer.detectRadix();
        }
        char* eptr = buffer.data;
        auto result = shims::toI64(buffer.data, &eptr, radix);
        if (ok) {
            *ok = eptr != buffer.data;
        }
        return result;
    }
    int32_t parseInt32(int radix = 0, bool* ok = nullptr) const {
        return static_cast<int32_t>(parseInt64(radix, ok));
    }
    int32_t i32(int radix = 0, bool* ok = nullptr) const {
        return parseInt32(radix, ok);
    }
    double parseDouble(bool* ok = nullptr) const {
        Convbuf buffer(*this);
        char* eptr = buffer.data;
        auto result = shims::toF64(buffer.data, &eptr);
        if (ok) {
            *ok = eptr != buffer.data;
        }
        return result;
    }
    bool parse(uint64_t& v) const {
        bool ok = false;
        auto result = parseUInt64(0, &ok);
        if (ok) {
            v = result;
        }
        return ok;
    }
    bool parse(int64_t& v) const {
        bool ok = false;
        auto result = parseInt64(0, &ok);
        if (ok) {
            v = result;
        }
        return ok;
    }
    bool parse(double& v) const {
        bool ok = false;
        auto result = parseDouble(&ok);
        if (ok) {
            v = result;
        }
        return ok;
    }
    template <typename T64, typename V>
    bool parseT(V& v) const {
        T64 parsed;
        if (parse(parsed)) {
            v = static_cast<V>(parsed);
            // TODO: range checking?
            return true;
        }
        return false;
    }
    bool parse(uint8_t& v) const {
        return parseT<uint64_t>(v);
    }
    bool parse(uint16_t& v) const {
        return parseT<uint64_t>(v);
    }
    bool parse(uint32_t& v) const {
        return parseT<uint64_t>(v);
    }
    bool parse(int8_t& v) const {
        return parseT<int64_t>(v);
    }
    bool parse(int16_t& v) const {
        return parseT<int64_t>(v);
    }
    bool parse(int32_t& v) const {
        return parseT<int64_t>(v);
    }
    bool parse(float& v) const {
        return parseT<double>(v);
    }

    friend std::ostream& operator<<(std::ostream& s, self const& v) {
        s.write(v.a_, v.size());
        return s;
    }

    int bsearch(self const* beg, self const* end) const {
        auto* i = std::lower_bound(beg, end, *this);
        if (i == end) {
            return -1;
        }
        if (equals(*i)) {
            return static_cast<int>(i - beg);
        }
        return -1;
    }

    int find(char ch, size_t offset = 0) const {
        for (auto* i = a_ + offset; i < b_; ++i) {
            if (*i == ch) {
                return static_cast<int>(i - a_);
            }
        }
        return -1;
    }
    int find_first_of(char const* chars, size_t offset = 0) const {
        for (auto* i = a_ + offset; i < b_; ++i) {
            for (auto* in = chars; *in; ++in) {
                if (*i == *in) {
                    return static_cast<int>(i - a_);
                }
            }
        }
        return -1;
    }
    int find_first_not_of(char const* chars, size_t offset = 0) const
    {
        for (auto* i = a_ + offset; i < b_; ++i) {
            bool found = false;
            for (auto* in = chars; *in; ++in) {
                if (*i == *in) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                return static_cast<int>(i - a_);
            }
        }
        return -1;
    }

    bool endsWith(char ch) const {
        if (empty()) {
            return false;
        }
        return (b_[-1] == ch);
    }
    bool startsWith(char ch) const {
        if (empty()) {
            return false;
        }
        return (a_[0] == ch);
    }
    bool startsWith(char const* s) const {
        return startsWith(self(s));
    }
    bool startsWith(self s) const {
        if (size() < s.size()) {
            return false;
        }
        auto* x = a_;
        for (auto ch : s) {
            if (*x != ch) {
                return false;
            }
            ++x;
        }
        return true;
    }
    bool startsWithCI(char const* s) const {
        return startsWithCI(self(s));
    }
    bool startsWithCI(self s) const {
        if (size() < s.size()) {
            return false;
        }
        auto* x = a_;
        for (auto ch : s) {
            if (lower_i(*x) != lower_i(ch)) {
                return false;
            }
            ++x;
        }
        return true;
    }
    bool endsWith(char const* s) const {
        return endsWith(self(s));
    }
    bool endsWith(self s) const {
        if (size() < s.size()) {
            return false;
        }
        auto* x = b_ - s.size();
        for (auto ch : s) {
            if (*x != ch) {
                return false;
            }
            ++x;
        }
        return true;
    }
    bool endsWithCI(char const* s) const {
        return endsWith(self(s));
    }
    bool endsWithCU(self s) const {
        if (size() < s.size()) {
            return false;
        }
        auto* x = b_ - s.size();
        for (auto ch : s) {
            if (lower_i(*x) != lower_i(ch)) {
                return false;
            }
            ++x;
        }
        return true;
    }

    static unsigned lower_i(char ch) {
        return ::tolower((unsigned)ch);
    }
    self mid(size_t offset, int n = -1) const
    {
        if (n < 0) {
            return self(a_ + offset, b_);
        }
        return self(a_ + offset, a_ + n);
    }
    self left(size_t n) const {
        return self(a_, a_ + n);
    }
    self right(size_t n) const {
        return self(b_ - n, b_);
    }
    self leftTrimmed() const {
        auto* a = a_;
        for (; a < b_; ++a) {
            if (!std::isspace((unsigned)*a)) {
                break;
            }
        }
        return self(a, b_);
    }
    self rightTrimmed() const {
        auto* b = b_;
        for (; b > a_; --b) {
            if (!std::isspace((unsigned)*(b - 1))) {
                break;
            }
        }
        return self(a_, b);
    }
    self rightTrimmed(char ch) const
    {
        auto* b = b_;
        for (; b > a_; --b) {
            if ((*(b - 1)) != ch) {
                break;
            }
        }
        return self(a_, b);
    }
    self trimmed() const {
        return leftTrimmed().rightTrimmed();
    }
    void trim() {
        *this = trimmed();
    }
    char pop_back() {
        if (b_ > a_) {
            auto ch = back();
            --b_;
            return ch;
        }
        return 0;
    }
    char pop_front() {
        if (b_ > a_) {
            auto ch = front();
            ++a_;
            return ch;
        }
        return 0;
    }
    //! @brief Split at separator char at pos.
    //! @return A string view of the chars up until 'pos'.  
    //!         'this' will be set to the range following 
    //!         (but not including) 'pos'.
    //!
    //! If pos == npos then the returned 
    //! string will be a copy of 'this' and 'this' will 
    //! become empty.
    self splitAtSeparatorPos(int pos)
    {
        self r = *this;
        if (pos != npos) {
            r = left(pos);
            a_ = r.b_ + 1;
        }
        else {
            a_ = b_;
        }
        return r;
    }
    //! @brief Split at the given separator character.
    //! @param separator The charater to search for.
    //! @return string view up until the separator.  
    //!         'this' will be set to the range following 
    //!         the separator.
    //! 
    //! If the separator is not found then the returned 
    //! string will be a copy of 'this' and 'this' will 
    //! become empty.
    self split(char separator) {
        auto pos = find(separator);
        return splitAtSeparatorPos(pos);
    }
    //! @brief Split at the first one of the given separator characters.
    //! @param separator The charater to search for.
    //! @return string view up until the separator.
    //!         'this' will be set to the range following
    //!         the separator.
    //!
    //! If the separator is not found then the returned
    //! string will be a copy of 'this' and 'this' will
    //! become empty.
    self splitAtAnyOf(char* separators)
    {
        auto pos = find_first_of(separators);
        return splitAtSeparatorPos(pos);
    }

    self take(uint32_t n) {
        if (size() > n) {
            auto r = left(n);
            *this = mid(n);
            return r;
        }
        auto r = *this;
        *this = mid(size());
        return r;
    }
    static uint8_t nibbleFromHex(char ch) {
        if (ch >= '0' && ch <= '9') {
            return (ch - '0');
        }
        if (ch >= 'A' && ch <= 'F') {
            return 10 + (ch - 'A');
        }
        if (ch >= 'a' && ch <= 'f') {
            return 10 + (ch - 'a');
        }
        return 0u;
    }
    uint8_t parseByteHex() const {
        if (size() > 1) {
            return (nibbleFromHex(a_[0]) << 4) + nibbleFromHex(a_[1]);
        }
        else if (a_ < b_) {
            return nibbleFromHex(a_[0]);
        }
        return 0;
    }
    uint8_t parseNibbleHex() const {
        if (a_ < b_) {
            return nibbleFromHex(a_[0]);
        }
        return 0;
    }
};


inline bool operator==(StringView const& lhs, std::string const& rhs) {
    return (0 == lhs.compare(rhs.c_str()));
}

inline bool operator<(StringView const& lhs, std::string const& rhs) {
    return (lhs.compare(rhs.c_str()) < 0);
}

inline bool operator>(StringView const& lhs, std::string const& rhs) {
    return (lhs.compare(rhs.c_str()) > 0);
}
inline bool operator<(std::string const& lhs, StringView const& rhs) {
    return (rhs.compare(lhs.c_str()) > 0);
}
inline bool operator>(std::string const& lhs, StringView const& rhs) {
    return (rhs.compare(lhs.c_str()) < 0);
}
