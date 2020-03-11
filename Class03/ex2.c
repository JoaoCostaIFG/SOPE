#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

int main(void) {
  // 123
  // 45
  /*
   * write(STDOUT_FILENO, "1", 1);
   * if (fork() > 0) {
   *   write(STDOUT_FILENO, "2", 1);
   *   write(STDOUT_FILENO, "3", 1);
   * } else {
   *   write(STDOUT_FILENO, "4", 1);
   *   write(STDOUT_FILENO, "5", 1);
   * }
   * write(STDOUT_FILENO, "\n", 1);
   */

  // 123
  // 145
  // fork copia buffer do printf do parent
  /*
   * printf("1");
   * if (fork() > 0) {
   *   printf("2");
   *   printf("3");
   * } else {
   *   printf("4");
   *   printf("5");
   * }
   * printf("\n");
   */

  // 1
  // 23
  // 45
  // '\n' flushes printf buffer
  printf("1\n");
  if (fork() > 0) {
    printf("2");
    printf("3");
  } else {
    printf("4");
    printf("5");
  }
  printf("\n");
  return 0;
}
