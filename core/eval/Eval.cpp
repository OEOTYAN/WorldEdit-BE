#pragma once
//
// Created by OEOTYAN on 2022/5/15.
//
#include <MC/Player.hpp>
#include "Eval.h"
#include "string/StringTool.h"

namespace worldedit {
    double eval(Player* player, const std::string& str) {
        auto pos = player->getPosition();
        std::map<std::string, double> variables;
        variables["pi"] = 3.14159265358979323846;
        variables["e"] = 2.7182818284590452354;
        variables["x"] = pos.x;
        variables["y"] = pos.y;
        variables["z"] = pos.z;
        variables["cx"] = (static_cast<int>(floor(pos.x))) >> 4;
        variables["cz"] = (static_cast<int>(floor(pos.z))) >> 4;
        EvalFunctions f;
        std::string s(str);
        std::string origin = s;
        s = toLowerString(s);
        stringReplace(s, "--", "+");
        stringReplace(s, "and", "&&");
        stringReplace(s, "xor", "^");
        stringReplace(s, "or", "||");
        stringReplace(s, "--", "+");
        stringReplace(s, "mod", "%");
        stringReplace(s, "==", "=");
        stringReplace(s, "π", "pi");
        auto x1 = cpp_eval::eval<double>(s.c_str(), variables, f);
        if (std::abs(x1 - round(x1)) < 10E-8) {
            x1 = round(x1);
        }
        if (x1 == -0)
            x1 = 0;
        return x1;
    }
}  // namespace worldedit