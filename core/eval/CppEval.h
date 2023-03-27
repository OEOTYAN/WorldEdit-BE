//////////////////////////////////////////////////////////////////////////////
//
//  Cpp Eval
//
//  https://blog.csdn.net/hollysky/article/details/345125
//
#pragma once
#ifndef CPP_EVAL_H_DFF520DB406EDCF31AB9A538F7E1C3BD_20040721__
#define CPP_EVAL_H_DFF520DB406EDCF31AB9A538F7E1C3BD_20040721__

#include <string>
#include <cmath>
#include <unordered_map>
#include <vector>
#include <sstream>
#include <stdexcept>
#include <cstdlib>
#include "llapi/utils/StringHelper.h"

namespace cpp_eval {
    template <typename number>
    number eval(std::string_view expression);

    template <typename number>
    number eval(std::string_view expression, const ::phmap::flat_hash_map<::std::string, number>& variables);

    template <typename number, typename functions>
    number eval(std::string_view expression,
                const ::phmap::flat_hash_map<::std::string, number>& variables,
                functions& funcs);

    template <typename number>
    class dummy_functions {
       public:
        number operator()(std::string_view, const ::std::vector<number>& params) { return 0; }
    };

    template <typename number>
    number eval(std::string_view expression) {
        ::phmap::flat_hash_map<::std::string, number> variables;
        dummy_functions<number> funcs;
        return eval(expression, variables, funcs);
    }

    template <typename number>
    number eval(std::string_view expression, const ::phmap::flat_hash_map<::std::string, number>& variables) {
        dummy_functions<number> funcs;
        return eval(expression, variables, funcs);
    }

    template <typename number, typename functions>
    class evaler {
        const ::phmap::flat_hash_map<::std::string, number>& mVariables;
        functions& mFuncs;
        std::string_view mStr;
        std::size_t iter;

        enum Type {
            LEFT_BRACKET = '(',
            RIGHT_BRACKET = ')',
            PARAMETER_SEPERATOR = ',',
            IDENTIFIER = 257,
            NUMBER = 258,
            FINISHED = 259,

            POWER = '^',  //**

            MULTIPLY = '*',
            DIVIDE = '/',
            MOD = '%',

            NOT = '!',
            ADD_OR_POSITIVE = '+',
            SUBTRACT_OR_NEGATIVE = '-',

            LESS_THAN = '<',              //<
            LESS_THAN_OR_EQUAL = 262,     //<=
            GREATER_THAN = '>',           //>
            GREATER_THAN_OR_EQUAL = 264,  //>=

            EQUAL = '=',      //==
            NOT_EQUAL = 266,  //!=

            AND = '&',
            XOR = 267,  //^^
            OR = '|',

            LOGIC_AND = 268,  //&&
            LOGIC_OR = 269    //||
        };
        Type mType;
        std::string mIdentifier;
        number mValue;

        void look_ahead() {
            for (;;) {
                if (iter == mStr.size()) {
                    mType = FINISHED;
                    return;
                }
                if (isspace(mStr[iter])) {
                    ++iter;
                    continue;
                }

                switch (do_hash2(mStr.substr(iter, 2))) {
                    case do_hash2("||"):
                        mType = LOGIC_OR, iter += 2;
                        break;
                    case do_hash2("=="):
                        mType = EQUAL, iter += 2;
                        break;
                    case do_hash2("^^"):
                        mType = XOR, iter += 2;
                        break;
                    case do_hash2("<="):
                        mType = LESS_THAN_OR_EQUAL, iter += 2;
                        break;
                    case do_hash2(">="):
                        mType = GREATER_THAN_OR_EQUAL, iter += 2;
                        break;
                    case do_hash2("!="):
                        mType = NOT_EQUAL, iter += 2;
                        break;
                    case do_hash2("&&"):
                        mType = LOGIC_AND, iter += 2;
                        break;
                    default:

                        switch (mStr[iter]) {
                            case ADD_OR_POSITIVE:
                            case SUBTRACT_OR_NEGATIVE:
                            case MULTIPLY:
                            case DIVIDE:
                            case MOD:
                            case LEFT_BRACKET:
                            case RIGHT_BRACKET:
                            case PARAMETER_SEPERATOR:
                            case LESS_THAN:
                            case GREATER_THAN:
                            case AND:
                            case POWER:
                            case OR:
                            case NOT:
                                mType = (Type)mStr[iter], ++iter;
                                break;
                            default:
                                if (isalpha(mStr[iter])) {
                                    mType = IDENTIFIER;
                                    mIdentifier.clear();
                                    mIdentifier += mStr[iter];
                                    ++iter;
                                    while (isalpha(mStr[iter]) || isdigit(mStr[iter]) || mStr[iter] == '_' ||
                                           mStr[iter] == ':')
                                        mIdentifier += mStr[iter], ++iter;
                                } else {
                                    mType = NUMBER;
                                    std::istringstream iss(asString(mStr.substr(iter)));
                                    // iss.setf(std::ios::fixed);
                                    // iss.precision(18);
                                    iss >> mValue;
                                    if (!iss)
                                        return;
                                    iter += iss.rdbuf()->pubseekoff(0, std::ios::cur, std::ios::in);
                                }
                                break;
                        }
                        break;
                }
                break;
            }
        }

