#include <queue>
#include <deque>

#ifndef MyClass
#define MyClass
#define MEMORY_SIZE 100
extern char main_memory[MEMORY_SIZE];
using namespace std;

class sim_mem
{
    typedef struct page_descriptor
    {
        unsigned int V;     // valid
        unsigned int D;     // dirty
        unsigned int P;     // permission
        unsigned int frame; //the number of a frame if in case it is page-mapped
    } page_descriptor;

    int swapfile_fd; //swap file fd
    int program_fd;  //executable file fd
    int text_size;
    int data_size;
    int bss_size;
    int heap_stack_size;
    int num_of_pages;
    int page_size;
    page_descriptor *page_table; //pointer to page table

    bool *frames;
    queue<int> page_order;
    int get_free_frame();
    int override_frame();

public:
    sim_mem(char exe_file_name[], char swap_file_name[], int text_size,
            int data_size, int bss_size, int heap_stack_size, int num_of_pages, int page_size);

    ~sim_mem();
    char load(int address);
    void store(int address, char value);
    void print_memory();
    void print_swap();
    void print_page_table();
};
#endif
