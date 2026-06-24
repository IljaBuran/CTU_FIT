#ifndef __PROGTEST__
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <functional>
#include <iostream>
#include <limits>
#include <list>
#include <map>
#include <optional>
#include <queue>
#include <set>
#include <sstream>
#include <stack>
#include <vector>
#include <unordered_map>
#include <unordered_set>

using State = unsigned;
using Symbol = uint8_t;
using Word = std::vector<Symbol>;

struct MISNFA {
    std::set<State> states;
    std::set<Symbol> alphabet;
    std::map<std::pair<State, Symbol>, std::set<State>> transitions;
    std::set<State> initialStates;
    std::set<State> finalStates;

    void Print() const
    {
        std::cout << "States: { ";
        for (const auto& state : states)
        {
            std::cout << state << " ";
        }
        std::cout << "}" << std::endl;

        std::cout << "Alphabet: { ";
        for (const auto& symbol : alphabet)
        {
            std::cout << symbol << " ";
        }
        std::cout << "}" << std::endl;

        std::cout << "Transitions:" << std::endl;
        for (const auto& [key, nextStates] : transitions)
        {
            const auto& [currentState, symbol] = key;
            std::cout << "  (" << currentState << ", " << symbol << ") -> { ";
            for (const auto& nextState : nextStates)
            {
                std::cout << nextState << " ";
            }
            std::cout << "}" << std::endl;
        }

        std::cout << "Initial States: { ";
        for (const auto& state : initialStates)
        {
            std::cout << state << " ";
        }
        std::cout << "}" << std::endl;

        std::cout << "Final States: { ";
        for (const auto& state : finalStates)
        {
            std::cout << state << " ";
        }
        std::cout << "}" << std::endl;
    }
};

struct DFA {
    std::set<State> states;
    std::set<Symbol> alphabet;
    std::map<std::pair<State, Symbol>, State> transitions;
    State initialState;
    std::set<State> finalStates;

    bool operator==(const DFA& x) const = default; // requires c++20

    void Print() const
    {
        std::cout << "States: { ";
        for (const auto& state : states)
        {
            std::cout << state << " ";
        }
        std::cout << "}" << std::endl;

        std::cout << "Alphabet: { ";
        for (const auto& symbol : alphabet)
        {
            std::cout << symbol << " ";
        }
        std::cout << "}" << std::endl;

        std::cout << "Transitions:" << std::endl;
        for (const auto& [key, nextState] : transitions)
        {
            const auto& [currentState, symbol] = key;
            std::cout << "  (" << currentState << ", " << symbol << ") -> " << nextState << std::endl;
        }

        std::cout << "Initial State: " << initialState << std::endl;

        std::cout << "Final States: { ";
        for (const auto& state : finalStates)
        {
            std::cout << state << " ";
        }
        std::cout << "}" << std::endl;
    }
};
#endif

