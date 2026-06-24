#pragma once
#include <memory>
#include <ostream>
#include <variant>
#include <vector>

// see https://en.cppreference.com/w/cpp/utility/variant/visit
template <class... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;
};
template <class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

namespace regexp {

struct Alternation;
struct Concatenation;
struct Iteration;
struct Symbol;
struct Epsilon;
struct Empty;

using RegExp = std::variant<std::unique_ptr<Alternation>, std::unique_ptr<Concatenation>, std::unique_ptr<Iteration>, std::unique_ptr<Symbol>, std::unique_ptr<Epsilon>, std::unique_ptr<Empty>>;

struct Alternation {
    Alternation(RegExp left, RegExp right);
    RegExp m_left, m_right;
};

struct Concatenation {
    Concatenation(RegExp left, RegExp right);
    RegExp m_left, m_right;
};

struct Iteration {
    Iteration(RegExp node);
    RegExp m_node;
};

struct Symbol {
    Symbol(uint8_t symb);
    uint8_t m_symbol;
};

struct Epsilon {
};

struct Empty {
};

Alternation::Alternation(RegExp left, RegExp right)
    : m_left(std::move(left))
    , m_right(std::move(right))
{
}
Concatenation::Concatenation(RegExp left, RegExp right)
    : m_left(std::move(left))
    , m_right(std::move(right))
{
}
Iteration::Iteration(RegExp node)
    : m_node(std::move(node))
{
}
Symbol::Symbol(uint8_t symb)
    : m_symbol(std::move(symb))
{
}

std::string print(const regexp::RegExp& n)
{
    using namespace std::string_literals;
    return std::visit(overloaded{
                          [](const std::unique_ptr<regexp::Alternation>& arg) { return "("s + print(arg->m_left) + "+" + print(arg->m_right) + ")"; },
                          [](const std::unique_ptr<regexp::Concatenation>& arg) { return "("s + print(arg->m_left) + " " + print(arg->m_right) + ")"; },
                          [](const std::unique_ptr<regexp::Iteration>& arg) { return "("s + print(arg->m_node) + ")*"; },
                          [](const std::unique_ptr<regexp::Symbol>& arg) { return std::string(1, arg->m_symbol); },
                          [](const std::unique_ptr<regexp::Epsilon>& arg) { return "#E"s; },
                          [](const std::unique_ptr<regexp::Empty>& arg) { return "#0"s; },
                      },
                      n);
}
} // namespace regexp

std::ostream& operator<<(std::ostream& os, const regexp::RegExp& regexp)
{
    return os << print(regexp);
}

using Word = std::vector<uint8_t>;

std::ostream& operator<<(std::ostream& os, const Word& word)
{
    os << '"';
    for (auto it = word.begin(); it != word.end(); ++it) {
        if (it != word.begin()) {
            os << ' ';
        }

        const auto& c = *it;

        if (std::isprint(c)) {
            os << c;
        } else {
            os << "0x" << std::hex << int(c) << std::dec;
        }
    }
    return os << '"';
}
