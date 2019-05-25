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
    
    uint16_t select_node(uint32_t random_bytes) {
        uint32_t threshold = random_bytes % f.total;
        return f.find_index(threshold);
    }

};


#endif