        void match(Type type) {
            if (mType == type)
                look_ahead();
            else
                return;
        }

        number expression() {  // expression        ->    higher_expression
            // expression_R
            number result = expression_LOGIC_OR(LOGIC_AND_expression());
            return result;
        }

        number expression_LOGIC_OR(const number& left) {
            number result = left;
            if (mType == LOGIC_OR)
                match(LOGIC_OR),
                    result = expression_LOGIC_OR(
                        (((left > 0.5) ? 1.0 : 0.0) + ((LOGIC_AND_expression() > 0.5) ? 1.0 : 0.0) > 0.5) ? 1.0 : 0.0);
            return result;
        }

        number LOGIC_AND_expression() { return expression_LOGIC_AND(OR_expression()); }

        number expression_LOGIC_AND(const number& left) {
            number result = left;
            if (mType == LOGIC_AND)
                match(LOGIC_AND),
                    result = expression_LOGIC_AND(((left > 0.5) ? 1.0 : 0.0) * ((OR_expression() > 0.5) ? 1.0 : 0.0));
            return result;
        }

        number OR_expression() { return expression_OR(XOR_expression()); }

        number expression_OR(const number& left) {
            number result = left;
            if (mType == OR)
                match(OR), result = expression_OR(static_cast<number>(static_cast<long long>(round(left)) |
                                                                      static_cast<long long>(round(XOR_expression()))));
            return result;
        }

        number XOR_expression() { return expression_XOR(AND_expression()); }

        number expression_XOR(const number& left) {
            number result = left;
            if (mType == XOR)
                match(XOR), result = expression_XOR(static_cast<number>(
                                static_cast<long long>(round(left)) ^ static_cast<long long>(round(AND_expression()))));
            return result;
        }

        number AND_expression() { return expression_AND(EQUAL_expression()); }

        number expression_AND(const number& left) {
            number result = left;
            if (mType == AND)
                match(AND),
                    result = expression_AND(static_cast<number>(static_cast<long long>(round(left)) &
                                                                static_cast<long long>(round(EQUAL_expression()))));
            return result;
        }

        number EQUAL_expression() { return expression_EQUAL(COMPARE_expression()); }

        number expression_EQUAL(const number& left) {
            number result = left;
            if (mType == NOT_EQUAL)
                match(NOT_EQUAL), result = expression_EQUAL(left != COMPARE_expression() ? 1.0 : 0.0);
            else if (mType == EQUAL)
                match(EQUAL), result = expression_EQUAL(left == COMPARE_expression() ? 1.0 : 0.0);
            return result;
        }

        number COMPARE_expression() { return expression_COMPARE(low_expression()); }

        number expression_COMPARE(const number& left) {
            number result = left;
            if (mType == GREATER_THAN_OR_EQUAL)
                match(GREATER_THAN_OR_EQUAL), result = expression_COMPARE(left >= low_expression() ? 1.0 : 0.0);
            else if (mType == GREATER_THAN)
                match(GREATER_THAN), result = expression_COMPARE(left > low_expression() ? 1.0 : 0.0);
            else if (mType == LESS_THAN_OR_EQUAL)
                match(LESS_THAN_OR_EQUAL), result = expression_COMPARE(left <= low_expression() ? 1.0 : 0.0);
            else if (mType == LESS_THAN)
                match(LESS_THAN), result = expression_COMPARE(left < low_expression() ? 1.0 : 0.0);
            return result;
        }

        number low_expression() { return expression_R(higher_expression()); }

        number expression_R(const number& left) {  // expression_R        -> +
            // higher_expression expression_R | -
            // higher_expression expression_R | /e/
            number result = left;
            if (mType == ADD_OR_POSITIVE)
                match(ADD_OR_POSITIVE), result = expression_R(left + higher_expression());
            else if (mType == SUBTRACT_OR_NEGATIVE)
                match(SUBTRACT_OR_NEGATIVE), result = expression_R(left - higher_expression());
            return result;
        }

        number higher_expression() {  // higher_expression    -> sign_expression
            // higher_expression_R
            return higher_expression_R(sign_expression());
        }

        number higher_expression_R(const number& left) {  // higher_expression_R    ->    *
            // sign_expression higher_expression_R | /
            // sign_expression higher_expression_R | /e/
            number result = left;
            if (mType == MULTIPLY)
                match(MULTIPLY), result = higher_expression_R(left * sign_expression());
            else if (mType == DIVIDE)
                match(DIVIDE), result = higher_expression_R(left / sign_expression());
            else if (mType == MOD)
                match(MOD), result = higher_expression_R(fmod(left, sign_expression()));
            return result;
        }

