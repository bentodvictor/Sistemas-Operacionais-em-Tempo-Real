#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#define X 5
/*
    Para compilar utilizar:
                            gcc -o teste Calcula.c -lm
*/

// Estrutura para as Tarefas
typedef struct tarefa{
  float C,PD;       // Computação e Período/Deadline
  int ID, PRIO;     // ID da tarefa e Prioridade
}Tarefa;

void utilization(Tarefa *tarefas);
void response_time(Tarefa *tarefas);

//  MAIN
int main(){
  srand(time(NULL));      // Valores randomicos aleatórios

  Tarefa v[X], vo[X];     // Vetor e Vetor_Ordenado
  int i, j, menor;        // Contadores e temporário

  // Inicializa valores das tarefas
  for (i = 0; i < X; i++) {
    v[i].ID = i+1;
    v[i].C = 1 + rand() % 10;
    v[i].PD = 5 + rand() % 100;
    v[i].PRIO = 0;
  }

  /*SEM ORDENAR*/
  printf("SEM ORDENACAO:\n\tID, Prioridade, Periodo/Deadline\n");
  for (i = 0; i < X; ++i){
    printf("v[%d]:\t%d\t%d\t\t%.2f\n", i, v[i].ID, v[i].PRIO, v[i].PD);
  }

  // Atribui prioridades conforme o valor do período.
  // MENOR PERIODO = MAIOR PRIORIDADE
  j= 0;
  menor = 0;
  for (i = 0; i < X; i++) {
    while(j < (X-1)){     // Até (X-1) pois verifica o termo "j+1" e o vetor possui 5 posições
      if (v[menor].PD >= v[j+1].PD){        // Percorre todo o vetor de tarefas até achar o menor preríodo
        menor = j+1;      // Salva índice da tarefa com menor período
      }
      j++;
    }
    v[menor].PRIO = i+1;      // Atribui menor prioridade a tarefa com menor preríodo
    vo[i] = v[menor];         // Vetor é ordenado por Prioridade
    v[menor].PD = 1001;       // Prioridade menor passa a ser a maior
    j = -1;       // -1 para poder analisar o termo de posição 0 no vetor
  }

  // ORDENADO POR PRIORIDADE
  printf("\nORDENADO:\n\tID, Prioridade, Periodo/Deadline\n");
  for (i = 0; i < X; ++i){
    printf("vo[%d]:\t%d\t%d\t\t%.2f\n", i, vo[i].ID, vo[i].PRIO, vo[i].PD);
  }

  //  Invoca Funções
  utilization(vo);
  response_time(vo);
}

// Escalonamento baseado em UTILIZAÇÂO
void utilization(Tarefa *tarefas){
  int count;      // Contador
  float use = 0.0;      // Utilizaçãod o Processador
  for(count = 0; count < X; count++){     // Laço para calcular o somatório U = Ci/PDi
    use += tarefas[count].C / tarefas[count].PD;
  }
  printf("\n\n==================================================");
  if(use < 1)
    printf("\n<!>\tEscalonamento baseado em UTILIZAÇÂO:\nTarefa ESCALONAVEL\nUtilização do Processador: %f\n", use);
  else
    printf("\n<!>\tEscalonamento baseado em UTILIZAÇÂO:\nTarefa NAO ESCALONAVEL\nUtilização do Processador: %f\n", use);
}

// Escalonamento por TEMPO DE RESPOSTA
void response_time(Tarefa *tarefas){
  float Ci, PDi;      // Tarefa Atual
  float Cj, PDj;      // Proxima Tarefa
  float Rant, Ratual, I;      // Janelas de tempo e Interferência
  int count = 0;        // Usado para controle da tarefa atual
  int count2;         // Usado para pegar os valores da tarefa anterior (maior prioridade) para efetuar os cálculos

  while (count < X) {       // Primeiro laço (Pega sempre o valor da Computação)
    Ci = tarefas[count].C;
    PDi = tarefas[count].PD;

    Rant = 0;
    Ratual = Ci;        // Na primeira Iteração Ratual = Computação

    while(Rant != Ratual && Ratual < PDi  && count > 0){        // Segundo laço (Verifica se Valor foi repetido, Deadline e se não é a primeira iteração)
      Rant = Ratual;
      I = 0;
      for(count2 = 0; count2 < count; count2++){      // Terceiro laço (Verifica Interferencia das tarefas de maior prioridade)
        Cj = tarefas[count2].C;       // Valores de Computação e Período da tarefa anterior (maior prioridade)
        PDj = tarefas[count2].PD;
        I += ceil(Rant/PDj) * Cj;
      }
      Ratual = Ci + I;
    }
    count++;
  }
  printf("==================================================");
  if(Ratual >= PDi){
    printf("\n<!>\tEscalonamento por TEMPO DE RESPOSTA:\nTarefa NAO ESCALONAVEL\n");
    printf("==================================================");
  }else{
    printf("\n<!>\tEscalonamento por TEMPO DE RESPOSTA:\nTarefa ESCALONAVEL\n");
    printf("==================================================\n\n");
  }
}