bool Determinize(const MISNFA& nfa, DFA& dfa)
{
    /* Declaring res, will be returned assigned to the dfa */
    DFA res;
    
    /* if alphabet of nfa is empty -> then return 1 state dfa */
    
    #ifdef DEBUG
    std::cout << std::boolalpha << "Alphabet empty: " << nfa.alphabet.empty() << '\n';
    #endif 

    if (nfa.alphabet.empty())
    {
        res.states.insert(0);
        res.initialState = 0;
        
        bool found = false;

        for (auto initial_state : nfa.initialStates)
            if (nfa.finalStates.find(initial_state) != nfa.finalStates.end())
                found = true;

        if (!found)
            res.finalStates.insert(0);

        dfa = res;
        return false;
    }
    
    /* Assigning nfa alphabet to dfa */
    res.alphabet = nfa.alphabet;

    /* creating map to keep track of set of states from nfa and assigned number of state for dfa */ 
    std::map<std::set<State>, State> state_mapping;
    /* tracking of no. of new state */
    size_t count = 0;

    /* Adding state 0 into dfa (initial state derived from nfa set of initial states) */
    state_mapping[nfa.initialStates] = count;
    /* Setting state 0 as initial state of dfa */
    res.initialState = count;
    /* State 0 inserted into set of dfa states */
    res.states.insert(count);
    /* incrasing number of states */
    count++;
    
    /* declaring unprocessed_states, this will contain all new states added yet to be processed */
    std::queue<std::set<State>> unprocessed_states;
    /* pushed new state (set of states) */
    unprocessed_states.push(nfa.initialStates);

    /* main algorithm of conversion of nfa -> dfa */
    while (!unprocessed_states.empty())
    {
        /* take one set of states to process and remove it from processed */
        std::set<State> current_states = unprocessed_states.front();
        unprocessed_states.pop();
        
        State current_dfa_state = state_mapping[current_states];

        /* for each symbol in alphabet */
        for (auto symbol_in_alphabet : nfa.alphabet)
        {
            std::set<State> next_states_set;

            /* for each state in set of states */
            for (auto x : current_states)
            {
                /* find if there is a transition for a state and symbol */
                auto it = nfa.transitions.find(std::make_pair(x, symbol_in_alphabet));
                if (it != nfa.transitions.end())
                {
                    /* if so, take the set of next states for state and symbol*/
                    const auto& nextStates = it->second;
                    /* add all states */
                    next_states_set.insert(nextStates.begin(), nextStates.end());
                }
            }

            if (!next_states_set.empty())
            {
                // Check if this set of states has been assigned a DFA state number
                if (state_mapping.find(next_states_set) == state_mapping.end())
                {
                    /* if not found -> create new state */
                    state_mapping[next_states_set] = count;
                    res.states.insert(count);
                    unprocessed_states.push(next_states_set);
                    State next_dfa_state = count++;
                    res.transitions[{current_dfa_state, symbol_in_alphabet}] = next_dfa_state;
                }
                else
                {
                    /* if found -> search for the state */
                    State next_dfa_state = state_mapping[next_states_set];
                    res.transitions[{current_dfa_state, symbol_in_alphabet}] = next_dfa_state;
                }
            }
        }
    }

    /* Assigning final states to dfa */
    /* Every dfa state derived from at least 1 nfa final state will be final state */
    for (const auto& [nfa_states_set, dfa_state_num] : state_mapping)
        for (auto s : nfa.finalStates)
            if (nfa_states_set.find(s) != nfa_states_set.end())
            {
                res.finalStates.insert(dfa_state_num);
                break;
            }

    dfa = res;
    return true;
}

/* If there are no transitions for [State, Symbol] -> ?, then make a dead state, where all those transitions will go */
/* Switches final states */
void CompleteAndSwitch(DFA& dfa)
{  
    State dead = dfa.states.size();

    bool dead_state_needed = false;

    /* loop -> going through every combination of [state, symbol], if there is no transition, make a transition to dead state */
    for (auto it = dfa.states.begin(); it != dfa.states.end(); it++)
    {
        /* Switching final states */
        if (dfa.finalStates.find(*it) != dfa.finalStates.end())
            dfa.finalStates.erase(*it);
        else
            dfa.finalStates.insert(*it);
        
        
        for (auto symbol_in_alphabet : dfa.alphabet)
        {
            if (dfa.transitions.find({(*it), symbol_in_alphabet}) == dfa.transitions.end())
            {
                if (!dead_state_needed)
                {
                    dead_state_needed = true;
                    dfa.states.insert(dead);
                }
                dfa.transitions[{*it, symbol_in_alphabet}] = dead;
            }
        }
    }

    /* setting transition of [dead, any symbol] -> dead */
    for (auto symbol_in_alphabet : dfa.alphabet)
    {
        dfa.transitions[{dead, symbol_in_alphabet}] = dead;
    }
    /* if we added dead state, then we make it final */
    if (dead_state_needed)
        dfa.finalStates.insert(dead);

    //dfa.Print();
}

void RemoveInvalidStatesTransitions(DFA& dfa)
{
    for (auto it = dfa.transitions.begin(); it != dfa.transitions.end(); )
        if (dfa.states.find(it->first.first) == dfa.states.end() || dfa.states.find(it->second) == dfa.states.end())
            it = dfa.transitions.erase(it);
        else
            it++;
}

void RemoveStatesFromFinalStateSet(DFA& dfa)
{
    for (auto it = dfa.finalStates.begin(); it != dfa.finalStates.end(); )
        if (dfa.states.find(*it) == dfa.states.end())
            it = dfa.finalStates.erase(it);
        else
            it++;
}

void RemoveUnreachable(DFA& dfa)
{
    std::set<State> reachable_states;
    std::queue<State> queue;

    queue.push(dfa.initialState);
    reachable_states.insert(dfa.initialState);

    /* filling set with reachable states */
    while(!queue.empty())
    {
        State current_state = queue.front();
        queue.pop();

        for (auto symbol_in_alphabet : dfa.alphabet)
        {
            auto it = dfa.transitions.find({current_state, symbol_in_alphabet});
            if (reachable_states.find(it->second) == reachable_states.end())
            {    
                reachable_states.insert(it->second);
                queue.push(it->second);
            }
        }
    }

    /* Assigning found reachable states to dfa states */
    dfa.states = reachable_states;

    /* Removing all the transitions tied to unreachable states */
    RemoveInvalidStatesTransitions(dfa);

    /* remove unreachable states from final states */
    RemoveStatesFromFinalStateSet(dfa);
    //dfa.Print();
}

