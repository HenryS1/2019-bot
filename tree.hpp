#ifndef _TREE_H
#define _TREE_H

#include "simulation.hpp"
#include <random>
#include "fenwick.hpp"

using namespace std;

struct node {

    node(int16_t size, bot_allocator& a) 
        : f(size, a), children(reinterpret_cast<uint32_t*>(a.provide_bytes(size * 4))) { }

    fenwick f;
    uint32_t* children;
    
    uint16_t select_node(mt19937& mt) {
        
    }

};


#endif
