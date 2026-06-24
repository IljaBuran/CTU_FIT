#ifndef __PROGTEST__
#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <functional>
#include <iomanip>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <numeric>
#include <optional>
#include <queue>
#include <random>
#include <set>
#include <sstream>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <variant>
#include <vector>

#include "sample.h"
#endif

using symbol   = uint8_t;
using position = uint32_t;

struct RegExprContext
{
    struct RegExprProperties
    {
        bool e_ = false;
        std::set<position> first_, last_;
        std::map<position, std::set<position>> neighbors_;
    };
    
    position posIdx = 0;
    std::map<position, symbol> posToSymbol;
    RegExprProperties regExprProps;

    RegExprContext()                      = delete;
    RegExprContext(const RegExprContext&) = delete;
    void operator=(const RegExprContext&) = delete;

    RegExprContext(const regexp::RegExp& r)
        : regExprProps(ProcessRegExprProperties(r)) {}

    RegExprProperties ProcessRegExprProperties(const regexp::RegExp& r)
    {    
        if (std::holds_alternative<std::unique_ptr<regexp::Alternation>>(r))
        {
            const auto& alt = *std::get<std::unique_ptr<regexp::Alternation>>(r);
            RegExprProperties leftChildRep = ProcessRegExprProperties(alt.m_left);
            RegExprProperties rightChildRep = ProcessRegExprProperties(alt.m_right);

            RegExprProperties rep
            {
                .e_ = leftChildRep.e_ || rightChildRep.e_,
                .first_ = leftChildRep.first_,
                .last_ = leftChildRep.last_,
                .neighbors_ = leftChildRep.neighbors_
            };
            
            rep.first_.insert(rightChildRep.first_.begin(), rightChildRep.first_.end());
            rep.last_.insert(rightChildRep.last_.begin(), rightChildRep.last_.end());

            for (const auto& [p, neigh] : rightChildRep.neighbors_) 
                rep.neighbors_[p].insert(neigh.begin(), neigh.end());

            return rep;
        }
        else if (std::holds_alternative<std::unique_ptr<regexp::Concatenation>>(r))
        {
            const auto& con = *std::get<std::unique_ptr<regexp::Concatenation>>(r);
            RegExprProperties leftChildRep = ProcessRegExprProperties(con.m_left);
            RegExprProperties rightChildRep = ProcessRegExprProperties(con.m_right);

            RegExprProperties rep
            {
                .e_ = leftChildRep.e_ && rightChildRep.e_,
                .first_ = {},
                .last_ = {},
                .neighbors_ = {}
            };

            rep.first_ = leftChildRep.first_;
            if (leftChildRep.e_)
                rep.first_.insert(rightChildRep.first_.begin(), rightChildRep.first_.end());
            if (rightChildRep.e_)
            {
                rep.last_ = leftChildRep.last_;
                rep.last_.insert(rightChildRep.last_.begin(), rightChildRep.last_.end());
            }
            else
                rep.last_ = rightChildRep.last_;
            
            
            rep.neighbors_ = leftChildRep.neighbors_;

            for (const auto& [p, neigh] : rightChildRep.neighbors_) 
                rep.neighbors_[p].insert(neigh.begin(), neigh.end());

            for (position s : leftChildRep.last_)
                rep.neighbors_[s].insert(rightChildRep.first_.begin(), rightChildRep.first_.end());

            return rep;
        }
        else if (std::holds_alternative<std::unique_ptr<regexp::Empty>>(r))
        {
            return
            {
                .e_ = false,
                .first_ = {},
                .last_ = {},
                .neighbors_ = {}
            };
        }
        else if (std::holds_alternative<std::unique_ptr<regexp::Epsilon>>(r))
        {
            return
            {
                .e_ = true,
                .first_ = {},
                .last_ = {},
                .neighbors_ = {}
            };
        }
        else if (std::holds_alternative<std::unique_ptr<regexp::Iteration>>(r))
        {
            const auto& it = *std::get<std::unique_ptr<regexp::Iteration>>(r);
            RegExprProperties childRep = ProcessRegExprProperties(it.m_node);

            RegExprProperties rep 
            {
                .e_ = true,
                .first_ = childRep.first_,
                .last_ = childRep.last_,
                .neighbors_ = childRep.neighbors_
            };

            for (position s : childRep.last_)
                rep.neighbors_[s].insert(childRep.first_.begin(), childRep.first_.end());

            return rep;
        }
        else if (std::holds_alternative<std::unique_ptr<regexp::Symbol>>(r))
        {
            const symbol s = std::get<std::unique_ptr<regexp::Symbol>>(r)->m_symbol;
            posToSymbol[posIdx] = s;   
            return
            {
                .e_ = false,
                .first_ = {posIdx},
                .last_ = {posIdx++},
                .neighbors_ = {}
            };  
        }
        throw std::runtime_error("ProcessRegExprProperties(): something's gone wrong!");
    }
};

