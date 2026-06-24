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
#include <set>
#include <map>
#include <stack>
#include <queue>
#include <random>
#include <type_traits>
#include <compare>
#include <ranges>
#include <optional>
#include <variant>

struct Item 
{
    enum Type : uint8_t 
    {
        Weapon = 0,
        Armor = 1,
        RubberDuck = 2,
        TYPE_COUNT = 3,
    };
    
    std::string name;
    Type type;
    int hp = 0, off = 0, def = 0;
    int stacking_off = 0, stacking_def = 0;
    bool first_attack = false; // Hero attacks first.
    bool stealth = false; // Hero can sneak past monsters (but cannot loot items while sneaking).
    
    friend auto operator <=> (const Item&, const Item&) = default;
};

struct Monster 
{
    int hp = 0, off = 0, def = 0;
    int stacking_off = 0, stacking_def = 0;
};

using RoomId = size_t;
using ItemId = size_t;

struct Room 
{
    std::vector<RoomId> neighbors;
    std::optional<Monster> monster;
    std::vector<Item> items;
};

struct Move { RoomId room; };
struct Pickup { ItemId item; };
struct Drop { Item::Type type; };
using Action = std::variant<Move, Pickup, Drop>;

namespace student_namespace 
{
    #endif
    
    #define internal      static
    #define local_persist static
    #define global_var    static

    global_var constexpr int HERO_BASE_HP  = 10000;
    global_var constexpr int HERO_BASE_OFF =     3;
    global_var constexpr int HERO_BASE_DEF =     2;

    using Hero = Monster;

    std::optional<int> turns_to_kill(int hp, int dmg, int stacking_dmg) {
        assert(hp > 0);
        
        if (stacking_dmg == 0) {
            if (dmg <= 0) return {};
            return (hp + dmg - 1) / dmg;
        }
        
        int i = 0;
        for (; hp > 0; i++) {
            if (dmg <= 0 && stacking_dmg < 0) return {};
            hp -= std::max(dmg, 0);
            dmg += stacking_dmg;
        }
        
        return i;
    }
    
    enum CombatResult {
        A_WINS, B_WINS, TIE
    };
    
    // Monster `a` attacks first
    CombatResult simulate_combat(Monster a, Monster b) {
        a.def += a.stacking_def;
        
        auto a_turns = turns_to_kill(b.hp, a.off - b.def, a.stacking_off - b.stacking_def);
        auto b_turns = turns_to_kill(a.hp, b.off - a.def, b.stacking_off - a.stacking_def);
        
        if (!a_turns && !b_turns) return TIE;
        if (!a_turns) return B_WINS;
        if (!b_turns) return A_WINS;
        return *a_turns <= *b_turns ? A_WINS : B_WINS;
    }

    struct Equipment
    {     
        static constexpr int NOT_EQUIPED = -1; 
        
        int w = NOT_EQUIPED, a = NOT_EQUIPED, d = NOT_EQUIPED;

        int operator[](Item::Type t) const
        {
            switch (t)
            {
                case Item::Type::Weapon:     return w;
                case Item::Type::Armor:      return a;
                case Item::Type::RubberDuck: return d;
                default:                     throw std::runtime_error("Invalid Item Type");
            }
        }

        int& operator[](Item::Type t)
        {
            switch (t)
            {
                case Item::Type::Weapon:     return w;
                case Item::Type::Armor:      return a;
                case Item::Type::RubberDuck: return d;
                default:                     throw std::runtime_error("Invalid Item Type");
            }
        }

        bool HasFirstAttack(const std::array<std::vector<Item>, Item::Type::TYPE_COUNT>& items) const noexcept
        {
            for (Item::Type t : {Item::Type::Weapon, Item::Type::Armor, Item::Type::RubberDuck})
            {
                int id = (*this)[t];
                if (id == Equipment::NOT_EQUIPED)
                    continue;
                const Item& it = items[t][id]; 
                if (it.first_attack)
                    return true;
            }
            return false;
        }

