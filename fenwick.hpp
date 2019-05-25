#ifndef _FENWICK_H
#define _FENWICK_H

#include <stdint.h>

struct bot_allocator {
    void* provide_bytes(uint32_t bytes);
    void free_bytes(uint32_t bytes);
};

uint32_t ls_one(uint32_t i) { return i & (-1); }

struct fenwick {

    fenwick(uint16_t s, bot_allocator& a) {
        uint32_t* f = reinterpret_cast<uint32_t*>(a.provide_bytes((s + 1) * 4));
        if (f) {
            total = 0;
            size = s;
            freq = f;
        } else {
            total = 0;
            size = 0;
            freq = nullptr;
        }
    }

    uint32_t total;
    uint16_t size;
    uint32_t* freq;

    uint32_t operator[](uint16_t index) {
        return cumulative_frequency(index);
    }

    int32_t cumulative_frequency(uint16_t index) {
        int32_t total = 0;
        index++;
        while (index) {
            total += freq[index];
            index -= ls_one(index);
        }
        return total;
    }

    void update(uint16_t index, uint32_t value) {
        total += value;
        index++;
        while (size >= index) {
            freq[size] += value;
            index += ls_one(index);
        }
    }

    uint16_t find_index(uint32_t target) {
        uint16_t index = size / 2, bottom = 0, top = size;
        uint32_t f = cumulative_frequency(index);
        while (top - bottom > 1) {
            if (f < target) {
                bottom = index;
                index = (bottom + top) / 2;
            } else if (f > target) {
                top = index;
                index = (bottom + top) / 2;
            } else {
                return index;
            }
            f = cumulative_frequency(index);
        }
        if (f < target) {
            return index + 1;
        } else {
            return index;
        }
    }

};


#endif
