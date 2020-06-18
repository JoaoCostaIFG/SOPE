#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

pthread_cond_t cond1 = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond2 = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond3 = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mut1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mut2 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mut3 = PTHREAD_MUTEX_INITIALIZER;

sem_t sem1, sem2;

unsigned short int aposta_feita = 0;
unsigned short int numero_sorteado = 0;
int aposta = 0;
int face = 0;
int numero_jogadas = 0;
int apostas_ganhas = 0;
int apostas_perdidas = 0;

void *t_aposta(void *arg) {
  while (1) {
    scanf("%d", &aposta);
    aposta_feita = 1;

    pthread_cond_signal(&cond3);

    pthread_mutex_lock(&mut1);
    while (aposta_feita)
      pthread_cond_wait(&cond1, &mut1);
    pthread_mutex_unlock(&mut1);
  }
}

void *t_roda(void *arg) {
  while (1) {
    face = rand() % 6 + 1;
    numero_sorteado = 1;

    pthread_cond_signal(&cond3);

    pthread_mutex_lock(&mut2);
    while (numero_sorteado)
      pthread_cond_wait(&cond2, &mut2);
    pthread_mutex_unlock(&mut2);
  }
}

void *t_compara(void *arg) {
  while (1) {
    pthread_mutex_lock(&mut3);
    while (!aposta_feita || !numero_sorteado)
      pthread_cond_wait(&cond3, &mut3);
    pthread_mutex_unlock(&mut3);

    sem_wait(&sem1);
    ++numero_jogadas;
    if (face == aposta)
      ++apostas_ganhas;
    else
      ++apostas_perdidas;
    sem_post(&sem2);

    aposta_feita = 0;
    pthread_cond_signal(&cond1);
    numero_sorteado = 0;
    pthread_cond_signal(&cond2);
  }
}

void *t_mostra(void *arg) {
  while (1) {
    sem_wait(&sem2);
    printf("jogadas: %d\tganhas: %d\tperdidas: %d\n", numero_jogadas,
           apostas_ganhas, apostas_perdidas);
    sem_post(&sem1);
    fflush(stdout);

    sleep(1);
  }
}

int main(int argc, char *argv[]) {
  pthread_t aposta, roda, compara, mostra;

  sem_init(&sem1, 0, 1);
  sem_init(&sem2, 0, 0);

  pthread_create(&aposta, NULL, &t_aposta, NULL);
  pthread_create(&roda, NULL, &t_roda, NULL);
  pthread_create(&compara, NULL, &t_compara, NULL);
  pthread_create(&mostra, NULL, &t_mostra, NULL);

  pthread_exit(0);
}