        bool HasStealth(const std::array<std::vector<Item>, Item::Type::TYPE_COUNT>& items) const noexcept
        {
            for (Item::Type t : {Item::Type::Weapon, Item::Type::Armor, Item::Type::RubberDuck})
            {
                int id = (*this)[t];
                if (id == Equipment::NOT_EQUIPED)
                    continue;
                const Item& it = items[t][id]; 
                if (it.stealth)
                    return true;
            }
            return false;
        }
         
        friend auto operator<=>(const Equipment&, const Equipment&) = default;

        #if defined(_DEBUG) && defined(_dq)
        friend std::ostream& operator<<(std::ostream& os, const Equipment& e) 
        {
            os << "{W=" << e.w << ",A=" << e.a << ",D=" << e.d << "}";
            return os;
        }
        #endif
    };

    internal Hero MakeHero(const std::array<std::vector<Item>, Item::Type::TYPE_COUNT>& items, const Equipment& e)
    {
        Hero hero = {HERO_BASE_HP, HERO_BASE_OFF, HERO_BASE_DEF, 0, 0};

        for (Item::Type t : {Item::Type::Weapon, Item::Type::Armor, Item::Type::RubberDuck})
        {
            int id = e[t];
            if (id == Equipment::NOT_EQUIPED)
                continue;
            const Item& it = items[t][id]; 
            
            hero.hp += it.hp; hero.off += it.off; hero.def += it.def; hero.stacking_off += it.stacking_off; hero.stacking_def += it.stacking_def;
        }

        hero.hp = std::max(hero.hp, 1);
        return hero;
    }

    internal bool HeroWins(const std::array<std::vector<Item>, Item::Type::TYPE_COUNT>& items, 
                           const Monster& monster,
                           const Equipment& eq)
    {
        const Hero hero = MakeHero(items, eq);
        return eq.HasFirstAttack(items) ? (simulate_combat(hero, monster) == A_WINS) 
                                        : (simulate_combat(monster, hero) == B_WINS);
    }

    struct State 
    { 
        RoomId roomId; Equipment eq; bool loot; bool treasure;

        friend auto operator<=>(const State&, const State&) = default;

        #if defined(_DEBUG) && defined(_dq)
        friend std::ostream& operator<<(std::ostream& os, const State& s)
        {
            os << "(room=" << s.roomId
               << ", eq=" << s.eq
               << ", loot=" << (s.loot ? "T" : "F")
               << ", treasure=" << (s.treasure ? "T" : "F") << ")";
            return os;
        }
        #endif
    };

    struct Transition { State previousState; Action action; };

    bool ItemComp(const Item& a, const Item& b) /* comparing items without comparing names */
    {
        return std::tie(a.type, a.hp, a.off, a.def, a.stacking_off, a.stacking_def, a.first_attack, a.stealth)
               == std::tie(b.type, b.hp, b.off, b.def, b.stacking_off, b.stacking_def, b.first_attack, b.stealth);
    }

    struct ItemDB
    {
        std::array<std::vector<Item>, Item::TYPE_COUNT> data;

        ItemDB() = delete;
        ItemDB(const std::vector<Room>& rooms) /* preprocess : merge duplicate items across the world (all rooms) */
        {
            for (const auto& r : rooms) 
                for (const auto& it : r.items) 
                {
                    auto& vec = data[it.type];
                    
                    if (std::none_of(vec.begin(), vec.end(), [&](const Item& x) { return ItemComp(x, it); }))
                        vec.push_back(it);
                }
        }

        int GetIndex(const Item& item) const
        {
            const auto& vec = data[item.type];
            for (int i = 0; i < (int)vec.size(); i++)
                if (ItemComp(vec[i], item)) 
                    return i;
            throw std::runtime_error("Came across an item we've not processed");
        }
    };

