# Scipio-bin-to-cfg
An educational binary analysis project exploring how to translate raw machine code into Control Flow Graphs (CFG) using C and C++


Scipio (Work in Progress)
Scipio is a student research project currently in development, aimed at exploring the fundamentals of binary analysis and reverse engineering. The primary goal is to build a proof-of-concept tool capable of decoding raw machine code to construct basic Control Flow Graphs (CFGs).

This repository serves as the foundation for my upcoming undergraduate engineering thesis. It is very much a learning exercise intended to deepen my understanding of system architectures, memory management, and graph theory.

Currently, I am attempting to target a restricted subset of the [ARM64 / x86] architecture.

Experimental Hybrid Architecture
As part of my thesis research, I am experimenting with a hybrid C/C++ architecture to understand how to bridge low-level parsing with higher-level data structures:

The C Component (Binary Parsing): I am writing the raw binary parsing (ELF/PE headers) and bitwise opcode decoding in pure C. The goal here is to learn how to handle raw bytes, pointer arithmetic, and memory allocation without relying on modern abstractions.

The C++ Component (Graph Modeling): Once the instructions are decoded, they are passed into a C++ engine. I am using C++ (specifically the Standard Template Library and RAII) to manage the complexity of building the nodes and edges of the Control Flow Graph, which helps prevent the memory leaks I would likely introduce if I built the graph entirely in C.

Project Milestones
The current roadmap for the thesis involves three main phases:

The Decoder: Write a basic parser that can open an executable binary, locate the .text section, and decode a limited subset of hardware-specific opcodes into a readable format.

Basic Block Generation: Implement an algorithm to scan the decoded instructions, identifying branching logic (jumps, calls, returns) to group the code into sequential Basic Blocks.

Graph Construction: Connect these Basic Blocks dynamically to generate a mapped representation (CFG) of the program's execution paths, ideally outputting data that can be visualized later.