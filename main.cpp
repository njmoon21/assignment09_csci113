#include <iostream>
#include <fstream>
#include <string>
#include <cmath>

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

struct Block {
    /*
     * valid = whether the cache block has data or not
     * tag = the 4 MSBs of the physical 7-bit address, determining which memory block was stored
     * data = the 3 bit index from the physical 7-bit address we'll store
     */

    int valid = 0;
    int tag, data;
};

int main() {
    /*
     * 0. Prepare data structures for register_file, cache, and main_memory modules;
     *
     * Logic:
     * 1. Read a machine instruction from input data file;
     * 2. Decode (analyze) the instruction:
     *   - determine opcode (lw: 35, sw: 43);
     *   - determine rs and rt register numbers;
     *   - determine byte_offset value;
     * 3. Compute effective memory address (byte address) and convert it to a word address;
     * 4. From the word address, compute cache index and tag values using MOD and DIV;
     * 5. Search cache and do read/write operation, i.e., one of
     *   {read_hit, read_miss, write_hit, write_miss};
     * 6. repeat step 1~5 for each instruction in the input data file (while loop until EOF).
     */

    std::ifstream file("input_object_code");
    int register_file[8]; // 8 register files
    Block cache[8][2]; // 2-way set associative cache
    int main_memory[128]; // 128 main memory blocks, 1 word per block

    while (!file.eof()){
        std::string s;
        int opcode;
        int register_rt = 0;
        int offset = 0;

        // Fetching line instruction
        std::getline(file, s);

        // Decoding
        if (s.substr(0, 6) == "100011")
            opcode = 35; // lw instr.
        else if (s.substr(0, 6) == "101010")
            opcode = 43; // sw instr.

        // $rs will always be zero
        // Decoding register_rt
        for (int i = 15; i >= 11; i--) {
            if (s.at(i) == '1')
                register_rt += pow(2, 15-i);
        }

        // Decoding offset
        for (int i = 31; i > 15; i--) {
            if (s.at(i) == '1')
                offset += pow(2, 31-i);
        }
        offset /= 4;  // Converting from byte to word address
    }

    file.close();
    return 0;
}