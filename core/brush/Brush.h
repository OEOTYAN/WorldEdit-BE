//
// Created by OEOTYAN on 2022/06/09.
//
#pragma once
#include <mc/BlockInstance.hpp>
#include <mc/Player.hpp>
#include "eval/Eval.h"
namespace worldedit {
    class Brush {
       public:
        unsigned short size = 0;
        class BlockPattern* pattern = nullptr;
        bool setted = false;
        std::string mask = "";
        bool needFace = false;
        bool lneedFace = false;
        Brush(){}
        Brush(unsigned short, BlockPattern*);
        void setMask(std::string const& str = "") { mask = str; };
        virtual long long set(class Player*, class ::BlockInstance);
        virtual long long lset(class Player* , class ::BlockInstance);
     bool maskFunc(class EvalFunctions& func, const std::unordered_map<std::string, double>& var,
                             std::function<void()> const& todo);

        virtual ~Brush();
    };
}  // namespace worldedit