        number sign_expression() {  // sign_expression        ->    + sign_expression |
            // - sign_expression | power_expression
            number result;
            if (mType == ADD_OR_POSITIVE)
                match(ADD_OR_POSITIVE), result = sign_expression();
            else if (mType == SUBTRACT_OR_NEGATIVE)
                match(SUBTRACT_OR_NEGATIVE), result = -sign_expression();
            else if (mType == NOT)
                match(NOT), result = (static_cast<number>(1) - sign_expression());
            else
                result = power_expression();
            return result;
        }

        number power_expression() {  // power_expression    ->    factor **
            // power_expression | factor
            number result = factor();
            if (mType == POWER)
                match(POWER), result = std::pow(result, (power_expression()));
            return result;
        }

        number factor() {  // factor            ->    number | ( expression ) |
            // lang_structure
            number result;
            if (mType == NUMBER)
                result = mValue, match(NUMBER);
            else if (mType == LEFT_BRACKET)
                match(LEFT_BRACKET), result = expression(), match(RIGHT_BRACKET);
            else
                result = lang_structure();
            return result;
        }

        number lang_structure() {  // lang_structure        ->    identifier lang_tail
            std::string id = mIdentifier;
            match(IDENTIFIER);
            return lang_tail(id);
        }

        number lang_tail(std::string_view id) {  // lang_tail        ->    (
            // parameter_list | /e/
            number result;
            if (mType == LEFT_BRACKET) {
                match(LEFT_BRACKET);
                std::vector<number> param = parameter_list();
                result = mFuncs(id, param);
            } else {
                switch (do_hash2(id)) {
                    case do_hash2("pi"):
                    case do_hash2("π"):
                        return 3.141592653589793238462643383279;
                    case do_hash2("phi"):
                    case do_hash2("φ"):
                        return 0.618033988749894848204586834365;
                    case do_hash2("γ"):
                        return 0.577215664901532860606512090082;
                    case do_hash2("e"):
                        return 2.718281828459045235360287471352;

                        // cellular disFunc
                    case do_hash2("sqrted"):
                        return 0;
                    case do_hash2("square"):
                        return 1;
                    case do_hash2("manhattan"):
                        return 2;
                    case do_hash2("hybird"):
                        return 3;

                        // cellular returnType
                    case do_hash2("value"):
                        return 0;
                    case do_hash2("dis"):
                    case do_hash2("dis1"):
                        return 1;
                    case do_hash2("dis2"):
                        return 2;
                    case do_hash2("disadd"):
                        return 3;
                    case do_hash2("dissub"):
                        return 4;
                    case do_hash2("dismul"):
                        return 5;
                    case do_hash2("disdiv"):
                        return 6;

                        // fractalType
                    case do_hash2("none"):
                        return 0;
                    case do_hash2("fbm"):
                        return 1;
                    case do_hash2("ridged"):
                        return 2;
                    case do_hash2("pingpong"):
                        return 3;
                        // simplex(x,y,z,seed,fractalType,octaves,lacunarity,gain,weighted,ppStrength)
                        // perlin(x,y,z,seed,fractalType,octaves,lacunarity,gain,weighted,ppStrength)
                        // cubic(x,y,z,seed,fractalType,octaves,lacunarity,gain,weighted,ppStrength)
                        // value(x,y,z,seed,fractalType,octaves,lacunarity,gain,weighted,ppStrength)
                        // voronoi(x,y,z,seed,returnType,disFunc,jitter,fractalType,octaves,lacunarity,gain,weighted,ppStrength)
                    default:
                        if (!mVariables.contains(id))
                            return 0;
                        result = mVariables.find(id)->second;
                        break;
                }
            }
            return result;
        }

        std::vector<number> parameter_list() {  // parameter_list        ->    )
            // | expression parameter_tail
            std::vector<number> result;
            for (;;) {
                if (mType == RIGHT_BRACKET) {
                    match(RIGHT_BRACKET);
                    break;
                }
                result.push_back(expression());
                parameter_tail(result);
            }
            return result;
        }

        void parameter_tail(std::vector<number>& param) {  // parameter_tail        ->    ,
            // expression parameter_tail | /e/
            if (mType == PARAMETER_SEPERATOR)
                match(PARAMETER_SEPERATOR), param.push_back(expression()), parameter_tail(param);
        }

       public:
        evaler(const ::phmap::flat_hash_map<::std::string, number>& variables, functions& funcs)
            : mVariables(variables), mFuncs(funcs) {}

        number operator()(std::string_view expr) {
            mStr = expr;
            iter = 0;
            look_ahead();
            number result = expression();
            if (mType != FINISHED)
                return 0;
            return result;
        }
    };

    template <typename number, typename functions>
    number eval(std::string_view expression,
                const ::phmap::flat_hash_map<::std::string, number>& variables,
                functions& funcs) {
        return evaler<number, functions>(variables, funcs)(expression);
    }
};  // namespace cpp_eval

#endif  // CPP_EVAL_H_DFF520DB406EDCF31AB9A538F7E1C3BD_20040721__