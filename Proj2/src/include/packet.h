#ifndef PACKET_H
#define PACKET_H

#include <sys/types.h>

#define EMPTY_FIELD -1
#define FIFO_DIR "/tmp"

struct packet {
  int i;
  pid_t pid;
  pthread_t tid;
  long dur;
  int pl;
};
typedef struct packet packet;

#endif // PACKET_H
