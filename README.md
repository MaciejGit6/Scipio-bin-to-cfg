# Scipio-bin-to-cfg

An educational binary analysis tool exploring how to translate raw machine code into Control Flow Graphs (CFG), written in hybrid C/C++. Built as the foundation for my undergraduate engineering thesis.

```
                           ..........            
                   --iiiii---------ii###ii-.                                 ####  
               .#00#i-.....---------......--i##-.                           ##     
                 .i#i#####000000000000##ii--. .-##i.                         ###   
                     -01111111111111111100000#i-...-0#-                        ##  
                       i1000##########00011110000#-..i00#                   ####   
                        ####iiiiiiiiiiiii###0011100#-.-#01i                
                        .i###i-----iiiiiiiii####01110i--i010.                ####  
                        .-0##i--.....-iiiiiiiii####010#i-i011-              ##  ## 
          #i-.      .i#00##iiiiii-..----iii--i#ii####110#-i011-             ##     
          -0100#i..0000##i#iii---i-ii----iii#i--##i###010i.-010.            ##  ## 
            i10000####i##i##iiiiii-i#-i--i###000-i0##0#010i.i010             ####  
             #000##ii###ii##0#####i###########000i-0#00001#--#11i          
             i0###i---i#00####0###00i####i-iii#001i.0#00000i.i010           ###### 
             000#ii--iii#00000000ii---#00#####00011-i#0#000#-i011             ##   
            #100#ii-..-iii#00000#--.-i#00000##00011#-0#0000#-i011             ##   
             0#iii##000####000#0000000000#0000000110-0#0001#-i011             ##   
             i111100000##00000#0000##00110###0010110-000000i.i010           ###### 
             .-#00i..-#ii-.iiii010##ii#00##i##00001-i###000ii011-          
            .    i. .iii..-ii##10#0# i#00##ii#0010#-.-i###0-i010            #####  
           . ----ii###i--#i##0000#0#i#01##i-i#000#.     10ii#00             ##  ## 
           i---i#i#0#i--##--i#000-.#01111##--0010.     -10###0.             #####  
            .-#.-i#0##i-#00##000#11111111110i-#0#-     i10000-              ##     
              ii#i0#i##0ii0##00##10##001111110####.    -11000               ##     
              -i##00#000##0000##0#.  i#01111111100#i.   #1010              
               -##00#0000000###0#.   .i#0111111111110#-  i000- -i           ###### 
              -i#000000####00110-    .-i#00111-.-#011110- .i#000-             ##   
              .#0##0000#0111110i.    .-i##0011#      ....                     ##   
                       0111110i.    .-i#000111i                               ##   
                       -11001#i--...--i##ii###0#--.                         ###### 
                        .#010##iiiiii--..-ii##000010-                      
                         -00#-ii#0#-..-iii#####0000010i                      ####  
                         -iii#####iiii...-i###000011111#.                   ##  ## 
                       ---i#0###iii##0110#i--..                             ##  ## 
                      ---#i010####010#-.                                    ##  ## 
                     i#00#011100#-.                                          ####  
                    .#00011#i-                                             
                      --..
```

## Demo

![Scipio in action](docs/assets/demo.gif)

## Description

Scipio is a student research project aimed at exploring the fundamentals of binary analysis and reverse engineering. The primary goal is to build a proof-of-concept tool capable of decoding raw machine code and constructing basic Control Flow Graphs (CFGs) from it.

This repository serves as the foundation for my upcoming undergraduate engineering thesis — a learning exercise to deepen my understanding of system architectures, memory management, and graph theory. Currently targeting a restricted subset of the x86-64 architecture.

## Hybrid Architecture

As part of my thesis research, I'm experimenting with a hybrid C/C++ design to understand how to bridge low-level parsing with higher-level data structures:

**The C Component (Binary Parsing)** — Raw binary parsing (ELF headers) and bitwise opcode decoding is written in pure C. The goal is to handle raw bytes, pointer arithmetic, and memory allocation without modern abstractions.

**The C++ Component (Graph Modeling)** — Once instructions are decoded, they are passed into a C++ engine that uses the STL and RAII to manage the complexity of building CFG nodes and edges — preventing the memory leaks I would likely introduce if I built the graph entirely in C.

## Build

```bash
make
```

After building, make Scipio accessible from any directory:

```bash
sudo ln -s $(pwd)/scipio /usr/local/bin/scipio
```

## Usage

```bash
scipio <executable> <output.cfg>
```

The fastest way to test it is to run Scipio on itself:

```bash
scipio scipio out.cfg
```

## Example Output

Running Scipio on itself produces this CFG visualization (only a small fragment of the full graph):

<img width="1871" height="778" alt="image" src="https://github.com/user-attachments/assets/440def56-a043-4dc7-a26c-fa54a543f5a9" />

## Project Milestones

**The Decoder** — A parser that opens an executable binary, locates the `.text` section, and decodes a subset of x86-64 opcodes into a readable format.

**Basic Block Generation** — Scans decoded instructions and identifies branching logic (jumps, calls, returns) to group code into sequential Basic Blocks.

**Graph Construction** — Connects Basic Blocks dynamically to generate a mapped representation of the program's execution paths, exported to Graphviz DOT format for visualization.

## Later Plans

By representing the CFG as an adjacency matrix and computing its eigenvalues, it should be possible to spot structural similarities between different binaries — if two executables produce matrices with equal eigenvalues, one is likely a modified variant of the other. The long-term goal is to use this as a foundation for malware detection.

## Dependencies

- Linux x86_64
- GCC / G++
- [Capstone](https://www.capstone-engine.org/) disassembly framework (`sudo apt install libcapstone-dev`)
- [Graphviz](https://graphviz.org/) for SVG rendering (`sudo apt install graphviz`)
