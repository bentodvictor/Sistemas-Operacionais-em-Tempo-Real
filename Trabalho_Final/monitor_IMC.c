    /***************************************************************************************
 ******************                   MONITOR                         ******************
 * MAIN:                                                                               *
 *      Envia comandos válidos(0, 1, 2) para o simulador.                              *
 *      Se comandos forem inválidos não envia, espera por comandos válidos.            *
 *                                                                                     *
 * LEITURA:                                                                            *
 *          Recebe do socket o resultado.                                              *
 *          Dependendo do comando existe um print específico, também                   * 
 *          ha uma sepação da string e conversão para o devido tipo para impressão.    *
 **************************************************************************************/ 


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <arpa/inet.h>		// inet_aton
#include <pthread.h>
#include <locale.h>


int sockfd;

/**
 * Função para leitura do socket:
 *      Lê informação e salva em buffer.
*/

void *leitura(void *arg) {
    char buffer[256];
    int n;
    char *imc, *id_comand, *_peso_altura, *tend;
    int ctrl_print = 0;
    while (1) {
        bzero(buffer,sizeof(buffer));
        n = recv(sockfd,buffer,50,0);
        
        if (n <= 0) {
            printf("Erro lendo do socket!\n");
            exit(1);
        }
        
        /*
         *  Identificação dos comandos para amostragem de dados:
         * id_comand recebe o comando.
         * imc recebe o calculo do IMC.
         */ 
        if (buffer[0] == '0'){
            id_comand = strtok(buffer, " "); 
            imc = strtok(NULL, " ");
            
            if(imc < "18.5")
                printf("<!>\tIMC: %s kg/m^2 - Magreza\n", imc);
            else if (imc >= "18.5" || imc <= "24.5")
                printf("<!>\tIMC: %s kg/m^2 - Normal\n", imc);
            else if (imc > "24.9" || imc <= "30")
                printf("<!>\tIMC: %s kg/m^2 - Sobrepeso\n", imc);
            else
                printf("<!>\tIMC: %s kg/m^2 - Obesidade\n", imc);
            
            /**
            *  _peso_altura recebe comando, e passa para variável id_comand.
            * ctrl_print, serve para separar as informações na hora do print primeiro
            * mostrando o peso médio (ctrl = 0), depois mostrando a altura média (ctrl = 1), 
            * ao fim a flag ctrl é setada para 0 para que a conversão da string pare.
            */
        }else if (buffer[0] == '1'){
            _peso_altura = strtok(buffer, " "); 
            id_comand = _peso_altura;
            ctrl_print = 0;
            
            while (_peso_altura != NULL) {
                _peso_altura = strtok(NULL, " ");
                if(ctrl_print == 0){    
                    printf("<!>\tPeso Médio: %s\n", _peso_altura);
                    ctrl_print = 1;
                } else if (ctrl_print == 1) {
                    printf("<!>\tAltura Média: %s\n", _peso_altura);
                    ctrl_print = 2;     // Não printa mais 
                }
            }

            /**
             * Se o comando for 2, será amostrada a tendência do indivíduo
             */
        }else{
            id_comand = strtok(buffer, " "); 
            tend = strtok(NULL, " ");
            printf("<!>\tTendencia do indivíduo: %s\n", tend);
        }
    }
}

int main(int argc, char *argv[]) {
   /* Função para utilizar Acentos no Printf */
    setlocale(LC_ALL, "Portuguese");

    int portno, n;
    struct sockaddr_in serv_addr;
    pthread_t t;
    char buffer[256];

    if (argc < 3) {
       fprintf(stderr,"Uso: %s nomehost porta\n", argv[0]);
       exit(0);
    }
   
    portno = atoi(argv[2]);     /* Pega numero da porta */
    
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        printf("Erro criando socket!\n");
        return -1;
    }
    
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
//    serv_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    inet_aton(argv[1], &serv_addr.sin_addr);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
        printf("Erro conectando!\n");
        return -1;
    }
    
    pthread_create(&t, NULL, leitura, NULL);
    printf("\n===========================================================\n");
    printf("\tComandos:\n0:Cálculo IMC\n1:Dados médios do grupo\n2:Tendência do indivíduo\n");
    printf("\tSintaxe:\n");
    printf("<comando>  espaço  <altura(em cm)>  espaço  <peso(em kg)>\n");
    printf("\t*para encerrar digite <exit>.\n");
    printf("===========================================================\n\n");
    
    printf("<?>\tInforme os comandos:\n");
    do {
        bzero(buffer,sizeof(buffer));   /* Limpa buffer */
        fgets(buffer,50,stdin);
        if (strcmp(buffer,"exit\n") == 0) {
            break;
        }
        /* Condição para comandoa aceitos */
        if (buffer[0] == '0' || buffer[0] == '1' || buffer[0] == '2'){
            n = send(sockfd,buffer,50,0);
            if (n == -1) {
                printf("Erro escrevendo no socket!\n");
                return -1;
            }
        }else {
            printf("\n<?>\tComando -> %s\tInválido, informe o comando novamente.\n\n", buffer);
        }
    } while (1);

    close(sockfd);

    return 0;
}
