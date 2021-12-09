#include "medico.h"
#include "utilis.h"
#include "balcao.h"

int main(int argc, char *argv[]){
    char m_fifo_fname[25];
    pergunta_c perg; /* Mensagem do "tipo" pergunta */
    resposta_c resp; /* Mensagem do "tipo" resposta */
    int fd_server_fifo,fd_cliente_fifo;
    especialista  a;
    strcpy(a.nome,argv[1]);
    strcpy(a.especialidade,argv[2]);

    if(access(bal_FIFO, F_OK) != 0){
        printf("O balcao nao esta a executar\n");
        exit(2);
    }
    /*--- Cria o FIFO do Medico --- */
    perg.pid_medico = getpid();
    sprintf(m_fifo_fname,MEDICO_FIFO,perg.pid_medico);
    if(mkfifo(m_fifo_fname,0777) ==  -1){
        perror("\nmkfifo FIFO medico deu erro");
        exit(EXIT_FAILURE);
    }
    fprintf(stderr,"\nFIFO do medico criado");

    if(argc < 3){
        printf("Faltam argumentos!\n");
        return  -1;
    }
    /* --- Abre o FIFO do Servidor p/ Escrita --- */
    fd_server_fifo = open(bal_FIFO,O_WRONLY); /* bloqueante */
    if(fd_server_fifo == -1){
        fprintf(stderr,"\nO Servidor nao esta a correr\n");
        unlink(m_fifo_fname);
        exit(EXIT_FAILURE);
    }
    fprintf(stderr,"\nFIFO do Servidor aberto WRITE / BLOCKING");
    /*printf("Nome: %s\n",a.nome[1]);
    printf("Especialidade: %s\n",a.especialidade[2]);*/

    return 0;
}

    //1º - Só aceita executar se o balcão estiver em funcionamento;

    // 1.2º - Interagir com o balcao para registar nome e especialidade



    //3º - Fico a aguardar que o balcao me indique que tenho um novo utente para atender


    //4º - Diálogo com o utente (especialista escreve pergunta e aguarda pela resposta)




