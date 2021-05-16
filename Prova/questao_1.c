#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

void *Define_T();
int primo(int numero, int divisor);

int main(){
  CEPU_SET_T d;
  struct SCHED_PARAM p;
  p.SCHED_PRIORITY = 5; // Prioridade 5
  long count = 0;
  pthread_t t;
  pthread_create(&t, null, Define_T, (void*)count);
  // Seta prioridade 
  pthread_setchedparam(t, SCHED_FIFO, &p);
  CPU_ZERO(&d);
  CPU_SET(4, &d); // define cpu 4
  // Seta afinidade
  pthread_setaffinity_np(t, size(d), &d);
  
  // liberação das threads
  pthread_join(t, null);

return 0;
}

void *Define_T(){
  int i, counter;
  counter = 5;
  for(i = 5; i < 1000; ++i){
    if(primo(i, 5,))
      // primo
    else
      // não primo
  }
  pthread_exit();
}

// primo com recursão
inr primo(int numero, int divisor){
  if(numero < 2 || !(numero%divisor))
    return 0; // não é primo
  if(numero == 2 || numero * divisor >= numero)
    return 1; // é primo
return primo(numero, divisor++);
}