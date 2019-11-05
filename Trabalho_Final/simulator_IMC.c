/***************************************************************************************
 ******************                  SIMULATOR                        ******************
 * MAIN:                                                                               *
 *      Efetua comunicação com monitor.                                                *
 *      Criação das Threads.                                                           *
 *                                                                                     *
 * RECEIVER:                                                                           *
 *          Conversão e atribuição dos valores para                                    *
 *          as variáveis globais.                                                      *
 *                                                                                     *
 * IMC:                                                                                *
 *      Efetua o calculo do IMC e atualiza variáves globais.                           *
 *      Conversão para string.                                                         *
 *                                                                                     *
 * MEDIDAS MEDIAS:                                                                     *
 *              Efetua o calculo das médias, atraves das variáveis globais que         *
 *              são sempre atualizadas.                                                *
 *              Conversão para string.                                                 *
 *                                                                                     *
 * TENDENCIA:                                                                          *
 *          Efetua o calculo da tendencia, considerando 10% o nível do IMC.            *
 *          Necessita do valor do IMC para fazer estimativa.                           *
 *          Conversão para string.                                                     *
 *                                                                                     *
 * SEND RESULT:                                                                        *
 *          Percorre buffer até encontrar comando específico.                          *
 *          Atualiza buffer.                                                           *
 *          Faz a comunicação com o monitor, retornando o resultado.                   *  
 ***************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <locale.h>
#include <time.h>
#include <signal.h>

/* Tempos para execução das tarefas periódicas*/
#define TIME_IMC 9000000
#define TIME_MED 6000000
#define TIME_TEND 3000000

#define T_buffers 256   /* Tamanho de linhas do buffer de informações */

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t protect = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;     


/*********************************************************************************************
 ***********************                    GLOBAIS                    ***********************   
 *   newsockfd       ->  Armazena cliente                                                    *
 *   id_comando      ->  Identificador do comando                                            *
 *   linhas          ->  Contador de linhas para o buffer SAVE                               *
 *   id_co_peso_alt  ->  Contador para armazenar as alturas e pesos recebidos                *
 *   peso[512]       ->  Vetor para armazenar pesos enviados                                 *
 *   altura[512]     ->  Vetor para armazenar alturas enviados                               *
 *   med_peso        ->  Variavel que recebe a média dos pesos                               *
 *   med_altura      ->  Variavel que recebe a média das alturas                             *
 *   imc             ->  Recebe calculo do IMC                                               *
 *   save[512][30]   ->  Buffer para salvar informações                                      *
 *   vazio           ->  Flag para indicar estado do buffer                                  *
 *   n_pesos         ->  Numero de pesos registrados                                         *
 *   n_alturas       ->  Numero de alturas registradas                                       *
 *   id_send         ->  Indice de ítens enviados                                            *
 *   find            ->  Indica se informação foi encontrada no buffer                       *
 *   i_save          ->  Indice para percorrer o buffer                                      *
 *   send_med_peso   ->  Variavel que recebe o peso médio calculado                          * 
 *   send_med_altura ->  Variavel que recebe a altura média calculada                        *
 *********************************************************************************************/

int newsockfd;                 
int id_comando[100], linhas = 0, id_co_peso_alt = 0, n_pesos = 0, n_alturas = 0, id_send = 0, find = 0, i_save = 0;
double peso[100], altura[100], med_peso = 0, med_altura = 0, imc = 0, send_med_peso = 0, send_med_altura = 0; 
char save[T_buffers][5];     
int vazio = 0;          

/* Periodic threads using POSIX timers */
struct periodic_info
{
	int sig;			
	sigset_t alarm_sig; 
};

/* Funções Periódicas */
void* calcula_IMC(void *arg);     
void* tendencia(void *arg);
void* medidas_medias(void *arg);

void* send_results(void *arg);
static int make_periodic(int unsigned period, struct periodic_info *info);
static void wait_period(struct periodic_info *info);




