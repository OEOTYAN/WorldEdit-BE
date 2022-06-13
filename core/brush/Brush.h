//
// Created by OEOTYAN on 2022/06/09.
//
#pragma once
#include <MC/BlockInstance.hpp>
#include <MC/Player.hpp>
namespace worldedit {
    class Brush {
       public:
        unsigned short      size    = 0;
        class BlockPattern* pattern = nullptr;
        bool                setted  = false;
        std::string         mask    = "";
        Brush()                     = default;
        Brush(unsigned short, BlockPattern*);
        void              setMask(std::string const& str = "") { mask = str; };
        virtual long long set(class Player*, class ::BlockInstance);
        ~Brush();
    };
}  // namespace worldedit
