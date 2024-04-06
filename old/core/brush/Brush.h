#pragma once
#include "eval/Eval.h"
#include "store/Patterns.h"
#include <mc/BlockInstance.hpp>
#include <mc/Player.hpp>

namespace we {
class Brush {
public:
    unsigned short           size      = 0;
    std::unique_ptr<Pattern> pattern   = nullptr;
    std::string              mask      = "";
    bool                     needFace  = false;
    bool                     lneedFace = false;
    Brush() {}
    Brush(unsigned short, std::unique_ptr<Pattern> p);
    void              setMask(std::string_view str = "") { mask = str; };
    virtual long long set(class Player*, class ::BlockInstance);
    virtual long long lset(class Player*, class ::BlockInstance);
    bool              maskFunc(
                     class EvalFunctions&                             func,
                     const phmap::flat_hash_map<std::string, double>& var,
                     std::function<void()> const&                     todo
                 );
};
} // namespace we