/********************************************   PERIODICIDADE    **********************************************/
static int make_periodic(int unsigned period, struct periodic_info *info)
{ //(intervalo, referência pra estrutura)
	static int next_sig;
	int ret;
	unsigned int ns;
	unsigned int sec;
	struct sigevent sigev;
	timer_t timer_id;
	struct itimerspec itval;
	/* Initialise next_sig first time through. We can't use static
	   initialisation because SIGRTMIN is a function call, not a constant */
	if (next_sig == 0)
		next_sig = SIGRTMIN;
	/* Check that we have not run out of signals */
	if (next_sig > SIGRTMAX)
		return -1;
	info->sig = next_sig;
	next_sig++;
	/* Create the signal mask that will be used in wait_period */
	sigemptyset(&(info->alarm_sig));
	sigaddset(&(info->alarm_sig), info->sig);
	/* Create a timer that will generate the signal we have chosen */
	sigev.sigev_notify = SIGEV_SIGNAL;
	sigev.sigev_signo = info->sig;
	sigev.sigev_value.sival_ptr = (void *)&timer_id;
	ret = timer_create(CLOCK_MONOTONIC, &sigev, &timer_id);
	if (ret == -1)
		return ret;
	/* Make the timer periodic */
	sec = period / 1000000;
	ns = (period - (sec * 1000000)) * 1000;
	itval.it_interval.tv_sec = sec;
	itval.it_interval.tv_nsec = ns;
	itval.it_value.tv_sec = sec;
	itval.it_value.tv_nsec = ns;
	ret = timer_settime(timer_id, 0, &itval, NULL);
	return ret;
}
static void wait_period(struct periodic_info *info)
{
	int sig;
	sigwait(&(info->alarm_sig), &sig);
}


/********************************************   RECEPTOR    **********************************************/
void *receiver(void *arg) {  /* Recebe dados e identifica comandos */
  int n, ctrl;
  char buffer[T_buffers], *aux;
  while (1) {
    bzero(buffer,sizeof(buffer));  /* zera buffer para receber dados */
    n = read(newsockfd,buffer,50);

    if (n <= 0) {           /* condição para fechar = 0 */
        printf("Erro lendo do socket!\n");
        exit(1);
    }

    /*  
     *  Atribui valores recebidos para as variáveis, já convertidos para que as operações sejam efetuadas
     */
    pthread_mutex_lock(&protect);    
        ctrl = 0;
        aux = strtok(buffer, " "); 
        id_comando[id_co_peso_alt] = atoi(aux);
        while (aux != NULL) {
            aux = strtok(NULL, " ");
            if (ctrl == 0){         // Controle para salvar o primeiro - altura
                altura[id_co_peso_alt] = atof(aux);
                ctrl = 1;
            }else if (ctrl == 1){   // Controle para salvar o segundo - peso
                peso[id_co_peso_alt] = atof(aux);
                ctrl = 2;           // Não salva mais
            }
        }
        if(id_co_peso_alt == T_buffers)       /* Começa a sobreescrever */
            id_co_peso_alt = 0;
        else
            id_co_peso_alt++;
    pthread_mutex_unlock(&protect);
   }
}


/********************************************   IMC    **********************************************/
void*  calcula_IMC(void *arg){ 
    char imc_char[50];	
    struct periodic_info info;
	make_periodic(TIME_IMC, &info); 
    while(1){
        pthread_mutex_lock(&protect);
            if(peso[linhas] != 0 && altura[linhas] != 0){   /* Só efetua o calculo se não forem zero */
                imc = peso[linhas]/(altura[linhas]*altura[linhas]) * 10000;
                sprintf(imc_char, "%f", imc);       /* conversão para char para poder colocar no buffer */
                if(id_comando[linhas] == 0){  
                    /* 
                     * incrementa numero de pesos e alturas lidos 
                     * e efetua a soma dos pesos e alturas
                     */
                    n_alturas++;                
                    n_pesos++;        
                    med_peso = (med_peso + peso[linhas]);
                    med_altura = (med_altura + altura[linhas]);
                    /* Começa a salvar no buffer */
                    save[linhas][0] = '0';
                    save[linhas][1] = ' ';
                    for(int i = 0; i < strlen(imc_char); ++i){
                        save[linhas][i+2] = imc_char[i];
                    }
                if(linhas == T_buffers)   /* Sobrescreve */
                    linhas = 0;
                else
                    linhas++;   
                }
                vazio = 1;
                pthread_cond_signal(&cond);
            }
        pthread_mutex_unlock(&protect);
		wait_period(&info);
    } 
}

