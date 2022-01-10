
#include "utilis.h"
#include "func_balcao.h"

int main(int argc, char *argv[]) {
    int fd_server_fifo, fd_cliente_fifo, n, res,n_write; // file descriptors para os FIFOS
    char c_fifo_fname[25]; /* Nome do FIFO deste cliente */
    char m_fifo_fname[25]; /* Nome do FIFO deste medico */
    char comando[30],str_com[30];
    int ClienteToMedico[2], MedicoToCliente[2];
    int estado_cli =0; //0 -> n esta em consulta 1-> consulta
    fd_set fds;
    struct timeval tempo;

    char debug[20];
    balcao b;
    b.cliente = 1;

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
    printf("PID [%d] Olá %s\n",b.id_utente,b.nome_utente);

    /* =======================  OBTEM SINTOMA DO UTENTE ======================= */
    printf("\nSintoma: ");
    fgets(b.sintoma, SINTOMA_MAX, stdin);

    /* ======================= ENVIA SINTOMA AO BALCAO ======================= */
    n = write(fd_server_fifo, &b, sizeof(balcao));
    if(n == -1){
        printf("\nNao conseguiu escrever para o balcao...");
        exit(1);
    }
    printf("Enviei ao balcao %d bytes ...\n", n);

    /* ======================= OBTEM ESPECIALIDADE E PRIORIDADE ======================= */
    n = read(fd_cliente_fifo, &b, sizeof(balcao));
    printf("Recebi do balcao %d bytes ...\n", n);

    if (n == sizeof(balcao)){
         if (b.cheio == 0) {
            printf("\nO balcão não consegue atender mais clientes - volte mais tarde!\n");
            exit(1);
        }
         else{
             printf("Especialidade = [%s]  Prioridade = [%d]\n", b.classificao, b.prioridade);
         }
    }
    else{
        printf("\nSem Resposta ou resposta incompreensivel [bytes lidos : %d]\n", n);
    }

    do{
        printf("-->\n");
        /* ======================= SELECT ======================= */

        /* ======================= PREPARAR DESCRITORES QUE QUERO MONOTORIZAR ============== */
        FD_ZERO(&fds); //LIMPAR DESCRITORES
        FD_SET(0, &fds); // TECLADO
        FD_SET(fd_cliente_fifo, &fds); // FIFO cliente

        tempo.tv_sec = 8; // TIMEOUT
        tempo.tv_usec = 0;

        res = select(fd_cliente_fifo + 1, &fds, NULL, NULL, &tempo);
        if (res == 0) {
            //printf("\nEstou sem fazer nada...\n");
        } else if (res > 0 && FD_ISSET(0, &fds)) { // o avisar o balcao para encerrar(sair do lado do balcao)
            fgets(b.msg,sizeof(b.msg),stdin);

            if(estado_cli == 1) {
                sprintf(m_fifo_fname, MEDICO_FIFO, b.id_medico);
                fd_cliente_fifo = open(m_fifo_fname, O_RDWR);
                n_write = write(fd_cliente_fifo, &b, sizeof(balcao));
                if (n_write == -1) {
                    perror("Erro a escrever no FIFO do medico.... \n");
                    exit(1);
                }
                printf("Enviei %d  bytes...\n", n);
                close(fd_cliente_fifo);

                /* ======================= LER DO MEDICO (MEU FIFO) ======================= */
                fd_cliente_fifo = open(c_fifo_fname, O_RDWR);
                n = read(fd_cliente_fifo, &b, sizeof(balcao));
                if (n == -1) {
                    perror("Não conseguiu ler do meu FIFO... \n");
                    exit(1);
                }
                if(strcmp(b.msg,"adeus") == 0){
                    break;
                }
                printf("Resposta do especialista [%d] : %s\n",b.id_medico,b.msg);
            }
            if(estado_cli == 0){ //ENVIAR PARA O BALCAO
                if(strcmp(comando,"sair") == 0){
                    break;
                }
                n = write(fd_server_fifo,&b,sizeof(balcao));


            }
        } else if (res > 0 && FD_ISSET(fd_cliente_fifo, &fds)) {
            n = read(fd_cliente_fifo, &b, sizeof(balcao));
            estado_cli = 0;
            if (n == -1) {
                perror("Erro ao ler do meu FIFO");
                exit(1);

            } else if(b.registo == 2){
                estado_cli = 1;
                printf("[PID Especialista: %d] \n", b.id_medico);
                printf("=======================================================\n");


            }
        }

    }while(strcmp(str_com, "sair") != 0);
    close(fd_server_fifo);
    unlink(c_fifo_fname);
    return 0;
}





    //1º - Só aceita executar se o balcão estiver em funcionamento : (Enviar um sinal para o balca para ver se está a correr ?)


    //1.2º - Enviar sintomas ao balcao


    //2º - Obter do balcao: area de especialidade, prioridade, numero de utentes nesse instante à frente dele nessa especialidade, numero de especialista dessa area


    //3º - Aguardar que seja encaminhado para um especialista (programa medico)

    //4º - Diálogo com o especialista (programa médico)





