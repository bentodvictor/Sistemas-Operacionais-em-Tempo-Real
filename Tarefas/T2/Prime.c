#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#define N 20

typedef struct Limites{
  long id;           // ID da Thread
  long sup, inf;    // Limite superior e inferior da Thread
}limites;

long vec_prime[N];    // Vetor para salvar numeros primos
long nprimo = 0;      // variável para controle do tamanho do Vetor

void *Primo(void *t){
  // Calculo do numero primo e print da thread que calculou os numero
  // no determinado intervalo

  limites *td = (limites *)t;
  printf("Thread: %ld @ Intervalo: %ld - %ld\n", td->id, td->inf, td->sup);
  long count = 2;
  long control = 0;             // Controla o print da inexistencia de numeros primos no intervalo

  while (td->inf <= td->sup)                     // Equanto o limite inferior for menor ou igual ao limite superior, executa cálculo
    {
        while (count < td->inf)                        // cotador sempre vai de 2 até o numero desejado
        {
            if (td->inf == 0 || td->inf == 1)     // 0 e 1 sempre não Primos
            {
              break;
            }
            else if(td->inf % count == 0)        // Se resto = 0, numero não é primo
            {
              break;
            }else
            {
              count++;
            }
        }
      if(td->inf == count)
      {         // Testa novamente, sendo ou não primo o laço while é determinado

        vec_prime[nprimo] = td->inf;
        printf("#%ld é primo.\n", td->inf);
        td->inf++;
        count = 2;
        control++;
        nprimo++;
      }else
      {      // Se não for primo, testa número seguinte e contador = 2
      td->inf++;
      count = 2;
      }
	}
  if(control == 0)
  {      // Variável criada para controle, caso não exista numeros primos no intervalo
    printf("Primos nao encontrados!\n");
    control = 0;
  }
  printf("\n");
}


int main (int argc, char *argv[]) {
  int numeroT;

  printf("\n<!>\tInforme a quantidade de threads: ");
  scanf("%d", &numeroT);
  printf("===========================================\n\n");

  pthread_t threads[numeroT];     // Criação das threads
  long count, CadaThread;
  long save[2*numeroT];          // Vetor que armazena o limite superior de inferior de cada thread
  CadaThread = N/numeroT;        // Parcela que cada Thread vai calcular
  limites *l;

  for(count = 0; count < numeroT; count++){
    l = malloc(sizeof(limites));             // Alocação de memória para cada thread
    if (l == NULL)
    {
        printf("erro ao alocar memoria\n");     // Erro na alocação de memória.
        exit(-1);
    }
    save[0] = 0;                                   // Iicialização dos limites para a primeira thread
    save[1] = CadaThread;
    l->id = count;
    l->inf = save[count] + 1;                 // limite inferior de cada thread é colocado na estrutura, + 1 para não repetir
    l->sup = save[count+1];                   // limite superior de cada thread é colocado na estrutura

    if(count == numeroT-1)
    {
      l->sup = N;
    }
        if (pthread_create(&threads[count], NULL, Primo, (void *)l))
        {
          printf("<!> ERROR na criação da Thread");
          exit(-1);
        }
    save[count+2] = CadaThread + save[count+1];                 // Faz o somatório para o limite superior da proxima thread

  }

  // Término das Threads
  for(count = 0; count < numeroT; count++)
  {
      pthread_join(threads[count],NULL);
  }

// PRINT para o vetor de numeros primos
  printf("\n===========================================\n\tVETOR DE PRIMOS:\n");
  count = 0;
  while (count < nprimo) {
    printf("v[%ld]: #%ld", count, vec_prime[count]);
    printf("\n");
    count++;
  }
  pthread_exit(NULL);


}