std::set<size_t> wordsMatch(const regexp::RegExp&    regexp,
                            const std::vector<Word>& words)
{
    RegExprContext ctx(regexp);
    RegExprContext::RegExprProperties& props = ctx.regExprProps;

    // is symbol last ? then add edge to finalPos
    const position finalPos = ctx.posIdx;

    for (position p : props.last_)
        props.neighbors_[p].insert(finalPos);

    std::set<size_t> result;

    for (size_t i = 0; i < words.size(); i++)
    {
        std::set<position> curr = props.first_;
        if (props.e_ && !words[i].size())
        {
            result.insert(i);
            continue;
        }

        for (symbol s : words[i])
        {
            if (curr.empty()) // in finalPos
                break;

            std::set<position> next;

            for (position p : curr)
            {
                if (p == finalPos) // from finalPos we don't continue accepting symblols
                    continue;

                if (auto it = ctx.posToSymbol.find(p); it == ctx.posToSymbol.end() || it->second != s)
                    continue;

                auto it = ctx.regExprProps.neighbors_.find(p);
                if (it != ctx.regExprProps.neighbors_.end())
                    next.insert(it->second.begin(), it->second.end());
            };

            if (next.empty()) // no valid transition
            {
                curr.clear();
                break;
            }
            curr.swap(next);
        }

        if (curr.count(finalPos))
            result.insert(i);
    }

    return result;
}
#ifndef __PROGTEST__
int main()
{
    // basic test 1
    regexp::RegExp re1 = std::make_unique<regexp::Iteration>(
        std::make_unique<regexp::Concatenation>(
            std::make_unique<regexp::Concatenation>(
                std::make_unique<regexp::Concatenation>(
                    std::make_unique<regexp::Iteration>(
                        std::make_unique<regexp::Alternation>(
                            std::make_unique<regexp::Symbol>('a'),
                            std::make_unique<regexp::Symbol>('b'))),
                    std::make_unique<regexp::Symbol>('a')),
                std::make_unique<regexp::Symbol>('b')),
            std::make_unique<regexp::Iteration>(
                std::make_unique<regexp::Alternation>(
                    std::make_unique<regexp::Symbol>('a'),
                    std::make_unique<regexp::Symbol>('b')))));
    #ifdef _DEBUG
    std::cout << re1 << '\n';
    #endif
    assert(wordsMatch(re1, {Word{}}) == std::set<size_t>{0});
    assert(wordsMatch(re1, {Word{'a', 'b'}}) == std::set<size_t>{0});
    assert(wordsMatch(re1, {Word{'a'}}) == std::set<size_t>{});
    assert(wordsMatch(re1, {Word{'a', 'a', 'a', 'a'}}) == std::set<size_t>{});
    assert(wordsMatch(re1, {Word{'a', 'a', 'a', 'c'}}) == std::set<size_t>{});
    assert(wordsMatch(re1, {Word{'a', 'a', 0x07, 'c'}}) == std::set<size_t>{});
    assert(wordsMatch(re1, {Word{'a', 'a', 'b'}}) == std::set<size_t>{0});
    assert(wordsMatch(re1, {Word{'a', 'a', 'b', 'a', 'a', 'b', 'a', 'a', 'b', 'a', 'a', 'b', 'a', 'a', 'b', 'a', 'a', 'b'}}) == std::set<size_t>{0});
    assert((wordsMatch(re1, {Word{}, Word{'a', 'b'}, Word{'a'}, Word{'a', 'a', 'a', 'a'}, Word{'a', 'a', 'a', 'c'}, Word{'a', 'a', 0x07, 'c'}, Word{'a', 'a', 'b'}, Word{'a', 'a', 'b', 'a', 'a', 'b', 'a', 'a', 'b', 'a', 'a', 'b', 'a', 'a', 'b', 'a', 'a', 'b'}}) == std::set<size_t>{0, 1, 6, 7}));

    // basic test 2
    regexp::RegExp re2 = std::make_unique<regexp::Concatenation>(
        std::make_unique<regexp::Concatenation>(
            std::make_unique<regexp::Iteration>(
                std::make_unique<regexp::Concatenation>(
                    std::make_unique<regexp::Concatenation>(
                        std::make_unique<regexp::Iteration>(
                            std::make_unique<regexp::Alternation>(
                                std::make_unique<regexp::Symbol>('a'),
                                std::make_unique<regexp::Symbol>('b'))),
                        std::make_unique<regexp::Iteration>(
                            std::make_unique<regexp::Alternation>(
                                std::make_unique<regexp::Symbol>('c'),
                                std::make_unique<regexp::Symbol>('d')))),
                    std::make_unique<regexp::Iteration>(
                        std::make_unique<regexp::Alternation>(
                            std::make_unique<regexp::Symbol>('e'),
                            std::make_unique<regexp::Symbol>('f'))))),
            std::make_unique<regexp::Empty>()),
        std::make_unique<regexp::Iteration>(
            std::make_unique<regexp::Alternation>(
                std::make_unique<regexp::Symbol>('a'),
                std::make_unique<regexp::Symbol>('b'))));
    #ifdef _DEBUG
    std::cout << re2 << '\n';
    #endif
    assert(wordsMatch(re2, {Word{}}) == std::set<size_t>{});
    assert(wordsMatch(re2, {Word{'a', 'b'}}) == std::set<size_t>{});
    assert(wordsMatch(re2, {Word{'a', 'b', 'c', 'd'}}) == std::set<size_t>{});
    assert(wordsMatch(re2, {Word{'a', 'b', 'c', 'd', 'e', 'f'}}) == std::set<size_t>{});
    assert(wordsMatch(re2, {Word{'a', 'b', 'c', 'd', 'e', 'f', 'a', 'b'}}) == std::set<size_t>{});
    assert((wordsMatch(re2, {Word{}, Word{'a', 'b'}, Word{'a', 'b', 'c', 'd'}, Word{'a', 'b', 'c', 'd', 'e', 'f'}, Word{'a', 'b', 'c', 'd', 'e', 'f', 'a', 'b'}}) == std::set<size_t>{}));

    regexp::RegExp re3 = std::make_unique<regexp::Concatenation>(
        std::make_unique<regexp::Concatenation>(
            std::make_unique<regexp::Concatenation>(
                std::make_unique<regexp::Symbol>('0'),
                std::make_unique<regexp::Symbol>('1')),
            std::make_unique<regexp::Symbol>('1')),
        std::make_unique<regexp::Iteration>(
            std::make_unique<regexp::Alternation>(
                std::make_unique<regexp::Alternation>(
                    std::make_unique<regexp::Concatenation>(
                        std::make_unique<regexp::Concatenation>(
                            std::make_unique<regexp::Symbol>('0'),
                            std::make_unique<regexp::Symbol>('1')),
                        std::make_unique<regexp::Symbol>('1')),
                    std::make_unique<regexp::Concatenation>(
                        std::make_unique<regexp::Concatenation>(
                            std::make_unique<regexp::Symbol>('1'),
                            std::make_unique<regexp::Iteration>(
                                std::make_unique<regexp::Symbol>('0'))),
                        std::make_unique<regexp::Symbol>('1'))),
                std::make_unique<regexp::Symbol>('0'))));
    #ifdef _DEBUG
    std::cout << re3 << '\n';
    #endif
    assert(wordsMatch(re3, {Word{'0', '1'}}) == std::set<size_t>{});
    assert(wordsMatch(re3, {Word{'0', '1', '1'}}) == std::set<size_t>{0});
    assert(wordsMatch(re3, {Word{'0', '1', '1', '0'}}) == std::set<size_t>{0});
    assert(wordsMatch(re3, {Word{'0', '1', '1', '0', '1', '1', '1', '0', '0', '0'}}) == std::set<size_t>{});
    assert(wordsMatch(re3, {Word{'0', '1', '1', '0', '1', '1', '1', '0', '0', '1'}}) == std::set<size_t>{0});
    assert(wordsMatch(re3, {Word{'0', '1', '1', '0', '1', '1', '1', '0', '0', '1', '0'}}) == std::set<size_t>{0});
    assert((wordsMatch(re3, {Word{'0', '1'}, Word{'0', '1', '1'}, Word{'0', '1', '1', '0'}, Word{'0', '1', '1', '0', '1', '1', '1', '0', '0', '0'}, Word{'0', '1', '1', '0', '1', '1', '1', '0', '0', '1'}, Word{'0', '1', '1', '0', '1', '1', '1', '0', '0', '1', '0'}}) == std::set<size_t>{1, 2, 4, 5}));
}
#endif