    internal std::vector<Action> Reconstruct(const std::map<State, Transition>& parent, State& s)
    {
        std::vector<Action> actions;
        
        for (auto it = parent.find(s); it != parent.end(); it = parent.find(s))
        {
            const Transition& t = parent.at(s);
            actions.push_back(t.action);
            s = t.previousState;
        }
        
        /* add first move into entrance */
        actions.emplace_back(Move{s.roomId});
        std::reverse(actions.begin(), actions.end());
        return actions;
    }
    
    #if defined(_DEBUG) && defined(_dq)
    internal void DEBUG_DEQUE_PRINT(const std::deque<State>& dq) 
    {
        std::cout << "Deque[" << dq.size() << "]:\n";
        size_t i = 0;
        for (const auto& s : dq) 
            std::cout << "  [" << i++ << "] " << s << "\n";

        std::cin.get();
    }
    #endif

    /* 0-1 BFS */
    std::vector<Action> find_shortest_path(const std::vector<Room>&   rooms, 
                                           const std::vector<RoomId>& entrances,
                                           RoomId                     treasure)
    {
        ItemDB itemDB(rooms);

        auto lIsEntrance = [&](RoomId r) -> bool 
        { 
            return std::find(entrances.begin(), entrances.end(), r) != entrances.end();
        };
        // TODO: CHECK THIS LATER
        // droping weapons that have no negative effects doesn't make sense (or we'll find out hehe)
        auto lHasNegativeStat = [&](Item::Type t, int id) -> bool 
        {
            const Item& it = itemDB.data[t][id];
            return it.hp < 0 || it.off < 0 || it.def < 0 || it.stacking_off < 0 || it.stacking_def < 0;
        };
        // true if a is better in all aspects compared to b
        auto lAbsBetterItem = [&](Item::Type t, int a, int b) -> bool
        {
            
            const Item& ai = a != -1 ? itemDB.data[t][a] : Item{};
            const Item& bi = b != -1 ? itemDB.data[t][b] : Item{};

            return ai.hp >= bi.hp && ai.off >= bi.off && ai.def >= bi.def
                && ai.stacking_off >= bi.stacking_off && ai.stacking_def >= bi.stacking_def
                && (ai.first_attack || !bi.first_attack) && (ai.stealth || !bi.stealth);
        };
                
        std::deque<State> dq;
        std::map<State, Transition> transitions; // tracks from what State and using which Action got into State
        std::map<State, int> dist; // tracks shortest distance to State

        /* enqueue neighbors into deque, 0-cost(pickup, drop) to front, 1-cost(move) to end */
        auto lEnqueue = [&](const State& from, const State& to, const Action& action, int cost)
        {
            #ifdef _DEBUG
            assert(cost == 0 || cost == 1);
            #endif 
            
            if (!dist.contains(to))
            {
                dist[to] = dist.at(from) + cost;
                transitions[to] = Transition{from, action};
                cost == 0 ? /* cost == 0 */ dq.push_front(to) 
                          : /* cost == 1 */ dq.push_back(to);
            }
        };
        
        /* Init BFS with entrances*/
        for (RoomId e : entrances)
        {
            if (rooms[e].monster && !HeroWins(itemDB.data, *rooms[e].monster, Equipment{})) /* unable to enter entrance room */
                continue;

            State s{e, Equipment{}, true, e == treasure};
            dist.emplace(s, 0);
            dq.push_front(s);
        }
        
        #if defined(_DEBUG) && defined(_dq)
        DEBUG_DEQUE_PRINT(dq);
        #endif
        
        State finalState{}; 
        bool found(false);
        
        #ifdef _DEBUG
        size_t DEBUG_LOOP_CNT = 0;
        local_persist size_t DEBUG_LOOP_CNT_TOTAL = 0;
        #endif

        while (!dq.empty()) 
        {
            #ifdef _DEBUG
            DEBUG_LOOP_CNT++;
            DEBUG_LOOP_CNT_TOTAL++;
            #endif

            State curr = dq.front(); dq.pop_front();
            
            /* end condition */
            if (curr.treasure && lIsEntrance(curr.roomId))
            { 
                finalState = curr; 
                found = true; 
                break;
            }
            
            const Room& currRoom = rooms[curr.roomId];
            
            /* 0 - cost (PICKUP) */
            if (curr.loot)
            {
                for (ItemId i = 0; i < currRoom.items.size(); i++)
                {
                    int id = itemDB.GetIndex(currRoom.items[i]);
                    Item::Type t = currRoom.items[i].type;
                    if (lAbsBetterItem(t, curr.eq[t], id)) /* check whether it's a total nonsense to swap */
                        continue;
                    
                    State nxt = curr;
                    nxt.eq[t] = id;
                    lEnqueue(curr, nxt, Pickup{i}, 0);
                }
            }
            
            /* 0 - cost (DROP) */
            for (Item::Type t : {Item::Type::Weapon, Item::Type::Armor, Item::Type::RubberDuck})
            {
                int id = curr.eq[t];
                if (id == Equipment::NOT_EQUIPED || !lHasNegativeStat(t, id)) /* skip if it's not beneficial */
                    continue;
                State nxt = curr; 
                nxt.eq[t] = -1; 
                lEnqueue(curr, nxt, Drop{t}, 0);
            }
            
            /* 1 - cost (MOVE) */
            for (RoomId nbId : currRoom.neighbors) 
            {
                const Room& neighbor = rooms[nbId];
                /* no monster or won fight = loot */
                if (!neighbor.monster || HeroWins(itemDB.data, *neighbor.monster, curr.eq))
                {
                    State nxt{nbId, curr.eq, true, curr.treasure || (nbId == treasure)};
                    lEnqueue(curr, nxt, Move{nbId}, 1);
                }
                /* lost fight -> has_stealth = no loot */
                else if (neighbor.monster.has_value() && curr.eq.HasStealth(itemDB.data)) 
                {
                    State nxt{nbId, curr.eq, false, curr.treasure};
                    lEnqueue(curr, nxt, Move{nbId}, 1);
                }
                /* die */
            }
            #if defined(_DEBUG) && defined(_dq)
            DEBUG_DEQUE_PRINT(dq);
            #endif
        }

        #ifdef _DEBUG
        std::cout << "Current loop count: " << DEBUG_LOOP_CNT << '\n'
                  << "Total loop count: "   << DEBUG_LOOP_CNT_TOTAL << '\n';
        #endif

        return found ? Reconstruct(transitions, finalState) : std::vector<Action>{};
    }

