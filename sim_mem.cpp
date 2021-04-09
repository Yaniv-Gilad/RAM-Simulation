#include "sim_mem.h"
#include <assert.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <iostream>
#include <string.h>
#include <queue>

using namespace std;
char main_memory[MEMORY_SIZE];

sim_mem::sim_mem(char exe_file_name[], char swap_file_name[], int text_size,
                 int data_size, int bss_size, int heap_stack_size, int num_of_pages, int page_size)
{
    if (exe_file_name == NULL || swap_file_name == NULL)
    {
        perror("Error in open: exe or swap is NULL\n");
        exit(1);
    }
    swapfile_fd = open(swap_file_name, O_RDWR | O_CREAT | O_APPEND | O_TRUNC, S_IRWXU);
    program_fd = open(exe_file_name, O_RDWR | O_APPEND, S_IRWXU);
    if (program_fd < 0 || swapfile_fd < 0)
    {
        perror("Error in open\n");
        exit(1);
    }
    this->text_size = text_size;
    this->data_size = data_size;
    this->bss_size = bss_size;
    this->heap_stack_size = heap_stack_size;
    this->num_of_pages = num_of_pages;
    this->page_size = page_size;

    this->page_table = (page_descriptor *)malloc(sizeof(page_descriptor) * num_of_pages);
    assert(page_table != NULL);

    this->frames = (bool *)malloc(sizeof(bool) * (MEMORY_SIZE / page_size));
    assert(frames != NULL);

    // initiate main memory
    for (int i = 0; i < MEMORY_SIZE; i++)
        main_memory[i] = '0';

    // initiate page table
    for (int i = 0; i < num_of_pages; i++)
    {
        page_table[i].frame = -1;
        page_table[i].V = 0;
        page_table[i].D = 0;

        if (i < (text_size / page_size)) // read
            page_table[i].P = 0;
        else
            page_table[i].P = 1; // write
    }

    // initiate swap file
    for (int i = 0; i < (page_size * num_of_pages); i++)
    {
        int check = write(swapfile_fd, "0", 1);
        if (check == -1) // error
        {
            perror("write error\n");
            free(page_table);
            free(frames);
            close(swapfile_fd);
            close(program_fd);
            exit(1);
        }
    }

    // initiate free_frame
    for (int i = 0; i < MEMORY_SIZE / page_size; i++)
        frames[i] = true;
}

char sim_mem::load(int address)
{
    if (address >= page_size * num_of_pages || address < 0) // address not valid
    {
        printf("address %d is not in memory range\n", address);
        return '\0';
    }

    int page = address / page_size;
    int offset = address % page_size;

    if (page_table[page].V == 1) //  in memory
    {
        int physical_add = page_table[page].frame * page_size + offset;
        return main_memory[physical_add];
    }

    // not in memory

    // from exe
    if (page_table[page].P == 0 || (page_table[page].P == 1 && page_table[page].D == 0 && address < text_size + data_size))
    {
        int frame = this->get_free_frame();
        frames[frame] = false; // frame not empty
        page_order.push(page);

        page_table[page].V = 1;
        page_table[page].frame = frame;

        // copy from exe to buf
        char *buf = (char *)malloc(sizeof(char) * (page_size + 1));
        assert(buf != NULL);
        lseek(program_fd, page * page_size, SEEK_SET);
        if (read(program_fd, buf, page_size) < 0)
        {
            // read error
            perror("Error in load: read error\n");
            free(buf);
            return '\0';
        }

        // copy from buf to main memory
        int physical_add = frame * page_size;
        for (int i = 0; i < page_size; i++)
        {
            main_memory[physical_add + i] = buf[i];
        }

        free(buf);
        return main_memory[frame * page_size + offset];
    }

    // from swap
    else if (page_table[page].P == 1 && page_table[page].D == 1)
    {
        int frame = this->get_free_frame();
        frames[frame] = false; // frame not empty
        page_order.push(page);

        page_table[page].V = 1;
        page_table[page].frame = frame;

        // copy from swap to buf
        char *buf = (char *)malloc(sizeof(char) * (page_size + 1));
        assert(buf != NULL);
        lseek(swapfile_fd, page * page_size, SEEK_SET);
        if (read(swapfile_fd, buf, page_size) < 0)
        {
            // read error
            perror("Error in load: read error\n");
            free(buf);
            return '\0';
        }

        // copy from buf to main memory
        int physical_add = frame * page_size;
        for (int i = 0; i < page_size; i++)
        {
            main_memory[physical_add + i] = buf[i];
        }

        free(buf);
        return main_memory[frame * page_size + offset];
    }

    // bss - init page to zero
    else if (page_table[page].P == 1 && page_table[page].D == 0 && address < text_size + data_size + bss_size)
    {
        int frame = this->get_free_frame();
        frames[frame] = false; // frame not empty
        page_order.push(page);

        page_table[page].V = 1;
        page_table[page].frame = frame;

        int physical_add = frame * page_size;
        for (int i = 0; i < page_size; i++)
        {
            main_memory[physical_add + i] = '0';
        }
        return main_memory[physical_add + offset];
    }

    // error heap or stack
    printf("can't init new page from load function if it is from heap or stack area\n");
    return '\0';
}

