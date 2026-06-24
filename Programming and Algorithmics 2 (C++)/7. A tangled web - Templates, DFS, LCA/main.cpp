#ifndef __PROGTEST__
#include <cstdlib>
#include <cmath>
#include <cassert>
#include <iostream>
#include <iomanip>
#include <vector>
#include <list>
#include <set>
#include <map>
#include <array>
#include <deque>
#include <queue>
#include <stack>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <compare>
#include <algorithm>
#include <memory>
#include <iterator>
#include <functional>
#include <stdexcept>
#include <optional>
using namespace std::literals;

class CDumbString
{
public:
    CDumbString             ( std::string v )
    : m_Data ( std::move ( v ) )
    {
    }
    bool        operator == ( const CDumbString & rhs ) const = default;
    private:
    std::string m_Data;
};
#endif /* __PROGTEST__ */

template <typename T_>
class CNet
{
private:
    
    struct Node
    {
        T_ key;
        std::vector<std::pair<int,int>> adj;
        int depth = 0;
        long long dist = 0;
        int comp = -1;
        std::vector<int> parent;

        Node(const T_ & k)
        : key(k),
          adj(),
          depth(0),
          dist(0),
          comp(-1),
          parent() {}

    };

    std::vector<Node> data_;
    
public:
    
    // default constructor
    CNet() = default;
    
    // add ( x, y, cost )
    CNet& add(const T_& x, const T_& y, unsigned cost)
    {
        int ix = AddNode(x);
        int iy = AddNode(y);
        data_[ix].adj.emplace_back(iy, cost);
        data_[iy].adj.emplace_back(ix, cost);
        return *this;
    }
    // optimize ()
    CNet& optimize()
    {
        int N = static_cast<int>(data_.size());
        int LOG = 1;

        while ((1 << LOG) <= N) 
            LOG++;
        
        for (auto& node : data_)
            node.parent.assign(LOG, -1);

        int cid = 0;
        for (int i = 0; i < N; i++)
            if (data_[i].comp < 0)
                DFS(i, -1 , 0, 0, cid++);
        
        for (int k = 1; k < LOG; k++)
            for (int v = 0; v < N; v++)
            {
                int p = data_[v].parent[k - 1];
                if (p >= 0)
                    data_[v].parent[k] = data_[p].parent[k - 1];
            }
        return *this;
    }
    // totalCost ( x, y )
    long long totalCost(T_ x, T_ y)
    {
        int ix = FindNode(x), iy = FindNode(y);
        if (ix < 0 || iy < 0)
            return -1;
        if (ix == iy)
            return 0;
        if (data_[ix].comp !=data_[iy].comp)
            return -1;
        int a = LCA(ix, iy);
        return data_[ix].dist + data_[iy].dist - 2ULL * data_[a].dist;
    }

private:

    int AddNode(const T_& key)
    {
        int idx = FindNode(key);
        if (idx != -1) return idx;
        
        data_.push_back(Node{key});
        return static_cast<int>(data_.size() - 1);
    }

    int FindNode(const T_& key)
    {
        for (size_t i = 0; i < data_.size(); i++)
            if (data_[i].key == key)
                return static_cast<int>(i);
        return -1;
    }

    void DFS(int v, int p, int depth, long long dist, int cid)
    {
        auto& node = data_[v];
        node.comp = cid;
        node.parent[0] = p;
        node.depth = depth;
        node.dist = dist;

        for (auto [to, w] : node.adj)
        {
            if (to == p)
                continue;;
            DFS(to, v, depth + 1, dist + w, cid);
        }
    }

