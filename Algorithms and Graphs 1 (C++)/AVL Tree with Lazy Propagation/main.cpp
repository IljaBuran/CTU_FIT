#ifndef __PROGTEST__
#include <cassert>
#include <iomanip>
#include <cstdint>
#include <iostream>
#include <memory>
#include <limits>
#include <optional>
#include <algorithm>
#include <functional>
#include <bitset>
#include <list>
#include <array>
#include <vector>
#include <deque>
#include <unordered_set>
#include <unordered_map>
#include <stack>
#include <queue>
#include <random>
#include <type_traits>
#include <utility>

struct Hobbit {
    std::string name;
    int hp, off, def;
    
    friend bool operator == (const Hobbit&, const Hobbit&) = default;
};

std::ostream& operator << (std::ostream& out, const Hobbit& h) {
    return out
    << "Hobbit{\"" << h.name << "\", "
    << ".hp=" << h.hp << ", "
    << ".off=" << h.off << ", "
    << ".def=" << h.def << "}";
}

template < typename T >
std::ostream& operator << (std::ostream& out, const std::optional<T>& x) {
    if (!x) return out << "EMPTY_OPTIONAL";
    return out << "Optional{" << *x << "}";
}

#endif

struct HobbitArmy 
{
    static constexpr bool CHECK_NEGATIVE_HP = true;
private:

    struct Node 
    {
        struct SubtreeInfo
        {
            int hpDiff_ = 0, offDiff_ = 0, defDiff_ = 0;
            int minHpSubtree_;
        };
        
        Hobbit value_;
        Node*  parent_ = nullptr, *left_ = nullptr, *right_ = nullptr;
        int8_t balance_ = 0;
        SubtreeInfo subtreeInfo_;

        Node(Hobbit value, Node* parent = nullptr)
            : value_(value), parent_(parent), subtreeInfo_{.minHpSubtree_ = value_.hp} {}

        inline void add_lazy(int hpDiff, int offDiff, int defDiff)
        {
            subtreeInfo_.hpDiff_ += hpDiff;
            subtreeInfo_.offDiff_ += offDiff;
            subtreeInfo_.defDiff_ += defDiff;
        }

        inline bool has_lazy() const
        {
            return subtreeInfo_.hpDiff_ || subtreeInfo_.offDiff_ || subtreeInfo_.defDiff_;
        }

        inline void clear_lazy()
        {
            subtreeInfo_.hpDiff_ = 0;
            subtreeInfo_.offDiff_ = 0;
            subtreeInfo_.defDiff_ = 0;
        } 
    };

    size_t size_ = 0;
    Node*  root_ = nullptr;

public:

    HobbitArmy() = default;
    HobbitArmy(const HobbitArmy&)            = delete;
    HobbitArmy& operator=(const HobbitArmy&) = delete;
    HobbitArmy(HobbitArmy&&)                 = delete;
    HobbitArmy& operator=(HobbitArmy&&)      = delete;

    ~HobbitArmy()
    {
        auto free = [&](auto& self, Node* n) -> void
        {
            if (!n)
                return;
            self(self, n->left_);
            self(self, n->right_);
            delete(n);
        };
        free(free, root_);
        root_ = nullptr;
        size_ = 0;
    };

    bool add(const Hobbit& hobbit)
    {
        if (hobbit.hp < 1)
            return false;

        if (!root_)
        {
            root_ = new Node(hobbit);
            size_++;
            return true;
        }

        Node* curr    = root_;
        Node* parent  = nullptr;
        bool leftMove = false;;
        while (curr)
        {
            _propagate_lazy(curr);
            parent = curr;
            if (hobbit.name < curr->value_.name)
            {
                curr = curr->left_; 
                leftMove = true;
            }
            else if (hobbit.name > curr->value_.name)
            {
                curr = curr->right_; 
                leftMove = false;
            }
            else // names are equal
                return false;
        }

        #ifdef _DEBUG
        assert(parent);
        #endif

        Node* newNode = new Node(hobbit, parent);
        
        leftMove ? parent->left_ = newNode : parent->right_ = newNode;
        
        size_++;
        _propagate_insert(newNode);

        _recompute_subtree_info<true>(newNode);
        return true;
    }