void sim_mem::store(int address, char value)
{
    int page = address / page_size;
    int offset = address % page_size;

    // address not valid
    if (address >= page_size * num_of_pages || address < 0)
    {
        printf("address %d is not in memory range\n", address);
        return;
    }

    // if the is not premission
    if (page_table[page].P == 0)
    {
        printf("there is not permision to store in text area\n");
        return;
    }

    //  in memory
    if (page_table[page].V == 1)
    {
        int physical_add = page_table[page].frame * page_size + offset;
        main_memory[physical_add] = value;
        page_table[page].D = 1;
        return;
    }

    // not in memory

    // from exe
    if (page_table[page].P == 1 && page_table[page].D == 0 && address < text_size + data_size)
    {
        int frame = this->get_free_frame();
        frames[frame] = false; // frame not empty
        page_order.push(page);

        page_table[page].V = 1;
        page_table[page].frame = frame;

        // copy from exe to buf
        char *buf = (char *)malloc(sizeof(char) * (page_size + 1));
        assert(buf != NULL);
        lseek(program_fd, page * page_size, SEEK_SET);
        if (read(program_fd, buf, page_size) < 0)
        {
            // read error
            perror("Error in store: read error\n");
            free(buf);
            return;
        }

        // copy from buf to main memory
        int physical_add = frame * page_size;
        for (int i = 0; i < page_size; i++)
        {
            main_memory[physical_add + i] = buf[i];
        }

        free(buf);
        main_memory[physical_add + offset] = value;
        page_table[page].D = 1;
        return;
    }

    // from swap
    else if (page_table[page].P == 1 && page_table[page].D == 1)
    {
        int frame = this->get_free_frame();
        frames[frame] = false; // frame not empty
        page_order.push(page);

        page_table[page].V = 1;
        page_table[page].frame = frame;

        // copy from swap to buf
        char *buf = (char *)malloc(sizeof(char) * (page_size + 1));
        assert(buf != NULL);
        lseek(swapfile_fd, page * page_size, SEEK_SET);
        if (read(swapfile_fd, buf, page_size) < 0)
        {
            // read error
            perror("Error in store: read error\n");
            free(buf);
            return;
        }

        // copy from buf to main memory
        int physical_add = frame * page_size;
        for (int i = 0; i < page_size; i++)
        {
            main_memory[physical_add + i] = buf[i];
        }

        free(buf);
        main_memory[physical_add + offset] = value;
        page_table[page].D = 1;
        return;
    }

    // init new page
    else if (page_table[page].P == 1 && page_table[page].D == 0 && address >= text_size + data_size)
    {
        int frame = this->get_free_frame();
        frames[frame] = false; // frame not empty
        page_order.push(page);

        page_table[page].V = 1;
        page_table[page].frame = frame;

        int physical_add = frame * page_size;
        for (int i = 0; i < page_size; i++)
        {
            main_memory[physical_add + i] = '0';
        }

        main_memory[physical_add + offset] = value;
        page_table[page].D = 1;
        return;
    }
}

void sim_mem::print_memory()
{
    int i;
    printf("\n Physical memory\n");
    for (i = 0; i < MEMORY_SIZE; i++)
    {
        printf("[%c]\n", main_memory[i]);
    }
}

void sim_mem::print_swap()
{
    char *str = (char *)malloc(this->page_size * sizeof(char));
    assert(str != NULL);
    int i;
    printf("\n Swap memory\n");
    lseek(swapfile_fd, 0, SEEK_SET); // go to the start of the file
    while (read(swapfile_fd, str, this->page_size) == this->page_size)
    {
        for (i = 0; i < page_size; i++)
        {
            printf("%d - [%c]\t", i, str[i]);
        }
        printf("\n");
    }
    free(str);
}

void sim_mem::print_page_table()
{
    int i;
    printf("\n page table \n");
    printf("Valid\t Dirty\t Permission \t Frame\n");
    for (i = 0; i < num_of_pages; i++)
    {
        printf("[%d]\t[%d]\t[%d]\t[%d]\n",
               page_table[i].V,
               page_table[i].D,
               page_table[i].P,
               page_table[i].frame);
    }
}

sim_mem::~sim_mem()
{
    free(page_table);
    free(frames);
    close(swapfile_fd);
    close(program_fd);
}

// private functions //

int sim_mem::get_free_frame()
{
    for (int i = 0; i < MEMORY_SIZE / page_size; i++)
    {
        if (frames[i] == true)
        {
            frames[i] = false;
            return i;
        }
    }

    //  if here need to override the oldest frame
    int frame = this->override_frame();
    return frame;
}

int sim_mem::override_frame()
{
    int page = page_order.front();
    page_table[page].V = 0;

    if (page_table[page].D == 1) // if the pop page is dirty need to transfer to swap
    {
        // copy the memory to buf
        int check;
        int physical_add = page_table[page].frame * page_size;
        char *buf;
        buf = (char *)malloc(sizeof(char) * (page_size * num_of_pages + 1));
        assert(buf != NULL);
        char *buf2 = (char *)malloc(sizeof(char) * (page_size * (num_of_pages - page - 1)));
        assert(buf2 != NULL);

        // copy from swap
        lseek(swapfile_fd, 0, SEEK_SET);
        check = read(swapfile_fd, buf, page * page_size);
        if (check < 0)
        {
            perror("Error in override_frame: read error");
        }

        // copy from main memory
        for (int i = 0; i < page_size; i++)
        {
            buf[page * page_size + i] = main_memory[physical_add + i];
        }

        // copy from the rest of swap
        lseek(swapfile_fd, (page + 1) * page_size, SEEK_SET);
        check = read(swapfile_fd, buf2, (num_of_pages - page - 1) * page_size);
        if (check < 0)
        {
            perror("Error in override_frame: read error");
        }

        for (int i = 0; i < (num_of_pages - page - 1) * page_size; i++)
        {
            buf[(page + 1) * page_size + i] = buf2[i];
        }

        // to swap file the correct value
        ftruncate(swapfile_fd, 0);
        check = write(swapfile_fd, buf, page_size * num_of_pages);
        if (check < 0)
        {
            perror("Error in override_frame: write error");
        }

        free(buf);
        free(buf2);
    }

    int frame = page_table[page].frame;
    page_order.pop();
    return frame;
}
