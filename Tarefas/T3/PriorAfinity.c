#define _GNU_SOURCE
#include <sched.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/*
	Victor Dallagnol Bento
*/

#define N_THREADS  5
#define SEGUNDOS 2

void *Two_Seconds(void *arg);

int main() {
    pthread_t thread[N_THREADS];
    cpu_set_t cpu_set[N_THREADS];
    struct sched_param param[N_THREADS];
    int t, aux, procs, escalonador, prio_min, prio_max;
    char ch;
//==================    Escalonador
    
    printf("Selecionar o escalonador:\n");
    printf("%d: Default\n%d: FIFO\n%d: RR:\n\n", SCHED_OTHER, SCHED_FIFO, SCHED_RR);
    printf("Numero: ");
    scanf("%d", &escalonador);
    
    switch(escalonador){
        case SCHED_FIFO:
            printf("[FIFO] - Prioridade minima: %d\n", prio_min = sched_get_priority_min(SCHED_FIFO));
            printf("[FIFO] - Prioridade maxima: %d\n\n", prio_max = sched_get_priority_max(SCHED_FIFO));
            break;
        
        case SCHED_RR:
            printf("[RR] - Prioridade minima: %d\n", prio_min = sched_get_priority_min(SCHED_RR));
            printf("[RR] - Prioridade maxima: %d\n\n", prio_max = sched_get_priority_max(SCHED_RR));
            break;
        
        case SCHED_OTHER:
            printf("[DAFAULT] - Prioridade minima: %d\n", prio_min = sched_get_priority_min(SCHED_OTHER));
            printf("[DAFAULT] - Prioridade maxima: %d\n\n", prio_max = sched_get_priority_max(SCHED_OTHER));
            break;
        
        default:
            printf("\nEscalonador invalido!\n\n");
            return 0;
    }
    
    
//============================== Prioridades
    printf("Selecionar prioridade:\n");
    
    for(t = 0; t < N_THREADS; t++){
        printf("Thread [%d]: ", t);
        
        while(1){
            scanf("%d", &aux);
            if(aux >= prio_min && aux <= prio_max){
                param[t].sched_priority = aux;
                break;
            }
            else{
                printf("Prioridade invalida!\n");
            }
        }
    }

    printf("\n===============================\n");
    procs = sysconf(_SC_NPROCESSORS_ONLN);
    printf("Processadores disponiveis = %d\n", procs);
    
//================ Processadores
    for(t=0; t<N_THREADS; t++){
        CPU_ZERO(&cpu_set[t]);

        printf("\nSelecionar Processadores entre 0-%d (numeros != finalizam)\n", procs);
        printf("\nSelecionar processadores para a thread %d:\n", t);
   
        do{
            scanf("%d", &aux);
            
            if(CPU_ISSET(aux, &cpu_set[t]))
                printf("Processador ocupado!\n");
            else if(aux >= 0 && aux < procs)
                CPU_SET(aux, &cpu_set[t]);
                
        } while(aux >= 0 && aux < procs);
        
        printf("\nProcessadores selecionados para a thread %d: ", t);
        
        for(aux=0; aux<procs; aux++)
            if(CPU_ISSET(aux, &cpu_set[t]))
                printf("%d ", aux);
    }
    
    printf("\n\n");
    
    // Criação das Threads
    for(t = 0; t < N_THREADS; t++){
        aux = pthread_create(&thread[t], NULL, Two_Seconds, (void *)t);
        if(aux){
            printf("ERROR; return code from pthread_create() is %d\n", aux);
            exit(-1);
        }
        else{
            printf("Thread [%d]: Executando.\n", t);
            pthread_setschedparam(thread[t], escalonador, &param[t]);
            pthread_setaffinity_np(thread[t], sizeof(cpu_set), &cpu_set[t]);
        }
    }
    
    // Finaliza Threads
    for(t = 0; t < N_THREADS; t++)
    {
        pthread_join(thread[t], NULL);
    }
    printf("\n");
    
    return 0;
}


// Função para os dois segundos
void *Two_Seconds(void *arg) {
    int tid, aux = 0;
    tid = (int)arg;

    struct timespec _time;        // Estrutura para setar tempo
    
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &_time);     // Informa o tempo de CPU da thread (CLOCK_THREAD_CPUTIME_ID)

    printf("<Antes de executar>\tThread [%d]: %ld segundos e %ld nanosegundos!\n", tid, _time.tv_sec, _time.tv_nsec);
    
    // Executa thread por SEGUNDOS(2)
    while (_time.tv_sec < SEGUNDOS){
        clock_gettime(CLOCK_THREAD_CPUTIME_ID, &_time);
        
        if(_time.tv_sec > aux){
            printf("<Parcial>\tThread [%d]: %ld segundos e %ld nanosegundos!\n", tid, _time.tv_sec, _time.tv_nsec);
            aux++;
        }
    }
    
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &_time);
    printf("<Apos executar>\tThread [%d]: %ld segundos e %ld nanosegundos!\n", tid, _time.tv_sec, _time.tv_nsec);
    pthread_exit(NULL);
}