    std::optional<Hobbit> erase(const std::string& hobbit_name)
    {
        Node* curr = root_;
        while (curr)
        {
            _propagate_lazy(curr);
            if (hobbit_name < curr->value_.name)
                curr = curr->left_;
            else if (hobbit_name > curr->value_.name)    
                curr = curr->right_;
            else
                break;
        }

        if (!curr) // not found
            return std::nullopt;

        std::optional<Hobbit> target(curr->value_);

        // 0 or 1 son
        if (!curr->left_ || !curr->right_) 
        {
            Node* child = curr->left_ ? curr->left_ : curr->right_;
            Node* parent = curr->parent_;
            bool removedLeft = parent && (parent->left_ == curr);

            _switcharoo(curr, child);
            delete curr;
            size_--;

            if (parent)
            {
                _propagate_erase(parent, removedLeft);
                _recompute_subtree_info<true>(parent);
            }
            return target;
        }

        // find min in right son and switch with curr
        Node* succ = curr->right_;
        _propagate_lazy(succ);
        while (succ->left_)
        {
            succ = succ->left_;
            _propagate_lazy(succ);
        }

        curr->value_ = succ->value_;

        Node* succParent = succ->parent_;
        bool removedLeft = (succParent->left_ == succ);
        Node* succChild  = succ->right_;

        _switcharoo(succ, succChild);
        delete succ;
        size_--;

        if (succParent)
        {
            _propagate_erase(succParent, removedLeft);
            _recompute_subtree_info<true>(succParent);
        }
        return target;
    }

    std::optional<Hobbit> stats(const std::string& hobbit_name) const 
    {
        Node* curr = root_;
        while (curr)
        {
            if (hobbit_name < curr->value_.name)
                curr = curr->left_;
            else if (hobbit_name > curr->value_.name)
                curr = curr->right_;
            else 
            {
                _propagate_lazy(curr);
                return curr->value_;
            }
        }
        if (!curr)
            return std::nullopt;

        return curr->value_;
    }

    bool enchant(const std::string& first,
                 const std::string& last,
                 int hp_diff, int off_diff, int def_diff)
    {
        if (first > last)
            return true;

        if constexpr (CHECK_NEGATIVE_HP)
        {
            auto lCheck = [&](auto& self, Node* n, 
                              const std::string* leftKey, 
                              const std::string* rightKey) -> bool
            {
                if (!n)
                    return true;

                _propagate_lazy(n);

                // whole subtree in range
                if (leftKey && rightKey && first <= *leftKey && *rightKey <= last)
                    return n->subtreeInfo_.minHpSubtree_ + hp_diff > 0;

                const std::string& name = n->value_.name;

                // whole subtree is left to interval
                if (name < first)
                    return self(self, n->right_, &name, rightKey);

                // whole subtree is right to interval
                if (name > last)
                    return self(self, n->left_, leftKey, &name);

                // name is in range
                
                // check left
                if (!self(self, n->left_, leftKey, &name))
                    return false;

                // check this node
                if (n->value_.hp + hp_diff <= 0)
                    return false;

                // check rigth
                if (!self(self, n->right_, &name, rightKey))
                    return false;

                return true;
            };

            if (!lCheck(lCheck, root_, nullptr, nullptr))
                return false;
        }

        auto lApply = [&](auto& self, Node* n,
                          const std::string* leftKey,
                          const std::string* rightKey) -> void
        {
            if (!n)
                return;

            _propagate_lazy(n);

            // whole subtree in range
            if (leftKey && rightKey && first <= *leftKey && *rightKey <= last)
            {
                _apply_lazy_to_subtree(n, hp_diff, off_diff, def_diff);
                return;
            }

            const std::string& name = n->value_.name;

            // whole subtree is left to interval
            if (name < first)
            {
                self(self, n->right_, &name, rightKey);
                _recompute_subtree_info<false>(n);
                return;
            }

            // whole subtree is right to interval
            if (name > last)
            {
                self(self, n->left_, leftKey, &name);
                _recompute_subtree_info<false>(n);
                return;
            }

            // name is in range
            
            // apply left
            self(self, n->left_, leftKey, &name);
            // this
            _add_stats(n->value_, hp_diff, off_diff, def_diff);
            // apply right
            self(self, n->right_, &name, rightKey);
            _recompute_subtree_info<false>(n);
        };

        lApply(lApply, root_, nullptr, nullptr);


        return true;
    }

    void for_each(auto&& fun) const
    {
        for_each_impl(root_, fun);
    }

private:
    
    static void for_each_impl(Node* node, auto& fun) 
    {
        if (!node) 
            return;

        if (node->has_lazy())
            _propagate_lazy(node);

        for_each_impl(node->left_, fun);
        fun(node->value_);
        for_each_impl(node->right_, fun);
    }

