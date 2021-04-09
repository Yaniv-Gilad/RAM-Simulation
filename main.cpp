#include <iostream>
#include <unistd.h>
#include "sim_mem.h"
using namespace std;

int main()
{
  char val, val1, val2;
  sim_mem mem_sm((char *)"exe", (char *)"swap", 35, 40, 25, 25, 25, 5);
  val = mem_sm.load(0);
  val = mem_sm.load(5);
  val1 = mem_sm.load(10);
  val2 = mem_sm.load(15);
  val2 = mem_sm.load(20);
  val2 = mem_sm.load(25);
  val2 = mem_sm.load(30);
  val2 = mem_sm.load(35);
  val2 = mem_sm.load(40);
  val2 = mem_sm.load(45);
  val2 = mem_sm.load(50);
  val2 = mem_sm.load(55);
  val2 = mem_sm.load(60);
  val2 = mem_sm.load(65);
  val2 = mem_sm.load(70);
  val2 = mem_sm.load(100);
  mem_sm.store(0, 'z');
  mem_sm.store(75, 'p');
  mem_sm.store(80, 'q');
  mem_sm.store(85, 'r');
  mem_sm.store(90, 's');
  mem_sm.store(95, 't');
  mem_sm.store(100, 'u');
  mem_sm.store(105, 'v');
  mem_sm.store(110, 'w');
  mem_sm.store(115, 'x');
  mem_sm.store(120, 'y');
  mem_sm.store(91, 'y');
  val = mem_sm.load(0);
  val = mem_sm.load(5);
  val1 = mem_sm.load(10);
  val2 = mem_sm.load(15);
  val2 = mem_sm.load(20);
  val2 = mem_sm.load(25);
  val2 = mem_sm.load(30);
  val2 = mem_sm.load(35);
  val2 = mem_sm.load(40);
  val2 = mem_sm.load(45);
  val2 = mem_sm.load(50);
  mem_sm.store(37, '*');
  printf("%c\n%c\n%c\n", val, val1, val2);
  mem_sm.print_memory();
  mem_sm.print_page_table();
  mem_sm.print_swap();

  return 0;
}

// int main()
// {
//   char c;
//   sim_mem mem_sm((char *)"exe", (char *)"swap", 25, 50, 25, 25, 25, 5);

//   printf("write to memory 'X' and 'Y'\n");
//   for (int i = 30; i < 125; i += 2)
//   {
//     printf("i value is: %d\n", i);
//     mem_sm.store(i, 'X');
//     c = mem_sm.load(i);
//     printf("%c\n", c);

//     printf("i value is: %d\n", i + 1);
//     mem_sm.store(i + 1, 'Y');
//     c = mem_sm.load(i + 1);
//     printf("%c\n", c);
//   }

//   printf("load from memory\n");
//   for (int i = 0; i < 30; i++)
//   {
//     printf("i value is: %d\n", i);
//     c = mem_sm.load(i);
//     printf("%c\n", c);
//     printf("\n");
//   }

//   printf("write to memory 'A' and 'B'\n");
//   for (int i = 30; i < 125; i += 2)
//   {
//     printf("i value is: %d\n", i);
//     mem_sm.store(i, 'A');
//     c = mem_sm.load(i);
//     printf("%c\n", c);

//     printf("i value is: %d\n", i + 1);
//     mem_sm.store(i + 1, 'B');
//     c = mem_sm.load(i + 1);
//     printf("%c\n", c);
//   }

//   printf("load from memory\n");
//   for (int i = 0; i < 125; i++)
//   {
//     printf("i value is: %d\n", i);
//     c = mem_sm.load(i);
//     printf("%c\n", c);
//     printf("\n");
//   }

//   mem_sm.print_page_table();
//   mem_sm.print_memory();
//   mem_sm.print_swap();

//   return 0;
// }