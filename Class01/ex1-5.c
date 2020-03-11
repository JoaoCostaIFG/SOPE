#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[], char *envp[])
{
  if (argc <= 1) {
    printf("no args\n");
    return 1;
  }

  if (!strcmp("USER", argv[1])) {
    int i = 0;
    while (strncmp("USER=", envp[i], 4)) {
      ++i;
    }
    puts(envp[i]);

    puts(getenv("USER"));
    return 2;
  }

  return 0;
}