    // marks whole subtree by given diffs
    static void _apply_lazy_to_subtree(Node* n, int hpDiff, int offDiff, int defDiff) 
    {
        if (!n) 
            return;
        n->add_lazy(hpDiff, offDiff, defDiff);
        n->subtreeInfo_.minHpSubtree_ += hpDiff;
    }

    // n has non-zero lazy diffs ? -> apply to n, add lazy to children and clears n's lazy
    static void _propagate_lazy(Node* n)
    {
        if (!n || !n->has_lazy()) 
            return;
        
        int hpDiff  = n->subtreeInfo_.hpDiff_;
        int offDiff = n->subtreeInfo_.offDiff_;
        int defDiff = n->subtreeInfo_.defDiff_;

        _add_stats(n->value_, hpDiff, offDiff, defDiff);

        if (n->left_)
            n->left_->add_lazy(hpDiff, offDiff, defDiff);
        if (n->right_)
            n->right_->add_lazy(hpDiff, offDiff, defDiff);

        n->clear_lazy();
    }

    static void _propagate_all(Node* n)
    {
        if (!n)
            return;

        _propagate_lazy(n);
        _propagate_all(n->left_);
        _propagate_all(n->right_);
    }

    // recompute minHpSubtree (can propagate up)
    template<bool PROPAGATE>
    static void _recompute_subtree_info(Node* n) 
    {
        while (n)
        {
            n->subtreeInfo_.minHpSubtree_ = n->value_.hp;

            if (n->left_) 
            {
                int leftMin = n->left_->subtreeInfo_.minHpSubtree_ + n->left_->subtreeInfo_.hpDiff_;
                if (leftMin < n->subtreeInfo_.minHpSubtree_)
                    n->subtreeInfo_.minHpSubtree_ = leftMin;
            }

            if (n->right_) 
            {
                int rightMin = n->right_->subtreeInfo_.minHpSubtree_ + n->right_->subtreeInfo_.hpDiff_;
                if (rightMin < n->subtreeInfo_.minHpSubtree_)
                    n->subtreeInfo_.minHpSubtree_ = rightMin;
            }

            if constexpr (PROPAGATE)
                n = n->parent_;
            else
                break;
        }
    }

    inline static void _add_stats(Hobbit& h, int hpDiff, int offDiff, int defDiff) 
    {
        h.hp  += hpDiff;
        h.off += offDiff;
        h.def += defDiff;
    }

    // replace node a in its parent (or root) with node b and updates parent links
    void _switcharoo(Node* a, Node* b)
    {
        #ifdef _DEBUG
        assert(a);
        #endif

        _propagate_lazy(a);
        if (b) 
            _propagate_lazy(b);

        Node* parent = a->parent_;

        if (!parent) // root
        {
            root_ = b;
            if (b) 
                b->parent_ = nullptr;
            return;
        }

        parent->left_ == a ? parent->left_ = b : parent->right_ = b;
        
        if (b) 
            b->parent_ = parent;
    }

    void _propagate_insert(Node* n)
    {
        #ifdef _DEBUG
        assert(n);
        #endif

        Node* parent = n->parent_;
        while (parent)
        {
            int8_t& parentBalance = parent->balance_;
            if (n == parent->left_)
            {
                parentBalance--;
                if (parentBalance == 0)
                    return;
                else if (parentBalance == -1)
                {
                    n = parent;
                    parent = parent->parent_;
                    continue;
                }
                else // parentBalance == -2
                {
                    Node* parentLeft = parent->left_;
                    _rotate(parent, parentLeft);
                    break;
                }
            }
            else
            {
                parentBalance++;
                if (parentBalance == 0)
                    return;
                else if (parentBalance == 1)
                {
                    n = parent;
                    parent = parent->parent_;
                    continue;
                }
                else // parentBalance == 2
                {
                    Node* parentRight = parent->right_;
                    _rotate(parent, parentRight);
                    break;
                }
            }
        }
    }

