#include <string.h>
#include <termios.h>
#include <unistd.h>

#define MAX_SIZE 1000

int main(void) {
  struct termios oldterm, newterm;

  write(STDOUT_FILENO, "password? ", 10);

  tcgetattr(STDIN_FILENO, &oldterm);
  newterm = oldterm;
  newterm.c_lflag &= ~(ECHO | ECHOE | ECHOK | ECHONL | ICANON);
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &newterm);

  // read stuff
  int i = 0;
  char pass[MAX_SIZE + 1], echo = '*';
  while (i < MAX_SIZE && read(STDIN_FILENO, &pass[i], 1) && pass[i] != '\n') {
    if (pass[i] == 127) {
      write(STDOUT_FILENO, "\b \b", 3);
      --i;
    } else {
      write(STDOUT_FILENO, &echo, 1);
      ++i;
    }
  }
  pass[i] = 0;

  tcsetattr(STDIN_FILENO, TCSANOW, &oldterm);
  write(STDOUT_FILENO, "\n", 1);
  write(STDOUT_FILENO, "password: ", 10);
  write(STDOUT_FILENO, pass, strlen(pass));
  write(STDOUT_FILENO, "\n", 1);

  return 0;
}
