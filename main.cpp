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
    int tag = 0;
    int data = 0;
    int history = 0;
};

void lwHit(Block &accessedBlock, Block &otherBlock, int rt_register, int* registers) { // Cache data -> reg.file[rt]
    registers[rt_register - 16] = accessedBlock.data;
    accessedBlock.history = 1;
    otherBlock.history = 0;

    std::cout << "hit\n";
}

void lwMiss(Block &block0, Block &block1, int tag, int index, int rt_register, int* registers, int* main_memory) {
    /*
     * 1. Select victim block from 2 blocks;
     * 2. If victim's valid bit == 1, update memory;
     * 3. Bring (copy) memory block to victim block;
     * 4. Set valid bit = 1, and tag field;
     * 5. reg.file[rt] <-- cache block data;
     */

    Block* victim_block;
    if (block0.history == 0)
        victim_block = &block0;
    else
        victim_block = &block1;

    if (victim_block->valid == 1) {
        main_memory[victim_block->tag * 8 + index] = victim_block->data;
    }

    // Bring copy (data) of memory block to victim block
    victim_block->data = main_memory[tag * 8 + index];

    victim_block->valid = 1;
    victim_block->tag = tag;

    registers[rt_register - 16] = victim_block->data;

    victim_block->history = 1;
    if (victim_block == &block0)
        block0.history = 1;
    else
        block0.history = 0;

    std::cout << "miss\n";
}

void swHit(Block &accessedBlock, Block &otherBlock, int rt_register, int* registers) { // reg.file[rt] --> cache data; (update cache only) WB cache
    accessedBlock.data = registers[rt_register - 16];
    accessedBlock.history = 1;
    otherBlock.history = 0;

    std::cout << "hit\n";
}

void swMiss(int physical_address, int rt_register, int* registers, int* main_memory) { // reg.file[rt] --> memory[word_addr]; (update memory only)
    main_memory[physical_address] = registers[rt_register - 16];

    std::cout << "miss\n";
}

void print32Bit(int v){
    int mask = 1; // mask = 00000000000000000000000000000001
    mask <<= 31; // shift left 31 positions
                 // mask = 10000000000000000000000000000000
    for (int i = 0; i < 32; i++) {
        if ((v & mask) == 0)
            std::cout << "0";
        else
            std::cout << "1";

        v <<= 1; // shift left 1 bit
    }
}

void print4Bit(int v) {
    int mask = 1;
    mask <<= 3;
    for (int i = 0; i < 4; i++) {
        if ((v & mask) == 0)
            std::cout << "0";
        else
            std::cout << "1";

        v <<= 1;
    }
}

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

    int register_file[8] = {0}; // 8 register files with all 0's

    Block cache[8][2]; // 2-way set associative cache with all 0's ; initialize every index with empty blocks
    for (int i = 0; i < 8; i++) {
        cache[i][0] = Block();
        cache[i][1] = Block();
    }

    int main_memory[128]; // 128 main memory blocks, 1 word per block, initialized with block_addr + 5
    for (int i = 0; i < 128; i++)
        main_memory[i] = i + 5;

    std::ifstream file("input_object_code");
    std::string s;

    while (std::getline(file,s)){
        int opcode;
        int rt_register = 0;
        int offset = 0;

        // Decoding
        if (s.substr(0, 6) == "100011")
            opcode = 35; // lw instr.
        else if (s.substr(0, 6) == "101011")
            opcode = 43; // sw instr.

        // $rs will always be zero
        // Decoding register_rt
        for (int i = 15; i >= 11; i--) {
            if (s.at(i) == '1')
                rt_register += pow(2, 15 - i);
        }

        // Decoding offset
        for (int i = 31; i > 15; i--) {
            if (s.at(i) == '1')
                offset += pow(2, 31 - i);
        }

        // Calculating physical address and then both the tag and index for the cache
        int word_address = offset / 4; // Converting from byte to word address
        int tag = word_address / 8;
        int index = word_address % 8;

        // Based on the index, we are going to check this set in our cache
        Block& set_block0 = cache[index][0];
        Block& set_block1 = cache[index][1];

        // Checking if there is valid data in one of the blocks, as well as matching tags
        bool hit_on_block0 = set_block0.valid && set_block0.tag == tag;
        bool hit_on_block1 = set_block1.valid && set_block1.tag == tag;

        // Printing out current instruction before printing out 'hit' or 'miss'
        std::cout << s << " ";

        // lw instruction
        if (opcode == 35) {
            if (hit_on_block0 == true)
                lwHit(set_block0, set_block1, rt_register, register_file);
            else if (hit_on_block1 == true)
                lwHit(set_block1, set_block0, rt_register, register_file);
            else // hit == false
                lwMiss(set_block0, set_block1, tag, index, rt_register, register_file, main_memory);
        }

        // sw instruction
        if (opcode == 43) {
            if (hit_on_block0 == true) {
                swHit(set_block0, set_block1, rt_register, register_file);
            }
            else if (hit_on_block1 == true) {
                swHit(set_block1, set_block0, rt_register, register_file);
            }
            else
                swMiss(word_address, rt_register, register_file, main_memory);
        }
    }

    file.close();

    std::cout << "\n\n";

    // Printing Register Values
    std::cout << "Registers\n";
    for (int i = 0; i < 8; i++) {
        std::cout << "$S" << i << "\t";
        print32Bit(register_file[i]);
        std::cout << "\n";
    }

    std::cout << "\n\n";

    // Printing Cache Block Values
    std::cout << "Cache Block 0                                               Cache Block 1\n"
              << "Set#     Valid   Tag     Data                               Set#     Valid   Tag     Data\n";
    for (int i = 0; i < 8; i++) {
        std::cout << i << "        " << cache[i][0].valid << "       ";
        print4Bit(cache[i][0].tag);
        std::cout << "    ";
        print32Bit(cache[i][0].data);

        std::cout << "   ";

        std::cout << i << "        " << cache[i][0].valid << "       ";
        print4Bit(cache[i][0].tag);
        std::cout << "      ";
        print32Bit(cache[i][0].data);
        std::cout << "\n";
    }

    std::cout << "\n\n";

    // Printing Main Memory Address Values
    std::cout << "Addr      Data\n";
    for (int i = 0; i < 128; i++) {
        // This is just to make the formatting way cleaner, because as 'i' gets bigger, it pushes the data value further
        if (i >= 0 && i < 10)
            std::cout << i << "         ";
        else if (i >= 10 && i < 100)
            std::cout << i << "        ";
        else
            std::cout << i << "       ";

        print32Bit(main_memory[i]);
        std::cout << "\n";
    }

    return 0;
}