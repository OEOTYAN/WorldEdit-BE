//
// Created by OEOTYAN on 2022/06/09.
//
#pragma once
#include <mc/BlockInstance.hpp>
#include <mc/Player.hpp>
#include "eval/Eval.h"
#include "store/Patterns.h"

namespace worldedit {
    class Brush {
       public:
        unsigned short size = 0;
        std::unique_ptr<Pattern> pattern = nullptr;
        std::string mask = "";
        bool needFace = false;
        bool lneedFace = false;
        Brush() {}
        Brush(unsigned short, std::unique_ptr<Pattern> p);
        void setMask(std::string const& str = "") { mask = str; };
        virtual long long set(class Player*, class ::BlockInstance);
        virtual long long lset(class Player*, class ::BlockInstance);
        bool maskFunc(class EvalFunctions& func,
                      const phmap::flat_hash_map<std::string, double>& var,
                      std::function<void()> const& todo);
    };
}  // namespace worldedit
