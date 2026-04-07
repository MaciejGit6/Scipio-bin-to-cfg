# Scipio-bin-to-cfg
An educational binary analysis project exploring how to translate raw machine code into Control Flow Graphs (CFG) using C and C++

                                                                                
                               ..........                                       
                        --iiiii---------ii###ii-.                               
                    .#00#i-.....---------......--i##-.                          
                      .i#i#####000000000000##ii--. .-##i.                       
                         -01111111111111111100000#i-...-0#-                     
                           i1000##########00011110000#-..i00#                   
                            ####iiiiiiiiiiiii###0011100#-.-#01i                 
                            .i###i-----iiiiiiiii####01110i--i010.               
                            .-0##i--.....-iiiiiiiii####010#i-i011-              
              #i-.      .i#00##iiiiii-..----iii--i#ii####110#-i011-             
              -0100#i..0000##i#iii---i-ii----iii#i--##i###010i.-010.            
                i10000####i##i##iiiiii-i#-i--i###000-i0##0#010i.i010            
                 #000##ii###ii##0#####i###########000i-0#00001#--#11i           
                 i0###i---i#00####0###00i####i-iii#001i.0#00000i.i010           
                 000#ii--iii#00000000ii---#00#####00011-i#0#000#-i011           
                #100#ii-..-iii#00000#--.-i#00000##00011#-0#0000#-i011           
                 0#iii##000####000#0000000000#0000000110-0#0001#-i011           
                 i111100000##00000#0000##00110###0010110-000000i.i010           
                 .-#00i..-#ii-.iiii010##ii#00##i##00001-i###000ii011-           
                .    i. .iii..-ii##10#0# i#00##ii#0010#-.-i###0-i010            
               . ----ii###i--#i##0000#0#i#01##i-i#000#.     10ii#00             
               i---i#i#0#i--##--i#000-.#01111##--0010.     -10###0.             
                .-#.-i#0##i-#00##000#11111111110i-#0#-     i10000-              
                  ii#i0#i##0ii0##00##10##001111110####.    -11000               
                  -i##00#000##0000##0#.  i#01111111100#i.   #1010               
                   -##00#0000000###0#.   .i#0111111111110#-  i000- -i           
                  -i#000000####00110-    .-i#00111-.-#011110- .i#000-           
                  .#0##0000#0111110i.    .-i##0011#      ....                   
                            0111110i.    .-i#000111i                            
                            -11001#i--...--i##ii###0#--.                        
                             .#010##iiiiii--..-ii##000010-                      
                              -00#-ii#0#-..-iii#####0000010i                    
                              -iii#####iiii...-i###000011111#.                  
                            .---i#0###iii##0110#i--..                           
                           ---#i010####010#-.                                   
                          i#00#011100#-.                                        
                         .#00011#i-                                             
                           --..                                                 


![IMPORTANT](https://img.shields.io/badge/%20after%20compiling-use%20this%20command%20to%20make%20scipio%20accessible%20from%20any%20directory-red)

    $sudo ln -s $(pwd)/scipio /usr/local/bin/scipio

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
