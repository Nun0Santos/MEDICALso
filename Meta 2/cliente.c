
#include "utilis.h"
#include "func_balcao.h"

int main(int argc, char *argv[]) {
    int fd_server_fifo, fd_cliente_fifo, n, res, n_write;
    char c_fifo_fname[25]; /* Nome do FIFO deste cliente */
    char m_fifo_fname[25]; /* Nome do FIFO deste medico */
    char  str_com[30];

    int estado; //0 -> n esta em consulta 1-> consulta

    fd_set fds;
    struct timeval tempo;

    balcao b;

    /* ======================= VERIFICAR SE O BALCAO ESTA A CORRER ======================= */

    if (access(bal_FIFO, F_OK) != 0) {
        perror("[ERRO] O balcão não está a executar\n");
        exit(1);
    }
    if (argc < 2) {
        perror("[ERRO] Faltam argumentos!\n");
        exit(1);
    }
    strcpy(b.nome_utente, argv[1]);

    /* ======================= Cria o FIFO do Cliente =============================*/

    b.id_utente = getpid();
    sprintf(c_fifo_fname, CLIENT_FIFO, b.id_utente);
    if (mkfifo(c_fifo_fname, 0600) == -1) {
        perror("\n[ERRO] ao tentar criar FIFO deste cliente\n");
        exit(EXIT_FAILURE);
    }
    fprintf(stderr, "\n[FIFO] do cliente criado\n");

    /* ======================= Abre o FIFO do Servidor p/ Escrita ======================= */

    if ((fd_server_fifo = open(bal_FIFO, O_WRONLY)) == -1) { /* bloqueante */
        fprintf(stderr, "\n[ERRO] O Servidor não está a correr\n");
        unlink(c_fifo_fname);
        exit(EXIT_FAILURE);
    };
    printf("Abri o fifo do servidor...\n");

    /* ======================= FIFO do Cliente (aberto para leitura e escrita) ============================= */

    if ((fd_cliente_fifo = open(c_fifo_fname, O_RDWR)) == -1) { /* bloqueante */
        perror("\n[ERRO] ao abrir FIFO do cliente");
        close(fd_server_fifo);
        unlink(c_fifo_fname);
        exit(EXIT_FAILURE);
    };
    printf("Abri o fifo do cliente...\n");
    printf("\nPID [%d] Olá %s\n", b.id_utente, b.nome_utente);

    /* =======================  OBTEM SINTOMA DO UTENTE ======================= */
    b.cliente = 1;
    b.id_medico = 0;
    b.consulta = 0;
    b.sair = 0;

    printf("\nSintoma: ");
    fgets(b.sintoma, sizeof(b.sintoma), stdin);

    /* ======================= ENVIA SINTOMA AO BALCAO ======================= */
    n = write(fd_server_fifo, &b, sizeof(balcao));
    if (n == -1) {
        printf("\nNao conseguiu escrever para o balcao...");
        exit(1);
    }
    printf("Enviei ao balcao %d bytes ...\n", n);

    /* ======================= OBTEM ESPECIALIDADE E PRIORIDADE ======================= */
    estado = 1;
    n = read(fd_cliente_fifo, &b, sizeof(balcao));
    printf("Recebi do balcao %d bytes ...\n", n);

    if (n == sizeof(balcao)) {
        if (b.registo_utente == 1) {
            printf("Especialidade = [%s]  Prioridade = [%d]\n", b.classificao, b.prioridade);
        }
    } else {
        printf("\nSem Resposta ou resposta incompreensivel [bytes lidos : %d]\n", n);
    }
    do {
        printf("Estado: %d\n", estado);
        /* ======================= SELECT ======================= */
        /* ======================= PREPARAR DESCRITORES QUE QUERO MONOTORIZAR ============== */

        FD_ZERO(&fds);
        FD_SET(0, &fds);
        FD_SET(fd_cliente_fifo, &fds);
        tempo.tv_sec = 10;
        tempo.tv_usec = 0;

        res = select(fd_cliente_fifo + 1, &fds, NULL, NULL, &tempo);
        if (res == 0) {
            //printf("\nEstou sem fazer nada...\n");

        } else if (res > 0) {
            if (FD_ISSET(0, &fds)) { // o avisar o balcao para encerrar(sair do lado do balcao)
                fgets(str_com, sizeof(str_com), stdin);

                strcpy(b.msg, str_com);

                if (estado == 1) {
                    //se ja tiver recebido o confirmação do balcao, tudo o que for escrito é enviado para po balcao
                    b.cliente = 1;

                   if(strcmp(str_com,"sair\n")==0){
                       n = write(fd_server_fifo, &b, sizeof(balcao));
                        if (n == -1) {
                            printf("\nNão conseguiu escrever no FIFO do servidor...\n");
                            exit(1);
                        }
                        close(fd_server_fifo);
                        strcpy(str_com,"sair\n");
                    }

                } else{ //se estiver me consulta tudo que escrever vai ser enviado para o medico
                    /* ======================= ABRIR FIFO DO MEDICO ======================= */
                    sprintf(m_fifo_fname, MEDICO_FIFO, b.id_medico);
                    b.cliente = 1;
                    b.consulta = 1;
                    b.id_utente = getpid();

                   int fd_cliente_novo= open(m_fifo_fname, O_RDWR);
                    printf("Abri o FIFO do medico...\n");

                    /* ======================= ENVIAR AO MEDICO ======================= */
                    n_write = write(fd_cliente_novo, &b, sizeof(balcao));
                    if (n_write == -1) {
                        perror("Erro a escrever no FIFO do medico.... \n");
                        exit(1);
                    }
                    close(fd_cliente_novo);
                    printf("Enviei %d  bytes...\n", n);
                    if(strcmp(b.msg,"adeus\n") == 0){
                        strcpy(str_com,"sair\n");
                    }
                }
            }
            if (FD_ISSET(fd_cliente_fifo, &fds)) {
                n = read(fd_cliente_fifo, &b, sizeof(balcao));
                if (n == -1) {
                    perror("Erro ao ler do meu FIFO");
                    exit(1);
                }
                if (n == sizeof(balcao)){
                printf("Recebi %d bytes de alguém...\n", n); //balcao ou medico

                if (b.cliente == 0 && b.consulta == 1) {
                    estado = 2;
                    printf("Especialista [%d] : %s\n", b.id_medico, b.msg);

                }
                else if(b.flagB == 2 && b.cliente == 1){
                    printf("Tens especialista para te atender ...\n");
                    printf("\n=============================\n");
                    printf("Dados especialista :\n"
                           "Nome: %s \t Pid: %d \n",b.nome_medico,b.id_medico);
                    printf("\n=============================\n");
                    estado = 2;

                }
                else {
                    estado = 1;
                }
                if (b.cheio == 0 && b.flagN == -1 ) { // vem do balcao por nao poder atender mais clientes
                    printf("%s\n", b.msg);
                    strcpy(str_com,"sair\n");
                }
                if(b.consulta == -1){
                    estado = 1;
                    printf("O Especialista [%d] terminou a consulta\n",b.id_medico);
                    printf("Vou encerrar\n");
                    strcpy(str_com,"sair\n");
                }
                if(b.flagOcupado == 1 && b.consulta ==0){
                    printf("%s",b.msg);
                    estado = 1;
                }
                if(b.remove == 1){
                        printf("\nNao te quero aqui - Adeus...\n");
                        strcpy(str_com,"sair\n");

                }
                if(b.cliente == 1 && b.encerra == 1){
                    printf("\nOrdem do balcao para encerrar\n");
                    strcpy(str_com,"sair\n");
                }
            }
        }
        }
    }while (strcmp(str_com, "sair\n") != 0);

        //quando o cliente encerra da sinal ao balcao
        b.sair = 1;
        b.cliente = 1;
        b.consulta = 0;
        fd_server_fifo = open(FIFO_SINAL, O_WRONLY);
        if(fd_server_fifo == -1){
            printf("\nNao conseguiu abrir o FIFo do balcao...");
            exit(1);
        }
        n = write(fd_server_fifo, &b, sizeof(balcao));
        if (n == -1) {
            printf("\nNao conseguiu escrever no FIFO do balcao...\n");
            exit(1);
        }
        close(fd_server_fifo);
        strcpy(str_com, "sair\n");
        close(fd_cliente_fifo);
        unlink(c_fifo_fname);
        exit(0);

}





