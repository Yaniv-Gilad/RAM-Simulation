RAM Simulation

The software simulates reading and writing to virtual memory using the page table principle.
The virtual memory is at size 100 (can be changed).
When thee virtual memory get full, it start to write to the swap file if it is necessary.
 

functions:
1. load(address) - load the address to the main memory and return the value, return '\n' if there is an error.
2. store(address, value) - stored the given value in the address.
3. print_memory - print the main memory.
4. print_swap - print the swap file.
5. print_page_table - print the page table.


How to compile:
In VSCode ctrl+shift+b.
In linux: g++ sim_mem.cpp main.cpp -o main.

How to run:
In VSCode ctrl+f5.
In linux: open the terminal in the folder and write "./main" and press enter.

Program files:
makefile
sim_mem.h // header file
sim_mem.cpp // implementaion of the header file
main.cpp // main
main    // Executable file
