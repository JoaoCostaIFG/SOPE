#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  int fd[2];
  pipe(fd);
  pipe(fd);
  fork();
  fprintf(stderr, "PID: %d\n", getpid());
  sleep(10000);

  char proc_path[50];
  sprintf(proc_path, "/proc/%d/fd/", getpid());
  char path[100];

  char *env_result = NULL;

  char fd_info[PIPE_BUF + 1];
  int fd_info_len;
  struct dirent *direntp;
  struct stat stat_buf;
  int fd_r;
  DIR *dirp = opendir(proc_path);

  int i = 0;
  while ((direntp = readdir(dirp))) {
    if (!strcmp(direntp->d_name, ".") || !strcmp(direntp->d_name, ".."))
      continue;

    /* evaluate type */
    sprintf(path, "%s/%s", proc_path, direntp->d_name);
    stat(path, &stat_buf);
    if (S_ISFIFO(stat_buf.st_mode)) {
      fd_r = atoi(direntp->d_name); // get file descriptor
      fprintf(stderr, "%d - %s - %d\n", i, direntp->d_name, fd_r); // TODO
      /* skip stdin, stdout and stderr (we don't want them) */
      if (fd_r == STDIN_FILENO || fd_r == STDOUT_FILENO ||
          fd_r == STDERR_FILENO)
        continue;

      if ((fd_info_len = readlink(path, fd_info, PIPE_BUF)) == -1) {
        fprintf(stderr, "readlink failed (broken symlink)");
        continue;
      }
      fd_info[fd_info_len] = '\0';

      /* alloc memory */
      if (env_result == NULL) {
        env_result = (char *)malloc(sizeof(char) * (strlen(fd_info) + 2));
        strcpy(env_result, fd_info);
        env_result[fd_info_len] = '\n';
        env_result[fd_info_len + 1] = '\0';
      } else {
        env_result = realloc(env_result, sizeof(char) * (strlen(env_result) +
                                                         strlen(fd_info) + 1));
        strcat(env_result, fd_info);

        int k = strlen(env_result);
        env_result[k] = '\n';
        env_result[k + 1] = '\0';
      }
    }
    ++i;
  }

  if (env_result == NULL)
    env_result = "\n";
  fprintf(stderr, "END:\n%s\n", env_result);

  /*
   * char cmp[500];
   * // [> tokenize '\n' <]
   * for (int k = 0; k < strlen(env_result); ++k) {
   *   if (env_result[k] == '\n')
   *     cmp[k] = '\0';
   *   else
   *     cmp[k] = env_result[k];
   * }
   */
  char* cmp = "abc";
  fprintf(stderr, "Search:\n%s\n", cmp);

  char *abc = strstr(env_result, cmp);
  if (abc != NULL)
    fprintf(stderr, "found\n");
  else
    fprintf(stderr, "not found\n");

  sleep(1000);
  return 0;
}