void RemoveUseless(DFA& dfa)
{
    if (dfa.finalStates.empty())
    {
        std::set<State> temp;
        temp.insert(dfa.initialState);
        dfa.states = temp;
        RemoveInvalidStatesTransitions(dfa);
        //dfa.Print();
        return;
    }
    
    std::set<State> useful_states = dfa.finalStates; 
    std::queue<State> queue;

    for (const auto& final_state : dfa.finalStates)
        queue.push(final_state);

    /* finding all states that can reach a final state */
    while(!queue.empty())
    {
        State current_state = queue.front();
        queue.pop();

        for (const auto& symbol_in_alphabet : dfa.alphabet)
        {
            for (const auto& [key, value] : dfa.transitions)
            {
                if (value == current_state && symbol_in_alphabet == key.second)
                    if(useful_states.find(key.first) == useful_states.end())
                    {
                        useful_states.insert(key.first);
                        queue.push(key.first);
                    }
            }
        }
    }
    /* Assigning found useful states to dfa states */
    dfa.states = useful_states;

    /* Removing all the transitions tied to unreachable states */
    RemoveInvalidStatesTransitions(dfa);
    //dfa.Print();
}

DFA complement(const MISNFA& nfa)
{
    DFA dfa;
    
    if (Determinize(nfa, dfa))
    {
        CompleteAndSwitch(dfa);
        RemoveUnreachable(dfa);
        RemoveUseless(dfa);
    }

    return dfa;
}

bool run(const DFA& dfa, const Word& word)
{
    State current_state = dfa.initialState;

    for (Symbol s : word)
    {
        if (dfa.alphabet.find(s) == dfa.alphabet.end())
            return false;
        
        auto it = dfa.transitions.find({current_state, s});

        if (it != dfa.transitions.end())
            current_state = it->second;

        else
            return false;
    }

    return dfa.finalStates.find(current_state) != dfa.finalStates.end();
}

#ifndef __PROGTEST__

