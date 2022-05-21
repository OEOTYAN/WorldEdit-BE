//
// Created by OEOTYAN on 2021/02/19.
//
#pragma once
#ifndef WORLDEDIT_STRINGTOOL_H
#define WORLDEDIT_STRINGTOOL_H

#include <string>
#include <algorithm>
#include <iomanip>

namespace worldedit {

    namespace {
        template <typename T>
        std::string fto_string(const T a_value, int i = 23, bool j = false) {
            std::ostringstream out;
            out.precision(i);
            if (i < 23 && i > -1) {
                if (j) {
                    out << std::fixed << std::setw(i + 3) << std::setfill(' ')
                        << a_value;
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

        std::string toLowerString(std::string str) {
            transform(str.begin(), str.end(), str.begin(),
                      (int (*)(int))tolower);
            return str;
        }

        void stringReplace(std::string& str,
                           const std::string& fstr,
                           const std::string& rep) {
            std::string::size_type pos = 0;
            std::string::size_type a = fstr.length();
            std::string::size_type b = rep.length();
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
    }  // namespace
}  // namespace worldedit
#endif  // WORLDEDIT_STRINGTOOL_H