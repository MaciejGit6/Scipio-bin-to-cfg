#ifndef BASIC_BLOCK_HPP
#define BASIC_BLOCK_HPP

#include <string>
#include <vector>
#include <cstdint>
#include <memory>

// 1. Structure to hold a single decoded assembly instruction
struct Instruction {
    uint64_t address;
    std::string mnemonic;
    std::string operands;
};

// 2. The Node of our Graph Matrix
class BasicBlock {
public:
    uint64_t start_address;
    uint64_t end_address;
    
    // The straight-line code inside this block
    std::vector<Instruction> instructions;

    // Edges in our graph: Where does the program go after this block?
    // Using shared_ptr prevents memory leaks when building complex graphs.
    std::vector<std::shared_ptr<BasicBlock>> successors;

    // Constructor requires a starting address
    BasicBlock(uint64_t start) : start_address(start), end_address(0) {}

    // Helper method to cleanly add instructions from Capstone
    void add_instruction(uint64_t addr, const char* mnemonic, const char* operands) {
        instructions.push_back({addr, std::string(mnemonic), std::string(operands)});
        
        // The end address continuously updates to the last instruction added
        end_address = addr; 
    }
};

#endif // BASIC_BLOCK_HPP