int main()
{
    MISNFA in99 = {
        {0, 1, 2, 3, 4},
        {},
        {
            {{0, 'e'}, {1}},
            {{0, 'l'}, {1}},
            {{1, 'e'}, {2}},
            {{2, 'e'}, {0}},
            {{2, 'l'}, {2}},
        },
        {0},
        {1, 2},
    };
    auto out99 = complement(in99);
    assert(run(out99, {}) == true);
    assert(run(out99, {'e', 'l'}) == false);
    
    
    MISNFA in0 = {
        {0, 1, 2},
        {'e', 'l'},
        {
            {{0, 'e'}, {1}},
            {{0, 'l'}, {1}},
            {{1, 'e'}, {2}},
            {{2, 'e'}, {0}},
            {{2, 'l'}, {2}},
        },
        {0},
        {1, 2},
    };
    auto out0 = complement(in0);
    assert(run(out0, {}) == true);
    assert(run(out0, {'e', 'l'}) == true);
    assert(run(out0, {'l'}) == false);
    assert(run(out0, {'l', 'e', 'l', 'e'}) == true);
    MISNFA in1 = {
        {0, 1, 2, 3},
        {'g', 'l'},
        {
            {{0, 'g'}, {1}},
            {{0, 'l'}, {2}},
            {{1, 'g'}, {3}},
            {{1, 'l'}, {3}},
            {{2, 'g'}, {1}},
            {{2, 'l'}, {0}},
            {{3, 'l'}, {1}},
        },
        {0},
        {0, 2, 3},
    };
    auto out1 = complement(in1);
    assert(run(out1, {}) == false);
    assert(run(out1, {'g'}) == true);
    assert(run(out1, {'g', 'l'}) == false);
    assert(run(out1, {'g', 'l', 'l'}) == true);
    MISNFA in2 = {
        {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11},
        {'a', 'b'},
        {
            {{0, 'a'}, {1}},
            {{0, 'b'}, {2}},
            {{1, 'a'}, {3}},
            {{1, 'b'}, {4}},
            {{2, 'a'}, {5}},
            {{2, 'b'}, {6}},
            {{3, 'a'}, {7}},
            {{3, 'b'}, {8}},
            {{4, 'a'}, {9}},
            {{5, 'a'}, {5}},
            {{5, 'b'}, {10}},
            {{6, 'a'}, {8}},
            {{6, 'b'}, {8}},
            {{7, 'a'}, {11}},
            {{8, 'a'}, {3}},
            {{8, 'b'}, {9}},
            {{9, 'a'}, {5}},
            {{9, 'b'}, {5}},
            {{10, 'a'}, {1}},
            {{10, 'b'}, {2}},
            {{11, 'a'}, {5}},
            {{11, 'b'}, {5}},
        },
        {0},
        {5, 6},
    };
    auto out2 = complement(in2);
    assert(run(out2, {}) == true);
    assert(run(out2, {'a'}) == true);
    assert(run(out2, {'a', 'a', 'a', 'a', 'a', 'b'}) == true);
    assert(run(out2, {'a', 'a', 'a', 'c', 'a', 'a'}) == false);
    MISNFA in3 = {
        {0, 1, 2, 3, 4, 5, 6, 7, 8, 9},
        {'e', 'j', 'k'},
        {
            {{0, 'e'}, {1}},
            {{0, 'j'}, {2}},
            {{0, 'k'}, {3}},
            {{1, 'e'}, {2}},
            {{1, 'j'}, {4}},
            {{1, 'k'}, {5}},
            {{2, 'e'}, {6}},
            {{2, 'j'}, {0}},
            {{2, 'k'}, {4}},
            {{3, 'e'}, {7}},
            {{3, 'j'}, {2}},
            {{3, 'k'}, {1}},
            {{4, 'e'}, {4}},
            {{4, 'j'}, {8}},
            {{4, 'k'}, {7}},
            {{5, 'e'}, {4}},
            {{5, 'j'}, {0}},
            {{5, 'k'}, {4}},
            {{6, 'e'}, {7}},
            {{6, 'j'}, {8}},
            {{6, 'k'}, {4}},
            {{7, 'e'}, {3}},
            {{7, 'j'}, {1}},
            {{7, 'k'}, {8}},
            {{8, 'e'}, {2}},
            {{8, 'j'}, {4}},
            {{8, 'k'}, {9}},
            {{9, 'e'}, {4}},
            {{9, 'j'}, {0}},
            {{9, 'k'}, {4}},
        },
        {0},
        {1, 6, 8},
    };
    auto out3 = complement(in3);
    assert(run(out3, {}) == true);
    assert(run(out3, {'b', 'e', 'e'}) == false);
    assert(run(out3, {'e', 'e', 'e'}) == false);
    assert(run(out3, {'e', 'j'}) == true);
    assert(run(out3, {'e', 'k', 'j', 'e', 'j', 'j', 'j', 'e', 'k'}) == true);
    MISNFA in4 = {
        {0, 1, 2, 3, 4, 5},
        {'e', 'n', 'r'},
        {
            {{0, 'e'}, {1}},
            {{0, 'n'}, {1}},
            {{0, 'r'}, {2}},
            {{1, 'e'}, {2}},
            {{1, 'n'}, {3}},
            {{1, 'r'}, {3}},
            {{2, 'e'}, {3}},
            {{2, 'n'}, {3}},
            {{2, 'r'}, {1}},
            {{3, 'e'}, {1}},
            {{3, 'n'}, {1}},
            {{3, 'r'}, {2}},
            {{4, 'r'}, {5}},
        },
        {0},
        {4, 5},
    };
    auto out4 = complement(in4);
    assert(run(out4, {}) == true);
    assert(run(out4, {'e', 'n', 'r', 'e', 'n', 'r', 'e', 'n', 'r', 'e', 'n', 'r'}) == true);
    assert(run(out4, {'n', 'e', 'r', 'n', 'r', 'r', 'r', 'n', 'e', 'n', 'n', 'm', '\x0c', '\x20'}) == false);
    assert(run(out4, {'r', 'r', 'r', 'e', 'n'}) == true);
    MISNFA in5 = {
        {0, 1, 2, 3, 4, 5, 6},
        {'l', 'q', 't'},
        {
            {{0, 'l'}, {2, 4, 5}},
            {{0, 'q'}, {2}},
            {{0, 't'}, {1}},
            {{1, 'l'}, {0, 2}},
            {{1, 'q'}, {1, 4}},
            {{1, 't'}, {0, 2}},
            {{2, 'l'}, {5}},
            {{2, 'q'}, {0, 4}},
            {{2, 't'}, {0}},
            {{3, 'l'}, {3, 4}},
            {{3, 'q'}, {0}},
            {{3, 't'}, {0, 2}},
            {{4, 't'}, {4}},
            {{5, 'l'}, {0, 2}},
            {{5, 'q'}, {4, 5}},
            {{5, 't'}, {0, 2}},
            {{6, 'l'}, {4, 6}},
            {{6, 'q'}, {0}},
            {{6, 't'}, {0, 2}},
        },
        {2, 4},
        {0, 1, 3, 5, 6},
    };
    auto out5 = complement(in5);
    assert(run(out5, {}) == true);
    assert(run(out5, {'q', 'q'}) == true);
    assert(run(out5, {'q', 'q', 'l'}) == false);
    assert(run(out5, {'q', 'q', 'q'}) == false);

    MISNFA inX;
    inX.states = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    inX.alphabet = {'c', 'i', 'y'};
    
    // Přechody
    // Formát: {state, symbol} -> {set of states}

    // State 0
    inX.transitions[{0, 'c'}] = {1,2,5,9,13,15};
    inX.transitions[{0, 'i'}] = {4,5,6,9};
    inX.transitions[{0, 'y'}] = {0,1,5,6,7,9,14,15};

    // State 1 (initial)
    inX.transitions[{1, 'c'}] = {5};
    inX.transitions[{1, 'i'}] = {1,2,5,8,9,13,15};
    inX.transitions[{1, 'y'}] = {0,2,4,5,6,8,9,12,14};

    // State 2
    inX.transitions[{2, 'c'}] = {9,13,14};
    inX.transitions[{2, 'i'}] = {3,5,6};
    inX.transitions[{2, 'y'}] = {0,1,5,7,9};

    // State 3 (final)
    inX.transitions[{3, 'c'}] = {1,3,5,12,13,14};
    inX.transitions[{3, 'i'}] = {1,3,6,7,9};
    inX.transitions[{3, 'y'}] = {0,1,13};

    // State 4 (final)
    inX.transitions[{4, 'c'}] = {0,1,3,6,7,12};
    inX.transitions[{4, 'i'}] = {0,3,13,15};
    inX.transitions[{4, 'y'}] = {5,8};

    // State 5 (initial & final)
    inX.transitions[{5, 'c'}] = {1,2,5,7};
    inX.transitions[{5, 'i'}] = {1,3,4,7,9,13,15};
    inX.transitions[{5, 'y'}] = {1,7,9,15};

    // State 6 (initial)
    inX.transitions[{6, 'c'}] = {0,1,2,3,8,9};
    inX.transitions[{6, 'i'}] = {1,2,3,4,5,13};
    inX.transitions[{6, 'y'}] = {0,2,4,8};

    // State 7 (final)
    inX.transitions[{7, 'c'}] = {1,3,7,8,13,14};
    inX.transitions[{7, 'i'}] = {0,3,6,8,12,14};
    inX.transitions[{7, 'y'}] = {1,3,9,12};

    // State 8
    inX.transitions[{8, 'c'}] = {0,1,6,8,13,15};
    inX.transitions[{8, 'i'}] = {0,1,3,6,12};
    inX.transitions[{8, 'y'}] = {2,6,13,15};

    // State 9 (final)
    inX.transitions[{9, 'c'}] = {0,6,8,9,14,15};
    inX.transitions[{9, 'i'}] = {1,6,8,9};
    inX.transitions[{9, 'y'}] = {2,3,6,7,9,15};

    // State 10
    inX.transitions[{10, 'c'}] = {1,2,5,7,9,13};
    inX.transitions[{10, 'i'}] = {0,5,7,8,10,13,15};
    inX.transitions[{10, 'y'}] = {0,2,3,10};

    // State 11
    inX.transitions[{11, 'c'}] = {0,3,5,8,9,10,11,13,14};
    inX.transitions[{11, 'i'}] = {2,5};
    inX.transitions[{11, 'y'}] = {0,1,5,8};

    // State 12
    // c: -, i: -, y: 13
    // No transitions on 'c' and 'i', only on 'y'
    // Assuming '-' means no transition, thus no entries for 'c' and 'i'
    inX.transitions[{12, 'y'}] = {13};

    // State 13
    inX.transitions[{13, 'c'}] = {13,14};
    inX.transitions[{13, 'i'}] = {12};
    // No transition on 'y'

    // State 14
    // c: -, i: -, y:13
    // No transitions on 'c' and 'i', only on 'y'
    inX.transitions[{14, 'y'}] = {13};

    // State 15 (final)
    inX.transitions[{15, 'c'}] = {0,1,3,6,7,12};
    inX.transitions[{15, 'i'}] = {0,3,4,13,15};
    inX.transitions[{15, 'y'}] = {5,8};

    // Počáteční stavy
    inX.initialStates = {1,5,6};

    // Konečné stavy
    inX.finalStates = {3,4,5,7,9,15};
    //inX.Print();
    // Výpočet komplementu
    auto outX = complement(inX);
    //outX.Print();
}
#endif