    int LCA(int a, int b) const
    {
        if (data_[a].depth < data_[b].depth)
            std::swap(a, b);
        int LOG = static_cast<int>(data_[a].parent.size());

        for (int k = LOG - 1; k >= 0; k--)
        {
            int pa = data_[a].parent[k];
            
            if (pa >= 0 && data_[pa].depth >= data_[b].depth)
                a = pa;
        }
        
        if (a == b)
            return a;

        for (int k = LOG - 1; k >= 0; k--)
        {
            if (data_[a].parent[k] != data_[b].parent[k])
            {
                a = data_[a].parent[k];
                b = data_[b].parent[k];
            }
        }
        return data_[a].parent[0];
    }

};

#ifndef __PROGTEST__
int main ()
{
    CNet<std::string> a;
    a . add ( "Adam", "Bob", 100 )
    . add ( "Bob", "Carol", 200 )
    . add ( "Dave", "Adam", 300 )
    . add ( "Eve", "Fiona", 120 )
    . add ( "Kate", "Larry", 270 )
    . add ( "Ivan", "John", 70 )
    . add ( "Kate", "Ivan", 300 )
    . add ( "George", "Henry", 10 )
    . add ( "Eve", "George", 42 )
    . add ( "Adam", "Eve", 75 )
    . add ( "Ivan", "George", 38 )
    . optimize ();
    assert ( a . totalCost ( "Adam", "Bob" ) == 100 );
    assert ( a . totalCost ( "John", "Eve" ) == 150 );
    assert ( a . totalCost ( "Dave", "Henry" ) == 427 );
    assert ( a . totalCost ( "Carol", "Larry" ) == 1025 );
    assert ( a . totalCost ( "George", "George" ) == 0 );
    assert ( a . totalCost ( "Alice", "Bob" ) == -1 );
    assert ( a . totalCost ( "Thomas", "Thomas" ) == -1 );
    
    CNet<int> b;
    b . add ( 0, 1, 100 )
    . add ( 1, 2, 200 )
    . add ( 3, 0, 300 )
    . add ( 4, 5, 120 )
    . add ( 10, 11, 270 )
    . add ( 8, 9, 70 )
    . add ( 10, 8, 300 )
    . add ( 6, 7, 10 )
    . add ( 4, 6, 42 )
    . add ( 0, 4, 75 )
    . add ( 8, 6, 38 )
    . optimize ();
    assert ( b . totalCost ( 0, 1 ) == 100 );
    assert ( b . totalCost ( 9, 4 ) == 150 );
    assert ( b . totalCost ( 3, 7 ) == 427 );
    assert ( b . totalCost ( 2, 11 ) == 1025 );
    assert ( b . totalCost ( 6, 6 ) == 0 );
    assert ( b . totalCost ( 0, 1 ) == 100 );
    assert ( b . totalCost ( 19, 19 ) == -1 );
    
    CNet<CDumbString> c;
    c . add ( "Adam"s, "Bob"s, 100 )
    . add ( "Bob"s, "Carol"s, 200 )
    . add ( "Dave"s, "Adam"s, 300 )
    . add ( "Eve"s, "Fiona"s, 120 )
    . add ( "Kate"s, "Larry"s, 270 )
    . add ( "Ivan"s, "John"s, 70 )
    . add ( "Kate"s, "Ivan"s, 300 )
    . add ( "George"s, "Henry"s, 10 )
    . add ( "Eve"s, "George"s, 42 )
    . add ( "Adam"s, "Eve"s, 75 )
    . add ( "Ivan"s, "George"s, 38 )
    . optimize ();
    assert ( c . totalCost ( "Adam"s, "Bob"s ) == 100 );
    assert ( c . totalCost ( "John"s, "Eve"s ) == 150 );
    assert ( c . totalCost ( "Dave"s, "Henry"s ) == 427 );
    assert ( c . totalCost ( "Carol"s, "Larry"s ) == 1025 );
    assert ( c . totalCost ( "George"s, "George"s ) == 0 );
    assert ( c . totalCost ( "Alice"s, "Bob"s ) == -1 );
    assert ( c . totalCost ( "Thomas"s, "Thomas"s ) == -1 );
    
    return EXIT_SUCCESS;
}
#endif /* __PROGTEST__ */
