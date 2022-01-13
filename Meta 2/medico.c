#include "utilis.h"
#include "func_balcao.h"

void acorda(int s, siginfo_t *info, void* uc){}

void *temporizador(void *dados){
    char cmd[100];
    int fd;

    balcao *td = (balcao *) dados;
    do{
        sleep(20);
        fd = open(FIFO_SINAL, O_WRONLY);
        td->cliente = 0;
        write(fd, td, sizeof(balcao));
        close(fd);

    }while(td->sair == 0);
    pthread_exit(NULL);
}


int main(int argc, char *argv[]) {
    char m_fifo_fname[25],c_fifo_fname[25], comando[30];
    int fd_server_fifo, fd_cliente_fifo,res,n,fd_novo;
    fd_set fds;
    pthread_t tid;
    struct timeval tempo;
    int estado = 0; // 0 -> Nao registado | 1 -> Registado | 2 -> estou em consulta

    balcao b;

    char *especialidade[5] = {"geral","ortopedia","estomatologia","neurologia","oftalmologia"};

    /* ======================= VERIFICAR SE O BALCAO ESTA A CORRER ======================= */

    if (access(bal_FIFO, F_OK) != 0) {
        perror("[ERRO] O balcão não está a executar\n");
        exit(1);
    }
    if (argc < 3) {
        perror("[ERRO] Faltam argumentos!\n");
        exit(1);
    }
    b.cliente = 0;
    b.consulta = 0;
    b.sair = 0;
    strcpy(b.nome_medico, argv[1]);
    strcpy(b.especialidade, argv[2]);
    //printf("%s\n",b.especialidade);

    /* ======================= VERIFICAR SE A ESPECIALIDADE E VALIDA =============================*/
    /*for(int i = 0; i <= 5; i++){
        if(strcmp(b.especialidade,especialidade[i]) != 0){
            printf("%s\n",especialidade[i]);
            printf("Introduza uma especialidade válida!\n");
            exit(1);
        }

    }*/

    /* ======================= Cria o FIFO do Medico =============================*/
    b.id_medico = getpid();
    sprintf(m_fifo_fname, MEDICO_FIFO, b.id_medico);
    if (mkfifo(m_fifo_fname, 0600) == -1) {
        perror("\n[ERRO] ao tentar criar FIFO deste médico\n");
        exit(EXIT_FAILURE);
    }
    fprintf(stderr, "\n[FIFO] do médico criado\n");
    /* ======================= FIFO do Médico (aberto para leitura e escrita) ============================= */

    if((fd_cliente_fifo = open(m_fifo_fname, O_RDWR)) == -1){ /* bloqueante */
        perror("\n[ERRO] ao abrir FIFO do médico");
        unlink(m_fifo_fname);
        exit(EXIT_FAILURE);
    };
    fprintf(stderr,"Abri o fifo do médico...\n");

    /* ======================= Abre o FIFO do Servidor p/ Escrita ======================= */
    if ((fd_server_fifo = open(bal_FIFO, O_WRONLY)) == -1) { /* bloqueante */
        fprintf(stderr, "\n[ERRO] O Servidor não está a correr\n");
        unlink(m_fifo_fname);
        exit(EXIT_FAILURE);
    };
    printf("Abri o fifo do servidor...\n");

    /* ======================= ENVIAR ESPECIALIDADE  E NOME AO BALCAO ======================= */
    n = write(fd_server_fifo, &b, sizeof(balcao));
    if(n == -1){
        printf("Não consegui eescrever no FIFO do servidor");
        exit(1);
    }
    printf("Enviei ao balcao %d bytes ...\n", n);
    pthread_create(&tid, NULL, temporizador, (void* ) &b);
    do{
        printf("Estado: %d\n",estado);
        /* ======================= SELECT ======================= */
        /* ======================= PREPARAR DESCRITORES QUE QUERO MONOTORIZAR ============== */

        FD_ZERO(&fds); //LIMPAR DESCRITORES
        FD_SET(0,&fds); // TECLADO
        FD_SET(fd_cliente_fifo,&fds);
        tempo.tv_sec = 8; // TIMEOUT
        tempo.tv_usec = 0;

        res = select(fd_cliente_fifo+1,&fds,NULL,NULL,&tempo);
        if(res == 0){
           //printf("\nEstou sem fazer nada...\n");
        }
        else if(res > 0) {
            if (FD_ISSET(0, &fds)) {
                fgets(b.msg, sizeof(b.msg), stdin);

                if (estado == 1) { //Registado //enviar comando para o balcao(se nao estiver numa consulta)
                    b.cliente = 0;
                    if(strcmp(b.msg,"sair\n")==0){
                        n = write(fd_server_fifo, &b, sizeof(balcao));
                        if (n == -1) {
                            printf("\nNão conseguiu escrever no FIFO do servidor...\n");
                            exit(1);
                        }
                        close(fd_server_fifo);
                        break;
                    }
                } else {
                    /* ======================= ABRIR FIFO DO CLIENTE ======================= */
                    printf("entrei no estado 2\n");
                    sprintf(c_fifo_fname, CLIENT_FIFO, b.id_utente);
                    b.cliente = 0;
                    b.consulta = 1;
                    if(strcmp(b.msg,"adeus\n") == 0){
                        b.consulta = -1;
                        estado = 1;
                        
                        n = write(fd_server_fifo,&b, sizeof(balcao));
                        if (n == -1) {
                            perror("\nNão conseguiu escrever no FIFO do servidor...\n");
                            exit(1);
                        }
                    }
                    fd_novo = open(c_fifo_fname, O_RDWR);
                    printf("Abri o Fifo do cliente\n");
                    /* ======================= ENVIAR AO CLIENTE ======================= */
                    b.id_medico = getpid();
                    n = write(fd_novo, &b, sizeof(balcao));
                    if (n == -1) {
                        perror("\nNão conseguiu escrever no FIFO do cliente...\n");
                        exit(1);
                    }
                    close(fd_novo);
                    printf("Enviei %d  bytes...\n", n);
                }
            }
            if (FD_ISSET(fd_cliente_fifo, &fds)) {
                n = read(fd_cliente_fifo, &b, sizeof(balcao));
                if (n == -1) {
                    printf("\nNão conseguiu ler...");
                    exit(1);
                }
                printf("Recebi %d bytes de alguem ...\n", n); //balcao ou cliente

                if (b.cliente == 1 && b.consulta == 1) {
                    estado = 2;
                    printf("\nUtente [%d] : %s\n", b.id_utente, b.msg);


                } else if(b.registo_medico == 1){
                    estado = 1;
                    printf("Especialista [%d] foi registado\n", b.id_medico);
                }
                else if(b.flagB == 1 && b.cliente == 0){
                    printf("Tens clientes para atender ...\n");
                    printf("\n=============================\n");
                    printf("Dados utente :\n"
                           "Nome: %s \t Pid: %d \t Sintomas: %s\n",b.nome_utente,b.id_utente,b.sintoma);
                    printf("\n=============================\n");
                    estado = 2;
                }
                else if(b.saiCli == -1){
                        estado = 1;
                        printf("O Utente [%d] terminou a consulta\n",b.id_utente);

                }

                /*if (strcmp(b.msg, "Esta ligado ao balcao!") == 0 &&
                    b.cheio == 0) { //filas cheias (nMedicos > MaxMedicos)
                    estado = 0;
                    printf("Nao foi possivel o registo devido a filas cheias ou especialidade invalida - tente mais tarde\n");
                }
                if (strcmp(b.msg, "Esta ligado ao balcao!") == 0 && b.registo_utente == 2) {
                    estado = 2;
                    b.consulta = 1;

                    printf("Consulta vai iniciar\n");
                    printf("Dados do Utente : \n");
                    printf("PID = %d \t Nome= %s \t Sintoma = %s\n", b.id_utente, b.nome_utente, b.sintoma);
                    printf("\n----------------------------------------------------\n");
                    printf("Pode escrever alguma coisa ao utente\n");
                }
                if (b.cheio == 0) {
                    printf("\nO balcão não consegue atender mais medicos - volte mais tarde!\n");
                    exit(1);
                }
                if (b.registo_medico == -1) { //Não há utentes para essa especialiade
                    printf("%s\n", b.msg);
                }*/
            }
        }
    }while(strcmp(b.msg, "sair\n") != 0);
    close(fd_cliente_fifo);

    b.sair =1;
    pthread_join(tid, NULL);

    unlink(m_fifo_fname);
    exit(0);
}

    //1º - Só aceita executar se o balcão estiver em funcionamento;

    // 1.2º - Interagir com o balcao para registar nome e especialidade



    //3º - Fico a aguardar que o balcao me indique que tenho um novo utente para atender


    //4º - Diálogo com o utente (especialista escreve pergunta e aguarda pela resposta)



