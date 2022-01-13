//
// Created by user on 07/11/21.
//
#include "func_balcao.h"
#include "utilis.h"

void acorda(int s, siginfo_t *info, void* uc){}

void* mostraListas(void* dados){
    thbalcao *td = (thbalcao* ) dados;

    do {
        sleep(td->tempo);
        printf("\n==========================================================\n");
                printf("\nClientes: ");
        for (int i = 0; i < td->ite_cli; ++i) {
            printf("\nCliente %d:", i);
            printf("\n\tPID: %d", td->p_cli[i].id_utente);
            printf("\n\tClassificacao: %s %d", td->p_cli[i].classificao, td->p_cli[i].prioridade);
            printf("\n\tEsta em consulta? (0->nao | 1->sim): %d\n", td->p_cli[i].consulta);
        }
                printf("\nMedicos: ");
        for (int i = 0; i < td->ite_med; ++i) {
            printf("\nMedico %d:", i);
            printf("\n\tPID: %d", td->p_med[i].id_medico);
            printf("\n\tEsta em consulta? (0->nao | 1->sim): %d\n", td->p_med[i].consulta);
            ++td->p_med[i].temp;
        }
        printf("\n==========================================================\n");
    }while(td->continua == 1);

    pthread_exit(NULL);
}

int main() {
    char comando[100], sintoma[256], resposta[256];
    int res, tam = 0, n, MaxClientes, MaxMedicos, nClientes = 0, nMedicos = 0,n_write;
    char *MaxClientes_str, *MaxMedicos_str,str_com[40]; //Var ambiente
    int maxfd;
    int balcaoToClassificador[2],ClassificadorToBalcao[2];
    pipe(balcaoToClassificador);
    pipe(ClassificadorToBalcao); //Pipe retorno

    int fd_cliente_fifo, fd_server_fifo,fd_sinal;
    char c_fifo_fname[50]; // Nome do fifo do cliente
    char m_fifo_fname[50]; // Nome do fifo do medico

    fd_set fds;
    struct timeval tempo;
    pthread_t tid;
    thbalcao t;
    struct sigaction act;
    act.sa_sigaction = acorda;
    act.sa_flags = SA_SIGINFO;
    sigaction(SIGUSR2, &act, NULL);

    setbuf(stdout, NULL);

    balcao clientes[5]; //array de estrutura  para utentes
    balcao medicos[5]; //array de estrutura para medicos
    balcao b;

    /* ======================= VERIFICAR SE O BALCAO ESTA A CORRER ======================= */

    if (access(bal_FIFO, F_OK) == 0) {
        printf("Balcão já está  executar\n");
        exit(1);
    }
    if (mkfifo(bal_FIFO, 0600) == -1) {
        perror("\n[ERRO] ao tentar criar FIFO do balcão\n");
        exit(EXIT_FAILURE);
    }
    fprintf(stderr, "\n[FIFO] do servidor criado\n");

    /* ======================= Abrir FIFO do Servidor ======================= */

    fd_server_fifo = open(bal_FIFO, O_RDWR);
    if ((fd_server_fifo) == -1) {
        perror("\n[Erro] ao abrir o FIFO do servidor\n");
        exit(EXIT_FAILURE);
    }
    fprintf(stderr, "\nFIFO aberto para READ (+WRITE) bloqueante\n");

    /* ======================= Criar/Abrir FIFO para o sinal ======================= */
    if (access(FIFO_SINAL, F_OK) == 0) {
        printf("[ERRO] O FIFO já existe\n");
        exit(1);
    }
    if(mkfifo(FIFO_SINAL, 0600) == -1){
        perror("\n[ERRO] ao tentar criar FIFO para a rececao do sinal\n");
        exit(EXIT_FAILURE);
    }
    fprintf(stderr, "\n[FIFO] do servidor(SINAL) criado\n");

    fd_sinal = open(FIFO_SINAL, O_RDWR);
    if (fd_sinal == -1) {
        perror("\nImpossivel abrir o FIFO");
        exit(1);
    }

    /* ======================= Variáveis Ambiente ======================= */
    MaxClientes_str = getenv("MAXCLIENTES");
    if (MaxClientes_str) {
        MaxClientes = atoi(MaxClientes_str);
        t.maxClientes = MaxClientes;
    } else {
        printf("Erro ao ler MAXCLIENTES\n");
        encerra();
        return -1;
    }
    printf("\nMAXCLIENTE = %d\n", MaxClientes);

    MaxMedicos_str = getenv("MAXMEDICOS");
    if (MaxMedicos_str) {
        MaxMedicos = atoi(MaxMedicos_str);
        t.maxMedicos = MaxMedicos;
    } else {
        printf("Erro ao ler MaxMedicos\n");
        encerra();
        return -1;
    }
    printf("MAXMEDICOS = %d\n", MaxMedicos);


    /* ======================= Novo Processo para lidar com o classificador ======================= */
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

        execlp("./classificador", "./classificador", NULL); //VER ESPACOS
        fprintf(stderr, "Comando nao encontrado\n");
        exit(123);

    }
    //pai aguarda pelo comando e depois continua
    close(balcaoToClassificador[0]);
    close(ClassificadorToBalcao[1]);

    t.continua = 1;
    t.tempo  = 10;
    pthread_create(&tid, NULL, mostraListas, (void* ) &t);

    do{
        /* ======================= SELECT ======================= */
        //printf("Comando?\n");

        /* ======================= PREPARAR DESCRITORES QUE QUERO MONOTORIZAR ============== */
        FD_ZERO(&fds);
        FD_SET(0, &fds);
        FD_SET(fd_server_fifo, &fds);
        FD_SET(fd_sinal, &fds);
        tempo.tv_sec = 8;
        tempo.tv_usec = 0;
        maxfd = (fd_server_fifo > fd_sinal) ? fd_server_fifo: fd_sinal;

        res = select(maxfd + 1, &fds, NULL, NULL, &tempo); //Primiero campo e o descritor mais alto +1
        if (res == 0) {
           // printf("Estou sem nada para fazer\n");
        } else if (res > 0) {
            if (FD_ISSET(0, &fds)) {
                /* ======================= TECLADO ======================= */
                scanf("%s", comando);
                if (strcasecmp(comando, "sair") == 0) {
                    encerra();
                } else if (strcmp(comando, "utentes") == 0) {
                    utentes(clientes, t.ite_cli);
                } else if (strcmp(comando, "especialistas") == 0) {
                    especialistas(medicos, nMedicos);
                }

            }
            if (FD_ISSET(fd_server_fifo, &fds)) {
                /* ======================= RECEBE STRUCT BALCAO ======================= */
                n = read(fd_server_fifo, &b, sizeof(balcao));
                if (n == -1) {
                    printf("\nNão conseguiu ler...");
                    exit(1);
                }
                printf("Recebi %d bytes ....\n", n);

                if (n == sizeof(balcao)) {
                    if (b.cliente == 1) {
                        if (t.ite_cli <= t.maxClientes - 1) {
                            b.registo_utente = 1; // esta registado
                            b.cheio = 1;
                                if(strcmp(b.msg,"sair\n") == 0){
                                    printf("O utente [%d] desistiu\n",b.id_utente);
                                    t.p_cli->id_utente = 0;
                                    continue;
                                }
                            //strcpy(b.msg, "Esta ligado ao balcao!");
                            printf("Novo utente [%d] com o sintoma: %s\n", b.id_utente, b.sintoma);

                            /* ======================= ENVIAR SINTOMAS AO CLASSIFICADOR ======================= */
                            strcpy(sintoma, b.sintoma);
                            n_write = write(balcaoToClassificador[1], sintoma, strlen(sintoma));
                            if (n_write == -1) {
                                printf("\nNão conseguiu escrever...");
                                exit(1);
                            }
                            tam = read(ClassificadorToBalcao[0], resposta, sizeof(resposta) - 1);
                            if (tam == -1) {
                                printf("\nNão conseguiu ler...");
                                exit(1);
                            }
                            resposta[tam] = '\0';
                            /* ======================= SEPARAR RESPOSTA ======================= */
                            sscanf(resposta, "%s %d", b.classificao, &b.prioridade);
                            printf("O utente [%d] tem a  especialidade = [%s] com prioridade = [%d]\n", b.id_utente,
                                   b.classificao, b.prioridade);
                            strcpy(b.sintoma, sintoma);

                            /* ======================= ABRIR FIFO DO CLIENTE ======================= */
                            sprintf(c_fifo_fname, CLIENT_FIFO, b.id_utente);
                            fd_cliente_fifo = open(c_fifo_fname, O_WRONLY);
                            printf("Abri o Fifo do cliente \n");

                            /* ======================= ENVIAR RESPOSTA AO CLIENTE ======================= */
                            n = write(fd_cliente_fifo, &b, sizeof(balcao));
                            if (n == -1) {
                                printf("\nNão conseguiu escrever...");
                                exit(1);
                            }

                            close(fd_cliente_fifo);
                            t.p_cli[t.ite_cli] = b;
                            ++t.ite_cli;
                            clientes[nClientes] = b;
                            nClientes++;

                            printf("\nN. clientes: %d\n", t.ite_cli);
                            printf("Enviei %d  bytes ao utente...\n", n);

                            /* ======================= PERCORRER ARRAY MEDICOS ======================= */
                            for(int i = 0; i<t.ite_med; i++){
                                if(strcmp(t.p_cli->classificao,t.p_med->especialidade) == 0){
                                    t.p_med[i].cliente = 1;
                                    t.p_med[i].flagB = 2;

                                    sprintf(c_fifo_fname, CLIENT_FIFO, b.id_utente);
                                    fd_cliente_fifo = open(c_fifo_fname, O_WRONLY);
                                    n_write = write(fd_cliente_fifo, &t.p_med[i], sizeof(balcao));
                                    if (n_write == -1) {
                                        printf("\nNão consegui escrever no FIFO do cliente\n");
                                        exit(1);
                                    }
                                    close(fd_cliente_fifo);
                                    printf("Enviei %d  bytes ao cliente...\n", n);
                                }
                            }

                        } else {
                            b.cheio = 0;
                            strcpy(b.msg, "O balcão não consegue atender mais clientes - volte mais tarde");
                            sprintf(c_fifo_fname, CLIENT_FIFO, b.id_utente);
                            fd_cliente_fifo = open(c_fifo_fname, O_WRONLY);
                            n_write = write(fd_cliente_fifo, &b, sizeof(balcao));
                            if (n_write == -1) {
                                printf("\nNão consegui escrever no FIFO do cliente\n");
                                exit(1);
                            }
                            close(fd_cliente_fifo);
                            continue;
                        }
                    } else {
                        if (t.ite_med <= t.maxMedicos - 1) {
                            b.registo_medico = 1;
                            printf("b.msg %s \n",b.msg);
                            if (strcmp(b.msg, "sair\n") == 0) {
                                    printf("O especialista [%d] terminou a ligação\n", b.id_medico);
                                    continue;
                                }
                            if (strcmp(b.msg, "adeus\n") == 0) {
                                printf("O especialista [%d] terminou a consulta e esta disponivel para outra\n", b.id_medico);
                                continue;
                            }

                            strcpy(b.msg, "Esta ligado ao balcao!");
                            sprintf(m_fifo_fname, MEDICO_FIFO, b.id_medico);
                            fd_cliente_fifo = open(m_fifo_fname, O_WRONLY);
                            n_write = write(fd_cliente_fifo, &b, sizeof(balcao));
                            if (n_write == -1) {
                                printf("\nNão conseguiu escrever...");
                                exit(1);
                            }
                            close(fd_cliente_fifo);
                            t.p_med[t.ite_med] = b;
                            ++t.ite_med;
                            medicos[nMedicos] = b;
                            nMedicos++;
                            printf("\nN. medicos: %d\n", t.ite_med);

                            printf("Novo especialista [%d] para a especialidade [%s]\n", b.id_medico, b.especialidade);
                            printf("b.especialidade %s\n",b.especialidade);
                            printf("t.pmed %s\n",t.p_med->especialidade);

                            if (strcmp(b.especialidade, "geral") == 0 && b.filas[0] < 5) {
                                printf("Entrei no geral\n");
                                /* ======================= PERCORRER ARRAY DOS CLIENTES ======================= */
                                for (int i = 0; i < t.ite_cli; i++) {
                                    if (strcmp(t.p_cli[i].classificao, b.especialidade) == 0) {
                                        printf("Há cliente para essa especialidade\n");
                                        t.p_cli[i].cliente = 0;
                                        t.p_cli[i].flagB = 1;
                                        /* ======================= ABRIR FIFO DO MEDICO ======================= */
                                        sprintf(m_fifo_fname, MEDICO_FIFO, b.id_medico);
                                        fd_cliente_fifo = open(m_fifo_fname, O_WRONLY);
                                        printf("Abri o Fifo do medico \n");

                                        n = write(fd_cliente_fifo, &t.p_cli[i], sizeof(balcao));
                                        if (n == -1) {
                                            printf("\nNão conseguiu escrever...");
                                            exit(1);
                                        }
                                        close(fd_cliente_fifo);
                                        printf("Enviei %d  bytes...\n", n);
                                    }
                                }
                                b.filas[0]++;
                            } else if (strcmp(b.especialidade, "ortopedia") == 0 && b.filas[1] < 5) {
                                printf("Entrei na ortopedia\n");
                                /* ======================= PERCORRER ARRAY DOS CLIENTES ======================= */
                                for (int i = 0; i < t.ite_cli; i++) {
                                    if (strcmp(t.p_cli[i].classificao, b.especialidade) == 0) {
                                        printf("Há cliente para essa especialidade\n");
                                        t.p_cli[i].cliente = 0;
                                        t.p_cli[i].flagB = 1;
                                        /* ======================= ABRIR FIFO DO MEDICO ======================= */
                                        sprintf(m_fifo_fname, MEDICO_FIFO, b.id_medico);
                                        fd_cliente_fifo = open(m_fifo_fname, O_WRONLY);
                                        printf("Abri o Fifo do medico \n");

                                        n = write(fd_cliente_fifo, &t.p_cli[i], sizeof(balcao));
                                        if (n == -1) {
                                            printf("\nNão conseguiu escrever...");
                                            exit(1);
                                        }
                                        close(fd_cliente_fifo);
                                        printf("Enviei %d  bytes...\n", n);
                                    }
                                }
                                b.filas[1]++;
                            } else if (strcmp(b.especialidade, "estomatologia") == 0 && b.filas[2] < 5) {
                                printf("Entrei na estamatologia\n");
                                /* ======================= PERCORRER ARRAY DOS CLIENTES ======================= */
                                for (int i = 0; i < t.ite_cli; i++) {
                                    if (strcmp(t.p_cli[i].classificao, b.especialidade) == 0) {
                                        printf("Há cliente para essa especialidade\n");
                                        t.p_cli[i].cliente = 0;
                                        t.p_cli[i].flagB = 1;
                                        /* ======================= ABRIR FIFO DO MEDICO ======================= */
                                        sprintf(m_fifo_fname, MEDICO_FIFO, b.id_medico);
                                        fd_cliente_fifo = open(m_fifo_fname, O_WRONLY);
                                        printf("Abri o Fifo do medico \n");

                                        n = write(fd_cliente_fifo, &t.p_cli[i], sizeof(balcao));

                                        if (n == -1) {
                                            printf("\nNão conseguiu escrever...");
                                            exit(1);
                                        }
                                        close(fd_cliente_fifo);
                                        printf("Enviei %d  bytes...\n", n);
                                    }
                                }
                                b.filas[2]++;
                            } else if (strcmp(b.especialidade, "neurologia") == 0 && b.filas[3] < 5) {
                                printf("Entrei na neurologia\n");
                                /* ======================= PERCORRER ARRAY DOS CLIENTES ======================= */
                                for (int i = 0; i < t.ite_cli; i++) {
                                    if (strcmp(t.p_cli[i].classificao, b.especialidade) == 0) {
                                        printf("Há cliente para essa especialidade\n");
                                        t.p_cli[i].cliente = 0;
                                        t.p_cli[i].flagB = 1;

                                        /* ======================= ABRIR FIFO DO MEDICO ======================= */
                                        sprintf(m_fifo_fname, MEDICO_FIFO, b.id_medico);
                                        fd_cliente_fifo = open(m_fifo_fname, O_WRONLY);
                                        printf("Abri o Fifo do medico \n");

                                        n = write(fd_cliente_fifo, &t.p_cli[i], sizeof(balcao));

                                        if (n == -1) {
                                            printf("\nNão conseguiu escrever...");
                                            exit(1);
                                        }
                                        close(fd_cliente_fifo);
                                        printf("Enviei %d  bytes...\n", n);
                                    }
                                }
                                b.filas[3]++;
                            } else if (strcmp(b.especialidade, "oftalmologia") == 0 && b.filas[4] < 5) {
                                printf("Entrei na oftalmologia\n");
                                /* ======================= PERCORRER ARRAY DOS CLIENTES ======================= */
                                for (int i = 0; i < t.ite_cli; i++) {
                                    if (strcmp(t.p_cli[i].classificao, b.especialidade) == 0) {
                                        printf("Há cliente para essa especialidade\n");
                                        t.p_cli[i].cliente = 0;
                                        t.p_cli[i].flagB = 1;

                                        /* ======================= ABRIR FIFO DO MEDICO ======================= */
                                        sprintf(m_fifo_fname, MEDICO_FIFO, b.id_medico);
                                        fd_cliente_fifo = open(m_fifo_fname, O_WRONLY);
                                        printf("Abri o Fifo do medico \n");

                                        n = write(fd_cliente_fifo, &t.p_cli[i], sizeof(balcao));

                                        if (n == -1) {
                                            printf("\nNão conseguiu escrever...");
                                            exit(1);
                                        }
                                        close(fd_cliente_fifo);
                                        printf("Enviei %d  bytes...\n", n);
                                    }
                                }
                                b.filas[4]++;
                            } else {
                                /* ======================= ENVIAR PARA MEDICO QUE NAO HÁ UTENTES PARA ESSA ESPECIALIDADE ======================= */
                                b.registo_medico = -1;
                                strcpy(b.msg, "Não há utentes para essa especialdiade");
                                /* ======================= ABRIR FIFO DO MEDICO ======================= */
                                sprintf(m_fifo_fname, MEDICO_FIFO, b.id_medico);
                                fd_cliente_fifo = open(m_fifo_fname, O_WRONLY);
                                printf("Abri o Fifo do medico \n");
                                n = write(fd_cliente_fifo, &t.p_cli, sizeof(balcao));

                                if (n == -1) {
                                    printf("Não consegui escrever par o FIFO do médico\n");
                                    exit(1);
                                }
                                close(fd_cliente_fifo);
                                printf("Enviei %d  bytes...\n", n);
                            }
                        } else {
                            /* ======================= ENVIAR PARA MEDICO QUE NAO E POSSIVEL REGISTAR ======================= */
                            b.registo_medico = 0;
                            b.cheio = 0;
                            sprintf(m_fifo_fname, MEDICO_FIFO, b.id_medico);
                            fd_cliente_fifo = open(m_fifo_fname, O_WRONLY);
                            n_write = write(fd_cliente_fifo, &b, sizeof(balcao));
                            if (n_write == -1) {
                                printf("\nNão consegui escrever no FIFO do médico...\n");
                                exit(1);
                            }
                            close(fd_cliente_fifo);
                            printf("Enviei %d  bytes...\n", n);
                            continue;
                        }
                    }
                } else {
                    perror("[Erro] na receção de dados");
                    encerra();
                }
            }
            if (FD_ISSET(fd_sinal, &fds)) { //sinal de vida
                //ler pipe sinal
                read(fd_sinal,&b,sizeof(balcao));
                for(int i = 0; i < nMedicos; i++){\
                    printf("Sinal de vida do Especialista [%d]\n",b.id_medico);
                }
            }
        }
        int i;
        if( i == 0){
            if(t.ite_cli != 0 && t.ite_med != 0){
                t.p_cli[0].id_medico = t.p_med[0].id_medico;
                t.p_cli[0].consulta = 1;
                t.p_med[0].consulta = 1;
                t.p_cli[0].cliente = 0;
                //printf("\ncli: %d\tmed: %d\tmed_passado: %d\n", t.p_cli[0].id_utente, t.p_med[0].id_medico, t.p_cli[0].id_medico);

                /*fd_cliente_fifo = open(c_fifo_fname, O_WRONLY);
                n_write = write(fd_cliente_fifo, &t.p_cli[0], sizeof(balcao));
                if(n_write == -1){
                    printf("\nNao conseguiu mandar a informação para o cliente...");
                    close(fd_cliente_fifo);
                    break;
                }
                close(fd_cliente_fifo);
                ++i;*/
            }
        }
    }while(strcmp(str_com, "sair\n") != 0);
    close(fd_server_fifo);
    unlink("server_fifo");

    close(fd_sinal);
    unlink("sinal");

    b.sair = 1;
    pthread_kill(tid, SIGUSR2);
    pthread_join(tid, NULL);
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

