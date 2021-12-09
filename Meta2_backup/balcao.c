//
// Created by user on 07/11/21.
//
#include "balcao.h"
#include "func_balcao.h"
#include "utilis.h"
#include <signal.h>


int main() {
    char linha[100]; // espaco alocado para escrita de comandos
    int res,exstat,balcaoToClassificador[2],ClassificadorToBalcao[2];
    int tam = 0;
    char *MaxClientes_str, *MaxMedicos_str; //Var ambiente
    int MaxClientes, MaxMedicos;
    int fd_server_fifo,fd_cliente_fifo;
    pergunta_c perg; /* Mensagem do "tipo" pergunta */
    resposta_c resp; /* Mensagem do "tipo" resposta */
    char comando[30],resposta[30];
    char c_fifo_fname[50];
    pipe(balcaoToClassificador);
    pipe(ClassificadorToBalcao); //Pipe retorno

    // VERIFICAR SE O BALCAO ESTA A CORRER

    if(access(bal_FIFO, F_OK) == 0){
        printf("Balcão já está  executar\n");
        exit(2);
    }
    mkfifo(bal_FIFO, 0777);
    printf("Criei o FIFO do balcão...\n");

    /* --- Abrir FIFO Servidor --- */

    fd_server_fifo = open(bal_FIFO,O_RDWR);
    if((fd_server_fifo) == -1){
        perror("\nErro ao abrir o FIFO do servidor\n");
        exit(EXIT_FAILURE);
    }
    fprintf(stderr,"\nFIFO aberto para READ (+WRITE) bloqueante");


    MaxClientes_str = getenv("MAXCLIENTES");
    if (MaxClientes_str){
       MaxClientes = atoi(MaxClientes_str);
    }
    else{
        printf("Erro ao ler MAXCLIENTES\n");
        return -1;
    }
    printf("\nMAXCLIENTE = %d\n",MaxClientes);

    MaxMedicos_str = getenv("MAXMEDICOS");
    if (MaxMedicos_str){
        MaxMedicos = atoi(MaxMedicos_str);
    }
    else{
        printf("Erro ao ler MaxMedicos\n");
        return -1;
    }
    printf("MAXMEDICOS = %d\n",MaxMedicos);

   utente* ptrUtentes;
   especialista* ptrEspecialistas;

   ptrUtentes = malloc(sizeof(utente) * MaxClientes);
   if (ptrUtentes == NULL) {
       printf("Erro na alocacao dos utentes!\n");
       return -1;
   }

   ptrEspecialistas = malloc(sizeof(especialista) * MaxMedicos);
   if (ptrEspecialistas == NULL) {
       printf("Erro na alocacao dos medicos especialistas!\n");
       return -1;
   }
        res = fork();
        if (res == 0) { //filho -> vai executar o comand
            //printf("Sou o filho %d\n", getpid());
            close(0);
            dup(balcaoToClassificador[0]);
            close(balcaoToClassificador[0]);
            close(balcaoToClassificador[1]);

            close(1);
            dup(ClassificadorToBalcao[1]);
            close(ClassificadorToBalcao[0]);
            close(ClassificadorToBalcao[1]);

            execlp("./classificador","./classificador", NULL); //VER ESPACOS
            fprintf(stderr, "Comando nao encontrado\n");
            exit(123);
           
        }
        //pai aguarda pelo comando e depois continua
        close(balcaoToClassificador[0]);
        close(ClassificadorToBalcao[1]);
    while(1){
        /* --- OBTEM FRASE DO CLIENTE --- */
        res = read(fd_server_fifo,&perg,sizeof(perg));
        if(res < sizeof (perg)){
            fprintf(stderr,"\nRecebida pergunta incompleta [bytes lidos: %d]",res);
            continue;
        }
        fprintf(stderr,"\nRecebido [%s]",perg.frase);
        write(balcaoToClassificador[1], perg.frase, sizeof(perg.frase));
        /* --- OBTEM RESPOSTA DO CLASSIFICADOR --- */
        res = read(ClassificadorToBalcao[0],resp.frase,sizeof(resp.frase)-1);
        resp.frase[res] = '\0';
        fprintf(stderr,"\nResposta = [%s]",resp.frase);
        /* --- OBTEM FILENAME DO FIFO PARA A RESPOSTA --- */
        sprintf(c_fifo_fname,CLIENT_FIFO,perg.pid_cliente);
        /* --- ABRE O FIFO DO CLIENTE P/WRITE --- */
        fd_cliente_fifo = open (c_fifo_fname,O_WRONLY);
        if(fd_cliente_fifo == -1)
            perror("\nErro no open - Ninguém quis a resposta");
        else {
            fprintf(stderr,"\nFIFO cliente aberto para WRITE");
            /* --- ENVIA RESPOSTA --- */
            res = write(fd_cliente_fifo,&resp,sizeof (resp));
            if(res == sizeof(resp))
                fprintf(stderr,"\nEscreveu a resposta");
            else
                perror("\nErro a escrever a resposta");

            close(fd_cliente_fifo); /* FECHA LOGO O FIFO DO CLIENTE */
            fprintf(stderr,"\nFIFO cliente fechado");
        }
    } /* FIM DO CICLO PRINCIPAL DO SERVIDOR*/
        close(fd_server_fifo);
        wait(&exstat);
	    close(balcaoToClassificador[1]); //Fechar descritores
	    close(ClassificadorToBalcao[0]);
        unlink("server_fifo");
        return 0;
}	

// 1º - Verificar se já existe outro balcão a correr : (comando ps ax diz-me todos os processos que estão a decorrer)


//2º - Interagir em SIMULTÂNEO com clientes, médicos e classificador :  (função execl permite-me executar um progrma)

//3º - Recebe do médico um sinal de vida a cada 20 segundos (sleep(20)), : Usar sinais (kill -> Envia um sinal para um processo |  signal -> recebe o sinal )

//4º - Executa internamente o classificador e interage com ele para obter as classificações : (o comando bg permite-me correr um programa em background e continuar a usar a linha de comandos)

//5º-  Recebe informações dos clientes: utente chegou com estes sintomas; utente desistiu;

//6º - Recebe informações dos médicos: novo especialista para especialidade X, saída de especialista, fim de consulta;

//7º - Informa o cliente que o utente vai iniciar consulta no especialista X, dando as informações necessárias

//8º - Informa o médico que o especialista vai iniciar consulta ao utente X, dando as informações necessárias

//9º - Deteta que os médicos deixar de enviar o sinal de vida e age em conformidade : (Agir em conformidade é encerrar o programa médicos e todos os outros programas?)




