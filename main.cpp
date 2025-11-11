#include <iostream>

/*
 * ==== MEMORY HIERARCHY SIMULATION PROGRAMMING ====
 * 1. Get a clear understand on memory hierarchy
 *
 * Main Memory (128 Words ; addresses 0-127)    int Mem(127);
 * Register Files (s0-s7)   int reg[8];
 * Cache (2-way set cache)
 *    Struct block {
 *       int Valid;
 *       int Tag;
 *       int Data
 *    }
 *    Struct block[8][2];
 *
 * ==== HIT VS. MISS ====
 * Hit -> The content you want is already in cache
 * Miss -> If it's not in cache, go to Main Memory
 *
 * 4 Conditions:
 *    LW hit
 *    LW Miss
 *    SW hit
 *    SW Miss
 */

int main() {


    return 0;
}