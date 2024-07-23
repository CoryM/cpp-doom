//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// DESCRIPTION:
//  MapObj data. Map Objects or mobjs are actors, entities,
//  thinker, take-your-pick... anything that moves, acts, or
//  suffers state changes of more or less violent nature.
//


#ifndef __D_THINK__
#define __D_THINK__

#include <functional>
#include <variant>
#include <string>
//
// Experimental stuff.
// To compile this as "ANSI C with classes"
//  we will need to handle the various
//  action functions cleanly.
//

struct mobj_t;
struct player_t;
struct pspdef_t;
struct strobe_t;
struct ceiling_t;
struct vldoor_t;
struct floormove_t;
struct plat_t;
struct lightflash_t;
struct glow_t;
struct fireflicker_t;

// Store state rather then an action to take.
enum class actionf_v {
    empty = 0,     // added empty actionf_v to allow for nullptr initialization
    deleteMe = -1, // the memory for this object should be deleted//removed
    one = 1,       // what does this do?
    NotActionf_v = 2 // What to report if the actionf_t does not store a actionf_v
};
typedef void (*actionf_p1)(mobj_t *mo);
typedef void (*actionf_p2)(player_t *player, pspdef_t *psp);
typedef void (*actionf_p3)(mobj_t *mo, player_t *player, pspdef_t *psp); // [crispy] let pspr action pointers get called from mobj states
typedef void (*actionf_p4)(strobe_t *st);
typedef void (*actionf_p5)(ceiling_t *ceiling);
typedef void (*actionf_p6)(vldoor_t *door);
typedef void (*actionf_p7)(floormove_t *floor);
typedef void (*actionf_p8)(plat_t *plat);
typedef void (*actionf_p9)(lightflash_t *flash);
typedef void (*actionf_p10)(glow_t *glow);
typedef void (*actionf_p11)(fireflicker_t *fire);

using actionf_f1 = std::function<void(mobj_t *)>;
using actionf_f2 = std::function<void(player_t *, pspdef_t *)>;

struct actionf_t {
private:
    template<class U>
    auto get_void_impl(void * ptr) const
    {
      if(std::holds_alternative<U>(func)){
        ptr = reinterpret_cast<void *>( std::get<U>(func));
      };
      return ptr;
     };

public:
    std::variant<
        std::monostate, 
        actionf_v, 
        actionf_p1, 
        actionf_p2, 
        actionf_p3, 
        actionf_p4, 
        actionf_p5, 
        actionf_p6, 
        actionf_p7, 
        actionf_p8, 
        actionf_p9, 
        actionf_p10, 
        actionf_p11> func;
    
    actionf_t() : func { std::monostate{} } {};

    template<class T>
    actionf_t(T f) : func(f)
    {
    };

    void set(actionf_p1 f)
    {
        //auto i = reinterpret_cast<size_t>(f);
        //auto msg = "set actionf_p1: " + std::to_string(i);
        //std::puts(msg.c_str());
        func.emplace<actionf_p1>(f);
    };

    void * get_void_star() const
    {
        void *ptr = nullptr;
        
        // This is a bit ugly, but it's the only way to get the pointer to the function
        ptr = get_void_impl<actionf_v>(ptr);
        ptr = get_void_impl<actionf_p1>(ptr);
        ptr = get_void_impl<actionf_p2>(ptr);
        ptr = get_void_impl<actionf_p3>(ptr);
        ptr = get_void_impl<actionf_p4>(ptr);
        ptr = get_void_impl<actionf_p5>(ptr);
        ptr = get_void_impl<actionf_p6>(ptr);
        ptr = get_void_impl<actionf_p7>(ptr);
        ptr = get_void_impl<actionf_p8>(ptr);
        ptr = get_void_impl<actionf_p9>(ptr);
        ptr = get_void_impl<actionf_p10>(ptr);
        ptr = get_void_impl<actionf_p11>(ptr);
        
        return ptr;
    };

    // comparison operators
    [[nodiscard]] bool operator==(const actionf_t &rhs) const
    {
        if (std::holds_alternative<actionf_v>(func)) {
            return func == rhs.func;
        } 
        // Holds pointer to function
        return get_void_star() == rhs.get_void_star();
    };

    [[nodiscard]] bool operator!=(const actionf_t &rhs) const
    {
        return !(*this == rhs);
    };

    [[nodiscard]] bool operator==(const actionf_v &rhs) const {
        if (std::holds_alternative<actionf_v>(func) == false) {
            return false;
        }
        return std::get<actionf_v>(func) == rhs;
    }

    // true if any of the acp1, acp2, or acp3 is set
    bool is_set() const
    {
        return func.index() > 1;
    };

    actionf_v get_v() const
    {
        if (std::holds_alternative<actionf_v>(func) == false) {
            return actionf_v::NotActionf_v;
        }
        return std::get<actionf_v>(func);
    };

    bool is_p3() const
    {
        return std::holds_alternative<actionf_p3>(func);
    };

    void operator()(mobj_t *mo, player_t *player, pspdef_t *psp)
    {
        if (std::holds_alternative<actionf_p3>(func)) {
            std::get<actionf_p3>(func)(mo, player, psp);
        }
    };

    // yolo is a wrapper function to call the appropriate action function
    inline void yolo(auto *pointy) const
    {
        if (std::holds_alternative<actionf_p1>(func)) { 
            std::get<actionf_p1>(func)(reinterpret_cast<mobj_t * >(pointy));

        } else if (std::holds_alternative<actionf_p2>(func)) {
            std::get<actionf_p2>(func)(reinterpret_cast<player_t *>(pointy), nullptr);

        } else if (std::holds_alternative<actionf_p3>(func)) {
            std::get<actionf_p3>(func)(reinterpret_cast<mobj_t * >(pointy), nullptr, nullptr);

        } else if (std::holds_alternative<actionf_p4>(func)) {
            std::get<actionf_p4>(func)(reinterpret_cast<strobe_t *>(pointy));

        } else if (std::holds_alternative<actionf_p5>(func)) {
            std::get<actionf_p5>(func)(reinterpret_cast<ceiling_t *>(pointy));

        } else if (std::holds_alternative<actionf_p6>(func)) {
            std::get<actionf_p6>(func)(reinterpret_cast<vldoor_t *>(pointy));

        } else if (std::holds_alternative<actionf_p7>(func)) {
            std::get<actionf_p7>(func)(reinterpret_cast<floormove_t *>(pointy));

        } else if (std::holds_alternative<actionf_p8>(func)) {
            std::get<actionf_p8>(func)(reinterpret_cast<plat_t *>(pointy));

        } else if (std::holds_alternative<actionf_p9>(func)) {
            std::get<actionf_p9>(func)(reinterpret_cast<lightflash_t *>(pointy));

        } else if (std::holds_alternative<actionf_p10>(func)) {
            std::get<actionf_p10>(func)(reinterpret_cast<glow_t *>(pointy));

        } else if (std::holds_alternative<actionf_p11>(func)) {
            std::get<actionf_p11>(func)(reinterpret_cast<fireflicker_t *>(pointy));
        }
    }
};


// Doubly linked list of actors.
struct thinker_t {
    struct thinker_t *prev = nullptr;
    struct thinker_t *next = nullptr;
    actionf_t           function{};

};


#endif
