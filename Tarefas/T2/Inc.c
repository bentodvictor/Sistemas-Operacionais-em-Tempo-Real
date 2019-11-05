#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#define NUM_THREADS 10

/*
    O código não esta sincronizado. Por não possuir semáforo/mutex (binário), as threads
    não respeitam a seção crítica uma das outras o que pode acarretar em um incremento
    errado da variável global (var_global).
*/

int var_global = 0;     // Variável global para incremento

void *Inc(void *threadid) {
  long tid;

  tid = (long)threadid;
  var_global++;         // Cada Thread incrementa a variável global
  printf("Thread #%ld - var_global #%d!\n", tid, var_global);
  pthread_exit(NULL);
}

int main (int argc, char *argv[]) {
  pthread_t threads[NUM_THREADS];     // Definição das threads
  long count;

  for(count = 0; count < NUM_THREADS; count++){     // Criação das threads
        if (pthread_create(&threads[count], NULL, Inc, (void *)count)){
          printf("<!> ERROR na criação da Thread");
          exit(-1);
        }
  }
  pthread_exit(NULL);
}