    void _propagate_erase(Node* parent, bool removedLeft)
    {
        while (parent)
        {
            int8_t& parentBalance = parent->balance_;
            if (removedLeft)
            {
                parentBalance++;
                if (parentBalance == 1)
                    return;
                else if (parentBalance == 2)
                {
                    Node* parentRight = parent->right_;
                    if (!parentRight)
                        return;
                    int8_t parentRightBalance = parentRight->balance_;
                    bool sonZero = (parentRightBalance == 0);
                    Node* parentParent = parent->parent_;
                    removedLeft = (parentParent && parentParent->left_ == parent);
                    _rotate(parent, parentRight);
                    if (sonZero)
                        return;
                    parent = parentParent;
                }
                else // parentBalance == 0
                {
                    Node* parentParent = parent->parent_;
                    removedLeft = (parentParent && parentParent->left_ == parent);
                    parent = parentParent;
                }
            }
            else // !removedLeft
            {
                parentBalance--;
                if (parentBalance == -1)
                    return;
                else if (parentBalance == -2)
                {
                    Node* parentLeft = parent->left_;
                    if (!parentLeft)
                        return;
                    int8_t parentLeftBalance = parentLeft->balance_;
                    bool sonZero = (parentLeftBalance == 0);
                    Node* parentParent = parent->parent_;
                    removedLeft = (parentParent && parentParent->left_ == parent);
                    _rotate(parent, parentLeft);
                    if (sonZero)
                        return;
                    parent = parentParent;
                }
                else // parentBalance == 0
                {
                    Node* parentParent = parent->parent_;
                    removedLeft = (parentParent && parentParent->left_ == parent);
                    parent = parentParent;
                }
            }

        }
    }

    void _rotate(Node* parent, Node* son)
    {
        #ifdef _DEBUG
        assert(parent);
        assert(son);
        #endif

        _propagate_lazy(parent);
        _propagate_lazy(son);

        // single rotations
        if (_same_signs(parent->balance_, son->balance_)) 
        {
            if (parent->balance_ >= 0) // both positive(right overweight) -> single left rotation on parent
            {
                _l_rotate(parent);
                if (son->balance_ == 0) // only possible during deletion
                { 
                    parent->balance_ = +1;
                    son->balance_    = -1;
                } 
                else 
                {
                    parent->balance_ = 0;
                    son->balance_    = 0;
                }
            } 
            else // both negative(left overweight) -> single right rotation on parent
            {
                _r_rotate(parent);
                if (son->balance_ == 0) // only possible during deletion
                {
                    parent->balance_ = -1;
                    son->balance_    = +1;
                } 
                else 
                {
                    parent->balance_ = 0;
                    son->balance_    = 0;
                }
            }
            _recompute_subtree_info<false>(parent);
            _recompute_subtree_info<false>(son);
            return;
        }

        // double rotations
        if (parent->balance_ >= 0) // R-L rotation
        {
            Node* sonLeft = son->left_;
            int8_t sonLeftBalance = sonLeft ? sonLeft->balance_ : 0;

            _r_rotate(son);
            _l_rotate(parent);

            if (sonLeftBalance == 1) 
            { 
                parent->balance_ = -1; 
                son->balance_ = 0;
            }
            else if (sonLeftBalance == 0) 
            { 
                parent->balance_ = 0; 
                son->balance_ = 0; 
            }
            else // sonLeftBalance == -1 
            { 
                parent->balance_ = 0; 
                son->balance_ = 1; 
            }
            
            if (sonLeft) 
                sonLeft->balance_ = 0;
            
            _recompute_subtree_info<false>(parent);
            _recompute_subtree_info<false>(son);
            if (sonLeft)
                _recompute_subtree_info<false>(sonLeft);
        } 
        else // L-R rotation
        {
            Node* sonRight = son->right_;
            int8_t sonRightBalance = sonRight ? sonRight->balance_ : 0;

            _l_rotate(son);
            _r_rotate(parent);

            if (sonRightBalance == -1) 
            { 
                parent->balance_ = +1; 
                son->balance_ = 0; 
            }
            else if (sonRightBalance == 0) 
            { 
                parent->balance_ = 0; 
                son->balance_ = 0; 
            }
            else // sonRightBalance == +1
            { 
                parent->balance_ = 0; 
                son->balance_ = -1; 
            }
            
            if (sonRight) 
                sonRight->balance_ = 0;

            _recompute_subtree_info<false>(parent);
            _recompute_subtree_info<false>(son);
            if (sonRight)
                _recompute_subtree_info<false>(sonRight);
        }
    }

    void _l_rotate(Node* n)
    {
        #ifdef _DEBUG
        assert(n && n->right_);
        #endif

        Node* rightSon = n->right_;
        Node* rightLeftSon = n->right_->left_;

        _switcharoo(n, rightSon);

        rightSon->left_ = n;
        n->parent_ = rightSon;

        n->right_ = rightLeftSon;
        if (rightLeftSon)
            rightLeftSon->parent_ = n;
    }

