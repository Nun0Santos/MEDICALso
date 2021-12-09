
#include "utilis.h"
#include "balcao.h"



int main(int argc, char *nome[NOME_MAX]){
        int fd_server_fifo,fd_cliente_fifo;
        pergunta_c perg; /* Mensagem do "tipo" pergunta */
        resposta_c resp; /* Mensagem do "tipo" resposta */
        char c_fifo_fname[25]; /* Nome do FIFO deste cliente */
        int read_res;
        utente a;

        if(access(bal_FIFO, F_OK) != 0){
        printf("O balcao nao esta a executar\n");
        exit(2);
        }
        if( argc < 2){
            printf("Faltam argumentos!\n");
            return -1;
        }
        /*--- Cria o FIFO do Cliente --- */
        perg.pid_cliente = getpid();
        sprintf(c_fifo_fname,CLIENT_FIFO,perg.pid_cliente);
        if(mkfifo(c_fifo_fname,0777) ==  -1){
            perror("\nmkfifo FIFO cliente deu erro");
            exit(EXIT_FAILURE);
        }
        fprintf(stderr,"\nFIFO do cliente criado");

        /* --- Abre o FIFO do Servidor p/ Escrita --- */
        fd_server_fifo = open(bal_FIFO,O_WRONLY); /* bloqueante */
        if(fd_server_fifo == -1){
            fprintf(stderr,"\nO Servidor nao esta a correr\n");
            unlink(c_fifo_fname);
            exit(EXIT_FAILURE);
        }
        fprintf(stderr,"\nFIFO do Servidor aberto WRITE / BLOCKING");
        fd_cliente_fifo = open(c_fifo_fname, O_RDWR); /* bloqueante */
        if(fd_cliente_fifo == -1){
            perror("\nErro ao abrir FIFO do cliente");
            close(fd_server_fifo);
            unlink(c_fifo_fname);
            exit(EXIT_FAILURE);
        }
        fprintf(stderr,"\nFIFO do cliente aberto  para READ (+write) BLOCK");
        memset(perg.frase, '\0',TAM_MAX);

        while(1){
            /* ---  OBTEM PERGUNTA --- */
            printf("\n--> ");
            fgets(perg.frase,sizeof(perg.frase),stdin);
            if(!strcasecmp(perg.frase,"fim"))
                break;
            /* --- ENVIA A PERGUNTA --- */
            write(fd_server_fifo,&perg,sizeof(perg));
            /* --- OBTEM A RESPOSTA --- */
            read_res = read(fd_cliente_fifo, &resp, sizeof(resp));
            if (read_res == sizeof (resp))
                printf("\n Respsota --> %s",resp.frase);
            else
                printf("\nSem Resposta ou resposta incompreensivel [bytes lidos : %d]",read_res);
        }
        close(fd_cliente_fifo);
        close(fd_server_fifo);
        unlink(c_fifo_fname);
        return 0;
}





    //1º - Só aceita executar se o balcão estiver em funcionamento : (Enviar um sinal para o balca para ver se está a correr ?)


    //1.2º - Enviar sintomas ao balcao


    //2º - Obter do balcao: area de especialidade, prioridade, numero de utentes nesse instante à frente dele nessa especialidade, numero de especialista dessa area


    //3º - Aguardar que seja encaminhado para um especialista (programa medico)

    //4º - Diálogo com o especialista (programa médico)