/********************************************   TENDENCIAS    ***********************************************/
void* tendencia(void *arg){
    struct periodic_info info;
	make_periodic(TIME_TEND, &info); 
    while(1){
        if(id_comando[linhas] == 2 && imc != 0){
            pthread_mutex_lock(&protect);
                /* 
                 * incrementa numero de pesos e alturas lidos 
                 * e efetua a soma dos pesos e alturas
                 */
                n_alturas++;  
                n_pesos++;       
                med_peso = (med_peso + peso[linhas]);
                med_altura = (med_altura + altura[linhas]);

                /* Começa a salvar no buffer */
                save[linhas][0] = '2';
                save[linhas][1] = ' ';
                /* Dependendo do imc salva salva no buffer de informações (save) */
                if (imc <= 20.35){
                   save[linhas][2] = 'M';
                   save[linhas][3] = 'a';
                   save[linhas][4] = 'g';
                   save[linhas][5] = 'r';
                   save[linhas][6] = 'e';
                   save[linhas][7] = 'z';
                   save[linhas][8] = 'a';
                   save[linhas][9] = '\0';
                }else if (imc >= 22.41 && imc <= 27.39){
                    save[linhas][2] = 'S';
                    save[linhas][3] = 'o';
                    save[linhas][4] = 'b';
                    save[linhas][5] = 'r';
                    save[linhas][6] = 'e';
                    save[linhas][7] = 'p';
                    save[linhas][8] = 'e';
                    save[linhas][9] = 's';
                    save[linhas][10] = 'o';
                    save[linhas][11] = '\0';
                }else if (imc > 27.39){
                    save[linhas][2] = 'O';
                    save[linhas][3] = 'b';
                    save[linhas][4] = 'e';
                    save[linhas][5] = 's';
                    save[linhas][6] = 'i';
                    save[linhas][7] = 'd';
                    save[linhas][8] = 'a';
                    save[linhas][9] = 'd';
                    save[linhas][10] = 'e';
                    save[linhas][11] = '\0';
                }else{
                    save[linhas][2] = 'N';
                    save[linhas][3] = 'o';
                    save[linhas][4] = 'r';
                    save[linhas][5] = 'm';
                    save[linhas][6] = 'a';
                    save[linhas][7] = 'l';
                    save[linhas][8] = '\0';
                }
                
                if(linhas == T_buffers)   /* Sobrescreve */
                    linhas = 0;
                else
                    linhas++;
                vazio = 1;
                pthread_cond_signal(&cond);
            pthread_mutex_unlock(&protect);
        }  
		wait_period(&info); 
    }
}


/********************************************   MEDIDAS_MEDIAS    **********************************************/
void* medidas_medias(void *arg){
    char char_peso[50], char_altura[50];
    int i, j;
    struct periodic_info info;
    make_periodic(TIME_MED, &info);
    while(1){
        pthread_mutex_lock(&protect);             
            if(id_comando[linhas] == 1 && peso[linhas] != 0 && altura[linhas] != 0){
                /* 
                 * incrementa numero de pesos e alturas lidos 
                 * e efetua a soma dos pesos e alturas
                 */
                n_alturas++;  
                n_pesos++;       
                med_peso = (med_peso + peso[linhas]);
                med_altura = (med_altura + altura[linhas]);
                
                 /* conversão para char para poder colocar no buffer */
                send_med_peso = med_peso/n_pesos;
                send_med_altura = med_altura/n_alturas;
                sprintf(char_peso, "%.2f", send_med_peso);
                sprintf(char_altura, "%.2f", send_med_altura);
                
                save[linhas][0] = '1';
                save[linhas][1] = ' ';
                for(i = 0; i < strlen(char_altura); ++i){
                    save[linhas][i+2] = char_altura[i];
                }
                save[linhas][i+2] = ' ';
                j = i + 3;
                i = 0;
                for(j; i < strlen(char_peso); ++j){
                    save[linhas][j] = char_peso[i];
                    ++i;
                }
                vazio = 1;
                pthread_cond_signal(&cond);
                if(linhas == T_buffers)
                    linhas = 0;
                else
                    linhas++;   
            }
        pthread_mutex_unlock(&protect);  
	wait_period(&info); 
    }
}