    void _r_rotate(Node* n)
    {
        #ifdef _DEBUG
        assert(n && n->left_);
        #endif
        
        Node* leftSon      = n->left_;
        Node* leftRightSon = n->left_->right_;

        _switcharoo(n, leftSon);

        leftSon->right_ = n;
        n->parent_ = leftSon;

        n->left_ = leftRightSon;
        if (leftRightSon)
            leftRightSon->parent_ = n;
    }

    static bool _same_signs(int8_t a, int8_t b)
    {
        if (b == 0)
            return true;
        return (a >= 0) == (b >= 0);
    }
};

#ifndef __PROGTEST__

////////////////// Dark magic, ignore ////////////////////////

template < typename T >
auto quote(const T& t) { return t; }

std::string quote(const std::string& s) {
    std::string ret = "\"";
    for (char c : s) if (c != '\n') ret += c; else ret += "\\n";
    return ret + "\"";
}

#define STR_(a) #a
#define STR(a) STR_(a)

#define CHECK_(a, b, a_str, b_str) do { \
    auto _a = (a); \
    decltype(a) _b = (b); \
    if (_a != _b) { \
        std::cout << "Line " << __LINE__ << ": Assertion " \
        << a_str << " == " << b_str << " failed!" \
        << " (lhs: " << quote(_a) << ")" << std::endl; \
        fail++; \
    } else ok++; \
} while (0)

#define CHECK(a, b) CHECK_(a, b, #a, #b)

////////////////// End of dark magic ////////////////////////

void check_army(const HobbitArmy& A, const std::vector<Hobbit>& ref, int& ok, int& fail) {
    size_t i = 0;
    
    A.for_each([&](const Hobbit& h) {
        CHECK(i < ref.size(), true);
        CHECK(h, ref[i]);
        i++;
    });
    
    CHECK(i, ref.size());
}

void test1(int& ok, int& fail) {
    HobbitArmy A;
    check_army(A, {}, ok, fail);
    
    CHECK(A.add({"Frodo", 100, 10, 3}), true);
    CHECK(A.add({"Frodo", 200, 10, 3}), false);
    CHECK(A.erase("Frodo"), std::optional(Hobbit("Frodo", 100, 10, 3)));
    CHECK(A.add({"Frodo", 200, 10, 3}), true);
    
    CHECK(A.add({"Sam", 80, 10, 4}), true);
    CHECK(A.add({"Pippin", 60, 12, 2}), true);
    CHECK(A.add({"Merry", 60, 15, -3}), true);
    CHECK(A.add({"Smeagol", 0, 100, 100}), false);
    
    if constexpr(HobbitArmy::CHECK_NEGATIVE_HP)
    CHECK(A.add({"Smeagol", -100, 100, 100}), false);
    
    CHECK(A.add({"Smeagol", 200, 100, 100}), true);
    
    CHECK(A.enchant("Frodo", "Frodo", 10, 1, 1), true);
    CHECK(A.enchant("Sam", "Frodo", -1000, 1, 1), true); // empty range
    CHECK(A.enchant("Bilbo", "Bungo", 1000, 0, 0), true); // empty range
    
    if constexpr(HobbitArmy::CHECK_NEGATIVE_HP)
    CHECK(A.enchant("Frodo", "Sam", -60, 1, 1), false);
    
    CHECK(A.enchant("Frodo", "Sam", 1, 0, 0), true);
    CHECK(A.enchant("Frodo", "Sam", -60, 1, 1), true);
    
    CHECK(A.stats("Gandalf"), std::optional<Hobbit>{});
    CHECK(A.stats("Frodo"), std::optional(Hobbit("Frodo", 151, 12, 5)));
    CHECK(A.stats("Merry"), std::optional(Hobbit("Merry", 1, 16, -2)));
    
    check_army(A, {
        {"Frodo", 151, 12, 5},
        {"Merry", 1, 16, -2},
        {"Pippin", 1, 13, 3},
        {"Sam", 21, 11, 5},
        {"Smeagol", 200, 100, 100},
    }, ok, fail);
}

int main() {
    int ok = 0, fail = 0;
    test1(ok, fail);
    
    if (!fail) std::cout << "Passed all " << ok << " tests!" << std::endl;
    else std::cout << "Failed " << fail << " of " << (ok + fail) << " tests." << std::endl;
}

#endif
