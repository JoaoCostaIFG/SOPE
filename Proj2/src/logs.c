#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "include/logs.h"

void
write_log(char* action, packet* p)
{
  /*
  “inst ; i ; pid ; tid ; dur ; pl ; oper”
  - i , pid , tid , dur , pl – têm o mesmo significado que os
    campos apresentados acima.
  - inst - valor retornado pela chamada ao sistema time(),
    na altura da produção da linha.
  - oper – siglas de 5 letras ajustadas às fases da operação que cada
    processo/thread acabou de executar e que variam conforme se trate do
    cliente ou do servidor.
  */
  printf("%ld ; %d ; %d ; %lu ; %ld ; %d ; %s\n",
         time(NULL),
         p->i,
         p->pid,
         p->tid,
         p->dur,
         p->pl,
         action);
}
