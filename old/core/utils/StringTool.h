#pragma once
#include <algorithm>
#include <string>
namespace we {

namespace {
template <typename T>
std::string fto_string(const T a_value, int i = 23, bool j = false) {
    std::ostringstream out;
    out.precision(i);
    if (i < 23 && i > -1) {
        if (j) {
            out << std::fixed << std::setw(i + 3) << std::setfill(' ') << a_value;
        } else {
            out << std::fixed << a_value;
        }
    } else if (i < 0) {
        out << std::setw(-i) << std::setfill(' ') << a_value;
    } else {
        out << a_value;
    }
    return out.str();
}
} // namespace

inline std::string toLowerString(std::string str) {
    transform(str.begin(), str.end(), str.begin(), (int (*)(int))tolower);
    return str;
}

inline void stringReplace(std::string& str, std::string_view fstr, std::string_view rep) {
    std::string::size_type pos = 0;
    std::string::size_type a   = fstr.length();
    std::string::size_type b   = rep.length();
    if (b == 0)
        while ((pos = str.find(fstr, pos)) != std::string::npos) {
            str.erase(pos, a);
        }
    else
        while ((pos = str.find(fstr, pos)) != std::string::npos) {
            str.replace(pos, a, rep);
            pos += b;
        }
}

inline bool frontIs(std::string_view s, std::string_view front) {
    if (s.substr(0, front.size()) == front) {
        return true;
    }
    return false;
}

inline bool isHex(char s) {
    if ('A' <= s && s <= 'F') {
        return true;
    }
    if ('a' <= s && s <= 'f') {
        return true;
    }
    if ('0' <= s && s <= '9') {
        return true;
    }
    return false;
}

inline bool isColorHex(std::string_view x) {
    if (x[0] == '#' && (isHex(x[1]) && isHex(x[2]) && isHex(x[3]))
        && (x.length() == 4
            || (x.length() == 7 && (isHex(x[4]) && isHex(x[5]) && isHex(x[6]))))) {
        return true;
    }
    return false;
}
} // namespace we