    #ifndef __PROGTEST__
}
    
bool contains(const auto& vec, const auto& x) {
    return std::ranges::find(vec, x) != vec.end();
};

#define CHECK(cond, ...) do { \
    if (!(cond)) { fprintf(stderr, __VA_ARGS__); assert(0); } \
} while (0)

void check_solution(
    const std::vector<Room>& rooms,
    const std::vector<RoomId>& entrances,
    RoomId treasure,
    size_t expected_rooms,
    bool print = false) 
{
    // TODO check if hero survives combat
    // TODO check if treasure was collected
    
    using student_namespace::find_shortest_path;
    const std::vector<Action> solution = find_shortest_path(rooms, entrances, treasure);
    
    if (expected_rooms == 0) {
        CHECK(solution.size() == 0, "No solution should exist but got some.\n");
        return;
    }
    
    CHECK(solution.size() != 0, "Expected solution but got none.\n");
    
    try {
        CHECK(contains(entrances, std::get<Move>(solution.front()).room),
        "Path must start at entrance.\n");
        CHECK(contains(entrances, std::get<Move>(solution.back()).room),
        "Path must end at entrance.\n");
    } catch (const std::bad_variant_access&) {
        CHECK(false, "Path must start and end with Move.\n");
    }
    
    std::vector<Item> equip;
    RoomId cur = std::get<Move>(solution.front()).room;
    CHECK(cur < rooms.size(), "Room index out of range.\n");
    size_t room_count = 1;
    if (print) printf("Move(%zu)", cur);
    
    auto drop_items = [&](Item::Type type) {
        std::erase_if(equip, [&](const Item& i) { return i.type == type; });
    };
    
    for (size_t i = 1; i < solution.size(); i++) {
        if (auto m = std::get_if<Move>(&solution[i])) {
            CHECK(m->room < rooms.size(), "Next room index out of range.\n");
            CHECK(contains(rooms[cur].neighbors, m->room),
            "Next room is not a neighbor of the current one.\n");
            cur = m->room;
            room_count++;
            
            if (print) printf(", Move(%zu)", cur);
        } else if (auto p = std::get_if<Pickup>(&solution[i])) {
            CHECK(p->item < rooms[cur].items.size(), "Picked up item out of range.\n");
            const Item& item = rooms[cur].items[p->item];
            drop_items(item.type);
            equip.push_back(item);
            
            if (print) printf(", Pickup(%zu, %s)", p->item, item.name.c_str());
        } else {
            auto t = std::get<Drop>(solution[i]).type;
            drop_items(t);
            
            if (print) printf(", Drop(%s)",
                t == Item::Armor ? "Armor" :
                t == Item::Weapon ? "Weapon" :
                t == Item::RubberDuck ? "Duck" :
                "ERROR");
            }
        }
        
    if (print) printf("\n");

    CHECK(room_count == expected_rooms, 
        "Expected %zu rooms but got %zu.\n", expected_rooms, room_count);
}
#undef CHECK


