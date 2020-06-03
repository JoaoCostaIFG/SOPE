/** @file progprop_interface.h */
#ifndef PROGPROP_INTERFACE_H
#define PROGPROP_INTERFACE_H

/** options */
struct prog_prop {
  long nsecs;
  long nplaces;
  long nthreads;
  char fifoname[256];
};
typedef struct prog_prop prog_prop;

#endif // PROGPROP_INTERFACE_H
