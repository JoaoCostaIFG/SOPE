#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_NAME_LEN 1000

int main(int argc, char **argv) {
  int dest_fd;
  if ((dest_fd = open(argv[1], O_WRONLY | O_CREAT | O_APPEND | O_EXCL, 0644)) ==
      -1) {
    fprintf(stderr, "Fuck off.\n");
    exit(1);
  }

  printf("How many entries to read? ");
  int n;
  scanf("%d", &n);
  getc(stdin);

  char nome[MAX_NAME_LEN + 1], temp[33];
  int classfi;
  for (int i = 0; i < n; ++i) {
    fgets(nome, MAX_NAME_LEN, stdin);
    nome[strlen(nome) - 1] = 0;
    scanf("%d", &classfi);
    getc(stdin);

    sprintf(temp, "%d", classfi);
    /* printf("nome:%s classfi:%.2f\n", nome, classfi); */

    write(dest_fd, nome, strlen(nome));
    write(dest_fd, " ", 1);
    write(dest_fd, &temp, strlen(temp));
    write(dest_fd, "\n", 1);
  }

  close(dest_fd);
  return 0;
}
