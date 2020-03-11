#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define BUFFER_SIZE 10000

int main(int argc, char *argv[])
{
  char buf[BUFFER_SIZE] = "";
  fgets(buf, BUFFER_SIZE, stdin);
  printf("Line read: %s", buf);

  char *ptr = strtok(buf, " ");
  while (ptr) {
    printf("%s\n", ptr);
    ptr = strtok(NULL, " \n");
  }

  return 0;
}
