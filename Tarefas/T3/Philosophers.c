#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#define N_FIL 5 
#define ESQ(x) (x)
#define DIR(x) ((x+ (N_FIL-1)) % N_FIL)

/*
	Victor Dallagnol Bento
*/

pthread_mutex_t g[N_FIL] = PTHREAD_MUTEX_INITIALIZER;

void *F(void *a){
     long id;
     int e, d;
     id = (long)a;
     e = ESQ(id);
     d = DIR(id);

     // PENSANDO
     printf("Filosofo #%ld: PENSANDO\n", id);
     usleep(1000000);
    
//====================================     Solução para o Deadlock   ==========================================================
/*     if (id == 0){
        // GARFO DIREITA
        pthread_mutex_lock(&g[d]);
        printf("Filosofo #%ld: GARFO_DIREITA #%d\n", id, d);
        usleep(1000000);
     }else{
          // GARFO ESQUERDA
          pthread_mutex_lock(&g[e]);
          printf("Filosofo #%ld: GARFO_ESQUERDA #%d\n", id, e);
          usleep(1000000);
     }*/
//=============================================================================================================================

//===========================     Para resolver deadlock o trecho deve ser removido   =========================================
     // GARFO DIREITA
     pthread_mutex_lock(&g[d]);
     printf("Filosofo #%ld: GARFO_DIREITA #%d\n", id, d);
     usleep(1000000);

     // GARFO ESQUERDA
     pthread_mutex_lock(&g[e]);
     printf("Filosofo #%ld: GARFO_ESQUERDA #%d\n", id, e);
     usleep(1000000);
//=============================================================================================================================

     // COMENDO
     printf("Filosofo #%ld: COMENDO\n", id);
     usleep(1000000);
     
     // LIBERA GARFO ESQUERDA
     pthread_mutex_unlock(&g[e]);
     printf("Filosofo #%ld: GARFO_DIREITA #%d\n", id, e);
     usleep(1000000);

     // LIBERA GARFO DIREITA
     pthread_mutex_unlock(&g[d]);
     printf("Filosofo #%ld: GARFO_ESQUERDA #%d\n", id, d);
     usleep(1000000);
     
     pthread_exit(NULL);
}

int main(){
     pthread_t threads[N_FIL];     // Definição das threads
     long count;

     for(count = 0; count < N_FIL; count++){     // Criação das threads
          if (pthread_create(&threads[count], NULL, F, (void *)count)){
               printf("<!> ERROR na criação da Thread");
               exit(-1);
          }
     }

     for(count = 0; count < N_FIL; count++){     // Criação das threads
          pthread_join(threads[count], NULL);
     }
     
     return 0;
}