void combat_examples() {
    const Item defensive_duck = {
        .name = "Defensive Duck", .type = Item::RubberDuck,
        .off = -2, .def = 8,
    };
    
    const Item invincible_duck = {
        .name = "Invincible Duck", .type = Item::RubberDuck,
        .hp = -20'000, .def = 1'000,
    };
    
    const Item fast_duck = {
        .name = "Fast Duck", .type = Item::RubberDuck,
        .first_attack = true,
    };
    
    const Item offensive_duck = {
        .name = "Offensive Duck", .type = Item::RubberDuck,
        .stacking_off = 100,
    };
    
    std::vector<Room> rooms(2);
    rooms[0].neighbors.push_back(1);
    rooms[1].neighbors.push_back(0);
    
    check_solution(rooms, { 0 }, 1, 3);
    
    rooms[1].monster = Monster{ .hp = 9'999, .off = 3, .def = 2 };
    check_solution(rooms, { 0 }, 1, 3);
    
    rooms[1].monster->hp += 1;
    check_solution(rooms, { 0 }, 1, 0);
    
    rooms[1].monster = Monster{ .hp = 100'000, .off = 10 };
    check_solution(rooms, { 0 }, 1, 0);
    
    rooms[0].items = { defensive_duck };
    check_solution(rooms, { 0 }, 1, 3);
    
    rooms[0].items = { invincible_duck };
    check_solution(rooms, { 0 }, 1, 3);
    
    rooms[0].items = {};
    rooms[1].monster = Monster{ .hp=1, .off=3, .def=0, .stacking_def=100 };
    check_solution(rooms, { 0 }, 1, 0);
    
    rooms[0].items.push_back(offensive_duck);
    check_solution(rooms, { 0 }, 1, 0);
    
    rooms[0].items.push_back(fast_duck);
    check_solution(rooms, { 0 }, 1, 3);
}

void stealth_examples() {
    const Item stealth_duck = {
        .name = "Stealth Duck", .type = Item::RubberDuck,
        .stealth = true,
    };
    
    const Item sword = {
        .name = "Sword", .type = Item::Weapon,
        .off = 10,
    };
    
    const Monster m = { .hp = 10'000, .off=10, .def=2 };
    
    std::vector<Room> rooms(4);
    
    for (size_t i = 1; i < rooms.size(); i++) {
        rooms[i].neighbors.push_back(i - 1);
        rooms[i - 1].neighbors.push_back(i);
    }
    
    rooms[0].items = { stealth_duck };
    rooms[2].monster = m;
    
    check_solution(rooms, { 0 }, 2, 0); // Cannot stealth steal treasure
    
    rooms[3].items = { sword };
    // Stealth to 3, grab sword & kill monster
    check_solution(rooms, { 0 }, 2, 7);
    
    rooms[3].items = {};
    rooms[1].items = { sword };
    check_solution(rooms, { 0 }, 2, 5);
    
    rooms[1].monster = m;
    check_solution(rooms, { 0 }, 2, 0); // Cannot pickup while stealthing
}

void example_tests() {
    const Item sword = {
        .name = "Sword", .type = Item::Weapon,
        .off = 10, .def = -1,
    };
    
    const Item berserker_sword = {
        .name = "Berserker's Sword", .type = Item::Weapon,
        .hp = -1'000, .off = 10'000, .def = 0,
        .stacking_off = 1'000, .stacking_def = -500,
        .first_attack = true
    };
    
    const Item heavy_armor = {
        .name = "Heavy Armor", .type = Item::Armor,
        .hp = 5'000, .off = -10, .def = 300,
    };
    
    const Item debugging_duck = {
        .name = "Debugging Duck", .type = Item::RubberDuck,
        .stacking_off = 1,
        .stealth = true
    };
    
    std::vector<Room> rooms(14);
    enum : RoomId {
        no_monster = 10,
        weak,
        strong,
        durable
    };
    
    rooms[no_monster] = { {}, {}, { heavy_armor } };
    rooms[weak] = { {}, Monster{ .hp = 1000, .off = 10 }, { debugging_duck, sword } };
    rooms[strong] = { {}, Monster{ .hp = 10, .off = 10'000, .def = 1'000'000 },
    { berserker_sword } };
    rooms[durable] =  { {}, Monster{ .hp = 100'000, .off = 10, .stacking_def = 1 },
    { berserker_sword } };
    
    auto link = [&](RoomId a, RoomId b) {
        rooms[a].neighbors.push_back(b);
        rooms[b].neighbors.push_back(a);
    };
    
    link(0, no_monster);
    link(0, weak);
    link(weak, 7);
    link(0, strong);
    link(strong, 8);
    link(0, 1);
    link(1, 2);
    link(2, durable);
    link(durable, 6);
    
    check_solution(rooms, { 0 }, 0, 1); // Treasure at entrance
    check_solution(rooms, { 9 }, 0, 0); // No path to treasure
    check_solution(rooms, { 8 }, 0, 0); // Blocked by monster
    check_solution(rooms, { durable }, durable, 0); // Killed on spot
    check_solution(rooms, { 7 }, 0, 5); // Kills weak monster
    check_solution(rooms, { 6, 7 }, 2, 7); // Sneaks around durable
    check_solution(rooms, { 6, 7 }, durable, 9); // Kills durable
}

void example_tests2() {
    const Item duck_of_power = {
        .name = "Duck of Power", .type = Item::RubberDuck,
        .hp = 10'000'000, .off = 10'000'000, .def = 10'000'000,
    };
    
    const Item dull_sword = {
        .name = "Dull Sword", .type = Item::Weapon,
        .off = -10, .def = -5,
    };
    
    const Item sword = {
        .name = "Sword", .type = Item::Weapon,
        .off = 5, .def = -1,
    };
    
    const Item leather_pants = {
        .name = "Leather pants", .type = Item::Armor,
        .off = -3, .def = 1,
        .first_attack = true
    };
    
    const Item defensive_duck = {
        .name = "Defensive Duck", .type = Item::RubberDuck,
        .off = -2, .def = 8,
    };
    
    const Item stealth_duck = {
        .name = "Stealth Duck", .type = Item::RubberDuck,
        .off = -100, .def = -100,
        .stealth = true,
    };
    
    const Item slow_sword = {
        .name = "Slow Sword", .type = Item::Weapon,
        .off = -10'000,
        .stacking_off = 1,
    };
    
    constexpr int CYCLE_LEN = 100;
    enum : RoomId {
        impossible = CYCLE_LEN,
        r1, r2, r3, r4, r4a, r4b, ROOM_COUNT
    }; 
    std::vector<Room> rooms(ROOM_COUNT);
    
    auto link = [&](RoomId a, RoomId b) {
        rooms[a].neighbors.push_back(b);
        rooms[b].neighbors.push_back(a);
    };
    
    for (int i = 1; i < CYCLE_LEN; i++) link(i - 1, i);
    rooms[CYCLE_LEN-1].neighbors.push_back(0);
    
    rooms[impossible] = { {}, {{ .hp = 1'000'000, .off = 1'000'000 }}, { duck_of_power } };
    link(impossible, 0);
    
    rooms[r1] = { {}, {{ .hp = 9'999, .off = 3, .def = 2 }}, { defensive_duck, dull_sword } };
    link(r1, 1);
    
    rooms[r2] = { {}, {{ .hp = 100'000, .off = 10 }}, { sword, leather_pants } };
    link(r2, CYCLE_LEN - 3);
    
    rooms[r3] = { {}, {{ .hp = 100'000, .off = 10, .def = 1 }}, { stealth_duck, slow_sword } };
    link(r3, 2);
    
    rooms[r4] = { { r4a }, {{ .hp = 10'000, .off = 10'000 }}, {} };
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wmissing-field-initializers"
    rooms[r4a] = { { r4b } };
    #pragma GCC diagnostic pop
    rooms[r4b] = { {}, {{ .hp = 10'000, .off = 1 }}, {} };
    link(r4, CYCLE_LEN - 4);
    link(r4b, CYCLE_LEN - 4);
    
    // r1 (loots duck) -> r2 (loots pants & sword) -> r3
    check_solution(rooms, { 0 }, r3, CYCLE_LEN + 11);
    // r1 (loots duck) -> r2 (loots pants & sword) -> r3 (loots stealth duck) -> r4a
    check_solution(rooms, { 0 }, r4a, 2*CYCLE_LEN + 11);
}

void example_tests3() {
    const Item sword = {
        .name = "Sword", .type = Item::Weapon,
        .off = 10,
    };
    
    const Item stacking_duck = {
        .name = "Stacking Duck", .type = Item::RubberDuck,
        .hp = -9'999, .stacking_off = 100,
    };
    
    const Item heavy_armor = {
        .name = "Heavy Armor", .type = Item::Armor,
        .off = -1'000, .def = 1'000,
    };
    
    enum : RoomId {
        start, treasure, short_path, long_path = short_path + 3,
        COUNT = long_path + 4
    };
    
    std::vector<Room> rooms(COUNT);
    
    auto link = [&](RoomId a, RoomId b) {
        rooms[a].neighbors.push_back(b);
        rooms[b].neighbors.push_back(a);
    };
    
    rooms[treasure].neighbors.push_back(start); // one-way back to start
    
    link(start, long_path + 0);
    link(long_path + 0, long_path + 1);
    link(long_path + 1, long_path + 2);
    link(long_path + 2, long_path + 3);
    link(long_path + 3, treasure);
    
    link(start, short_path + 0);
    link(short_path + 0, short_path + 1);
    link(short_path + 1, short_path + 2);
    link(short_path + 2, treasure);
    
    rooms[short_path + 0].items = { sword };
    rooms[short_path + 1].monster = Monster{ .hp=10'000, .off=5, .def=3 };
    rooms[short_path + 1].items = { stacking_duck, heavy_armor };
    rooms[short_path + 2].monster = Monster{ .hp=100'000, .off=5, .def=3 };
    
    check_solution(rooms, { start }, treasure, 6);
}

void example_tests4() {
    const Item sword = {
        .name = "Sword", .type = Item::Weapon,
        .off = 3, .def = -1
    };
    
    enum : RoomId {
        start, treasure, short_path, long_path = short_path + 3,
        COUNT = long_path + 4
    };
    
    std::vector<Room> rooms(COUNT);
    
    auto link = [&](RoomId a, RoomId b) {
        rooms[a].neighbors.push_back(b);
        rooms[b].neighbors.push_back(a);
    };
    
    rooms[treasure].neighbors.push_back(start); // one-way back to start
    
    link(start, long_path + 0);
    link(long_path + 0, long_path + 1);
    link(long_path + 1, long_path + 2);
    link(long_path + 2, long_path + 3);
    link(long_path + 3, treasure);
    
    link(start, short_path + 0);
    link(short_path + 0, short_path + 1);
    link(short_path + 1, short_path + 2);
    link(short_path + 2, treasure);
    
    Monster needs_sword = Monster{ .hp=10'000, .off=6, .def=3 };
    Monster no_sword = Monster{ .hp=100'000, .off=3 };
    rooms[short_path + 0].monster = needs_sword;
    rooms[short_path + 1].monster = no_sword;
    rooms[short_path + 0].monster = needs_sword;
    
    check_solution(rooms, { start }, treasure, 7);
}

void example_tests5() {
    const Item sword = {
        .name = "Sword", .type = Item::Weapon,
        .off = 5, .def = -1,
    };
    
    constexpr int LEN = 300;
    std::vector<Room> rooms(LEN);
    
    auto link = [&](RoomId a, RoomId b) {
        rooms[a].neighbors.push_back(b);
        rooms[b].neighbors.push_back(a);
    };
    
    for (int i = 1; i < LEN; i++) {
        rooms[i].items = { sword, sword, sword };
        link(i - 1, i);
    }
    
    rooms[LEN - 1].monster = Monster{ .hp = 1'000'000, .off = 1'000'000 };
    
    check_solution(rooms, { 0 }, LEN - 1, 0);
}

void example_tests6() {
    const Item defensive_duck = {
        .name = "Defensive Duck", .type = Item::RubberDuck,
        .off = -100, .def = 100,
    };
    
    constexpr int LEN = 31;
    std::vector<Room> rooms(LEN + LEN + 10);
    
    auto link = [&](RoomId a, RoomId b) {
        rooms[a].neighbors.push_back(b);
        rooms[b].neighbors.push_back(a);
    };
    
    rooms[0].items = { defensive_duck }; 
    
    assert(LEN % 2 == 1);
    for (int i = 1; i + 1 < LEN; i += 2) {
        link(i - 1, i);
        link(i, i + 1);
        
        rooms[i+1].items = { defensive_duck };
        
        rooms[i].monster = Monster{ .hp = 10'000'000, .off = 50, .def = -120 };
        rooms[i+1].monster = Monster{ .hp = 10'000, .off = 1, .def = 1 };
    }
    
    for (int i = 1; i < LEN + 10; i++)
    link(LEN + i - 1, LEN + i);
    
    link(0, LEN);
    link(LEN - 1, 2*LEN + 10 - 1);
    
    check_solution(rooms, { 0 }, LEN - 1, 2*LEN - 1);
}

void example_tests7() {
    const Item sworddd = {
        .name = "excalibur", .type = Item::Weapon,
        .off = 5, .def = 0, .first_attack = true
    };
    
    std::vector<Room> rooms(6);
    
    auto link = [&](RoomId a, RoomId b) 
    {
        rooms[a].neighbors.push_back(b);
        rooms[b].neighbors.push_back(a);
    };
    
    rooms[2].items = { sworddd }; 

    rooms[4].monster = Monster{.hp = 1, .off = 1000000,};
    
    link(0, 1);
    link(1, 2);
    link(1, 3);
    link(3, 4);
    link(4, 5);

    check_solution(rooms, {0}, 5, 11);
}            
            
int main()
{
    combat_examples();
    stealth_examples();
    example_tests();
    example_tests2();
    example_tests3();
    example_tests4();
    example_tests5();
    example_tests6();
    example_tests7();
}

#endif