/********************************************   SEND_RESULTS    **********************************************/
void* send_results(void *arg){
    int n, j;
    char buffer[T_buffers], id_comando_char[2];
    while(1){
        pthread_mutex_lock(&protect); 
            /* Enquanto buffer com informações estiver vazio, espera */
            while(vazio <= 0){
                pthread_cond_wait(&cond, &protect);
            }
            
            /**
             *  Percorre vetor com informações até encontrar comando específico 
             *  Comando é convertido para char, para que a comparação aconteça
             */
            i_save = 0;
            while(i_save <= linhas-1){
                sprintf(id_comando_char, "%d", id_comando[i_save]);
                if(id_comando_char[0] == save[i_save][0]){
                    bzero(buffer,sizeof(buffer));  /* zera buffer para receber dados */
                        for(j = 0; save[i_save][j] != '\0'; ++j){
                            buffer[j] = save[i_save][j];
                        }

                        id_send++;
                        find = 1;
                        save[i_save][0] = '3';     // Inutiliza a linha do buffer  
                        break;
                /* Se não encontra, procura na outra linha */
                }else{
                    find = 0;
                    i_save++;
                }
            }
            /* Se informação não for encontrada ou todas já foram enviadas, buffer esta vazio */
            if(!find){
                vazio = 0;
            }else{
            /* Se elemento for encontrado, envia para monitor */
            pthread_mutex_lock(&mutex); 
                n = write(newsockfd,buffer,50);
                if (n < 0) {
                    printf("Erro escrevendo no socket!\n");
                    exit(1);
                }
                printf("<!>\tMensagem enviada\n");
            pthread_mutex_unlock(&mutex);             
            }
        pthread_mutex_unlock(&protect); 
    }
}


/********************************************   PRINCIPAL    **********************************************/
int main(int argc, char *argv[]) {
   /* Função para utilizar Acentos no Printf */
    setlocale(LC_ALL, "Portuguese");
    
    /*
     * Estrutura para servidor e para cliente (guarda IP)
     */
    struct sockaddr_in serv_addr, cli_addr;   

    socklen_t clilen;
    int sockfd, portno, i;        /* descritor socket e porta - passada pelo argv */
    pthread_t t, calc_imc, calc_med, calc_tend, envia;

    sigset_t alarm_sig; //conjunto de sinais "sinais de alarme"

	sigemptyset(&alarm_sig); //limpa o conjunto
	for (i = SIGRTMIN; i <= SIGRTMAX; i++)
		sigaddset(&alarm_sig, i);			  //adiciona sinais de SIGRTMIN até SIGRTMAX (32 sinais)
	sigprocmask(SIG_BLOCK, &alarm_sig, NULL); //"dado esses 32 sinais eu quero bloquear todos eles, se eles ocorrerem em algum momento"

    if (argc < 2) {
        printf("Erro, porta nao definida!\n");
        exit(1);
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);  /* definição do socket - TCPIP*/ 
    if (sockfd < 0) {  
        printf("Erro abrindo o socket!\n");
        exit(1);
    }
     
    /* Zera para receber dados */
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);

    /*
     *   Funções para conversões em endereço de REDE;
     */
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
     
    /* 
    * Associa o socket criado a porta 
    * local do sistema operacional. Nesta associação é verificado 
    * se a porta já não está sendo utilizada por algum outro processo 
    */
    if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
        printf("Erro fazendo bind!\n");
        exit(1);
    }
     
    listen(sockfd,1);  /* Coloca porta em escuta - 1 conexões aceitáveis */

    while (1) {
        /* Comunicação com o cliente */
    	newsockfd = accept(sockfd,(struct sockaddr *) &cli_addr,&clilen);
	pthread_mutex_lock(&mutex);
     	if (newsockfd < 0) {
        	printf("Erro no accept!\n");
         	exit(1);
    	}
        pthread_create(&t, NULL, receiver, NULL);
        pthread_create(&calc_imc, NULL, calcula_IMC, NULL);
        pthread_create(&calc_med, NULL, medidas_medias, NULL);
        pthread_create(&calc_tend, NULL, tendencia, NULL);
        pthread_create(&envia, NULL, send_results, NULL);
        printf("<!>\tConexao estabelecida. Esperando comandos...\n\n");
	pthread_mutex_unlock(&mutex);
    }
    return 0; 
}
