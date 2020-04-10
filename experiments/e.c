#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#define stupidenv "stupidname"

int main(int argc, char **argv) {
  /*
   * The child process shoulkd exec itself and write an integer to the parent
   * process.
   * It works by assigning the file descriptor 0 (usually stdin) to the pipe
   * writting end.
   * This only causes 1 problem: it breaks the standard input file pointer. This
   * file pointer is input only so this is technically bad coding. Still, it
   * works for this purpose.
   */

  int res = 0;
  if (getenv(stupidenv) != NULL) { // exec'ed
    res = 123456789;
    fprintf(stderr, "SOU CHILD: %d\n", res);

    /* important part (notice that I'm writting to "stdin") */
    write(STDIN_FILENO, &res, sizeof(int));

    /* the line bellow won't work, because the stream is broken */
    // fprintf(stdin, "%d", res);
    /* the line bellow would work if we did: dup2(fd[1], STDOUT_FILENO) */
    // printf("%d", res);

    return 0;
  } else
    setenv(stupidenv, stupidenv, 1);

  int fd[2];
  pipe(fd);
  if (!fork()) { // child
    close(fd[0]);
    dup2(fd[1], STDIN_FILENO);
    close(fd[1]);

    execv(argv[0], argv);
  } else { // parent
    close(fd[1]);

    // attempt to read an integer
    int read_res = read(fd[0], &res, sizeof(int));
    if (read_res == 0)
      fprintf(stderr, "PAI EOF\n");
    else if (read_res < 0)
      perror("PAI READ ERROR");

    close(fd[0]);
    fprintf(stderr, "SOU PAI: %d\n", res);

    wait(NULL);
  }

  return 0;
}
