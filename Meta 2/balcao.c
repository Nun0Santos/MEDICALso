#include "utilis.h"

void acorda(int s, siginfo_t *info, void* uc){}

void* mostraListas(void* dados){
    thbalcao *td = (thbalcao* ) dados;

    do {
        sleep(td->tempo);
        pthread_mutex_lock(td->trinco);
        if(td->ite_cli > 0 || td->ite_med > 0){
            printf("\n\n====================== Listar Clientes e Medicos ======================\n");
            for (int i = 0; i < td->ite_cli; ++i) {
                printf("\nCliente %d:", i);
                printf("\n\tPID: %d", td->p_cli[i].id_utente);
                printf("\n\tClassificacao: %s %d", td->p_cli[i].classificao, td->p_cli[i].prioridade);
                printf("\n\tEsta em consulta? (0->nao | 1->sim): %d", td->p_cli[i].consulta);
            }
            for (int i = 0; i < td->ite_med; ++i) {
                printf("\nMedico %d:", i);
                printf("\n\tPID: %d", td->p_med[i].id_medico);
                printf("\n\tEspecialidade: %s", td->p_med[i].especialidade);
                printf("\n\tTemporizador: %d", td->p_med[i].temp);
                printf("\n\tEsta em consulta? (0->nao | 1->sim): %d", td->p_med[i].consulta);
                ++td->p_med[i].temp;
            }
            printf("\n\n========================================================================\n");
        }
        pthread_mutex_unlock(td->trinco);
    }while(td->continua == 1);

    pthread_exit(NULL);
}
void* apagaMed(void* dados){
    thbalcao *td = (thbalcao* ) dados;

    do {
        sleep(2);
        pthread_mutex_lock(td->trinco);

        if(td->ite_med > 0){
            for (int i = 0; i < td->ite_med; ++i) {
                if(td->p_med[i].temp <= 0){
                    for (int j = i; j < td->ite_med - 1; ++j) {
                        td->p_med[j] = td->p_med[j+1];
                    }
                    --td->ite_med;
                }else{
                    td->p_med[i].temp -= 2;
                }
            }
        }
        pthread_mutex_unlock(td->trinco);
    }while(td->continua);
    pthread_exit(NULL);
}


int main() {
    char sintoma[256], resposta[256];
    int res, tam = 0, n, i, MaxClientes, MaxMedicos, nClientes = 0, nMedicos = 0, n_write,maxfd;
    char *MaxClientes_str, *MaxMedicos_str, str_com[40];
    int balcaoToClassificador[2], ClassificadorToBalcao[2];
    pipe(balcaoToClassificador);
    pipe(ClassificadorToBalcao);

    int fd_cliente_fifo, fd_server_fifo, fd_sinal;
    char c_fifo_fname[50]; // Nome do fifo do cliente
    char m_fifo_fname[50]; // Nome do fifo do medico

    balcao clientes[5]; //array de estrutura  para utentes
    balcao medicos[5]; //array de estrutura para medicos
    balcao b;
    thbalcao t;

    fd_set fds;
    struct timeval tempo;
    pthread_mutex_t mutex;
    pthread_t tid[2];

    struct sigaction act;
    act.sa_sigaction = acorda;
    act.sa_flags = SA_SIGINFO;
    sigaction(SIGUSR2, &act, NULL);


    //Thread
    pthread_mutex_t trinco;
    if (pthread_mutex_init(&trinco, NULL) != 0) {
        printf("\nErro na inicializa????o do mutex\n");
        return 1;
    }
    setbuf(stdout, NULL);

    /* ======================= VERIFICAR SE O BALCAO ESTA A CORRER ======================= */

    if (access(bal_FIFO, F_OK) == 0) {
        printf("Balc??o j?? est??  executar\n");
        exit(1);
    }
    if (mkfifo(bal_FIFO, 0600) == -1) {
        perror("\n[ERRO] ao tentar criar FIFO do balc??o\n");
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
        printf("[ERRO] O FIFO j?? existe\n");
        exit(1);
    }
    if (mkfifo(FIFO_SINAL, 0600) == -1) {
        perror("\n[ERRO] ao tentar criar FIFO para a rececao do sinal\n");
        exit(EXIT_FAILURE);
    }
    fprintf(stderr, "\n[FIFO] do servidor(SINAL) criado\n");

    fd_sinal = open(FIFO_SINAL, O_RDWR);
    if (fd_sinal == -1) {
        perror("\nImpossivel abrir o FIFO");
        exit(1);
    }

    /* ======================= Vari??veis Ambiente ======================= */

    MaxClientes_str = getenv("MAXCLIENTES");
    if (MaxClientes_str) {
        MaxClientes = atoi(MaxClientes_str);
        t.maxClientes = MaxClientes;
    } else {
        printf("Erro ao ler MAXCLIENTES\n");
        return -1;
    }
    printf("\nMAXCLIENTE = %d\n", MaxClientes);

    MaxMedicos_str = getenv("MAXMEDICOS");
    if (MaxMedicos_str) {
        MaxMedicos = atoi(MaxMedicos_str);
        t.maxMedicos = MaxMedicos;
    } else {
        printf("Erro ao ler MaxMedicos\n");
        return -1;
    }
    printf("MAXMEDICOS = %d\n", MaxMedicos);

    /* ======================= Novo Processo para lidar com o classificador ======================= */
    res = fork();
    if (res == 0) {
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

    //THREAD 1
    t.continua = 1; //Permitir Mostrar a lista de Clientes e Medicos
    t.tempo = 10;
    t.trinco = &trinco;
    pthread_create(&tid[0], NULL, mostraListas, (void *) &t);

    //THREAD 3
    t.continua = 1; //Permitir apagar o medico
    t.tempo = 20;
    t.trinco = &trinco;
    pthread_create(&tid[1], NULL, apagaMed, (void *) &t);


    do {
        /* ======================= SELECT ======================= */
        //printf("Comando?\n");

        /* ======================= PREPARAR DESCRITORES QUE QUERO MONOTORIZAR ============== */
        FD_ZERO(&fds);
        FD_SET(0, &fds);
        FD_SET(fd_server_fifo, &fds);
        FD_SET(fd_sinal, &fds);
        tempo.tv_sec = 10;
        tempo.tv_usec = 0;

        maxfd = (fd_server_fifo > fd_sinal) ? fd_server_fifo : fd_sinal;

        res = select(maxfd + 1, &fds, NULL, NULL, &tempo);
        if (res == 0) {
            printf("Estou sem nada para fazer\n");
        } else if (res > 0) {
            if (FD_ISSET(0, &fds)) {
                /* ======================= TECLADO ======================= */
                fgets(str_com, sizeof(str_com), stdin);

                char *ptr, str1[20];
                ptr = strtok(str_com, " ");
                strcpy(str1, ptr);
                ptr = strtok(NULL, " ");


                if (strcmp(str_com, "encerra\n") == 0) {
                    printf("\nA terminar o programa...\n");

                    strcpy(str_com, "sair\n");
                }
                if (strcmp(str1, "utentes\n") == 0) {
                    pthread_mutex_lock(&trinco);
                    if (t.ite_cli == 0) {
                        printf("\nNao ha clientes\n");
                        pthread_mutex_unlock(&trinco);
                        continue;
                    }
                    for (int j = 0; j < t.ite_cli; ++j) {
                        printf("\nCliente %d:", j);
                        printf("\n\tPID: %d", t.p_cli[j].id_utente);
                        printf("\n\tClassificacao: %s %d", t.p_cli[j].classificao, t.p_cli[j].prioridade);
                        printf("\n\tEsta em consulta? (0->nao | 1->sim): %d\n", t.p_cli[j].consulta);
                    }
                    pthread_mutex_unlock(&trinco);
                }

                if (strcmp(str1, "especialistas\n") == 0) {
                    pthread_mutex_lock(&trinco);
                    if (t.ite_med == 0) {
                        printf("\nNao ha medicos\n");
                        pthread_mutex_unlock(&trinco);
                        continue;
                    }
                    for (int j = 0; j < t.ite_med; ++j) {
                        printf("\nMedico %d:", i);
                        printf("\n\tPID: %d", t.p_med[j].id_medico);
                        printf("\n\tEspecialidade: %s", t.p_med[j].especialidade);
                        printf("\n\tTemporizador: %d", t.p_med[j].temp);
                        printf("\n\tEsta em consulta? (0->nao | 1->sim): %d\n", t.p_med[j].consulta);
                    }
                    pthread_mutex_unlock(&trinco);
                }
                if (strcmp(str1, "delut") == 0) {
                    pthread_mutex_lock(&trinco);
                    int x = atoi(ptr);
                    int flag = 0;

                    for (int j = 0; j < t.ite_cli; ++j) {
                        if (t.p_cli[j].id_utente == x && t.p_cli[i].consulta == 0) {
                            b = t.p_cli[j];
                            flag = 1;
                            for (int k = j; k < t.ite_cli - 1; ++k) {
                                t.p_cli[k] = t.p_cli[k + 1];
                            }
                            --t.ite_cli;
                        } else {
                            printf("\nO cliente ou est?? em consulta ou ent??o n??o existe\n");
                        }
                    }
                    pthread_mutex_unlock(&trinco);

                    if (flag == 1) {
                        b.remove =1;

                        sprintf(c_fifo_fname, CLIENT_FIFO, b.id_utente);
                        fd_cliente_fifo = open(c_fifo_fname, O_WRONLY);
                        printf("Abri o Fifo do cliente \n");

                        /* ======================= ENVIAR RESPOSTA AO CLIENTE ======================= */
                        n = write(fd_cliente_fifo, &b, sizeof(balcao));
                        if (n == -1) {
                            printf("\nN??o conseguiu escrever...");
                            exit(1);
                        }
                        close(fd_cliente_fifo);
                    }
                }
                if (strcmp(str1, "delesp") == 0) {
                    pthread_mutex_lock(&trinco);

                    int x = atoi(ptr);
                    int flag = 0;

                    for (int j = 0; j < t.ite_med; ++j) {
                        if (t.p_med[j].id_medico == x && t.p_med[i].consulta == 0) {
                            b = t.p_med[j];
                            flag = 1;
                            for (int k = j; k < t.ite_med - 1; ++k) {
                                t.p_med[k] = t.p_med[k + 1];
                            }
                            --t.ite_med;
                        } else {
                            printf("\nO medico ou est?? em consulta ou ent??o n??o existe");
                        }
                    }
                    pthread_mutex_unlock(&trinco);

                    if (flag == 1) {
                        b.sair = 1;
                        b.remove = 1;
                        sprintf(m_fifo_fname, MEDICO_FIFO, b.id_medico);
                        fd_cliente_fifo = open(m_fifo_fname, O_WRONLY);
                        n_write = write(fd_cliente_fifo, &b, sizeof(balcao));
                        if (n_write == -1) {
                            printf("\nN??o conseguiu escrever...");
                            exit(1);
                        }
                        close(fd_cliente_fifo);
                    }
                }
                if (strcmp(str_com, "freq") == 0) {
                    int x = atoi(ptr);
                    pthread_mutex_lock(&trinco);

                    t.tempo = x;
                    printf("\nMudou a frequencia para %d segundos\n", t.tempo);

                    pthread_mutex_unlock(&trinco);
                }

            }

        }
        if (FD_ISSET(fd_server_fifo, &fds)) {
            /* ======================= RECEBE STRUCT BALCAO ======================= */
            n = read(fd_server_fifo, &b, sizeof(balcao));
            if (n == -1) {
                printf("\nN??o conseguiu ler...");
                exit(1);
            }
            printf("Recebi %d bytes ....\n", n);

            if (n == sizeof(balcao)) {
                if (b.cliente == 1) {
                    if (t.ite_cli <= t.maxClientes - 1) {

                        if (strcmp(b.msg, "sair\n") == 0) {
                            printf("O utente [%d] desistiu\n", t.p_cli->id_utente);
                            t.p_cli->consulta = 0;
                            b.consulta = 0;
                            continue;
                        }
                        b.registo_utente = 1; // Est?? registado
                        b.cheio = 1;

                        printf("Novo utente [%d] com os sintoma: %s\n", b.id_utente, b.sintoma);

                        /* ======================= ENVIAR SINTOMAS AO CLASSIFICADOR ======================= */
                        strcpy(sintoma, b.sintoma);
                        n_write = write(balcaoToClassificador[1], sintoma, strlen(sintoma));

                        if (n_write == -1) {
                            printf("\nN??o conseguiu escrever...");
                            exit(1);
                        }
                        tam = read(ClassificadorToBalcao[0], resposta, sizeof(resposta) - 1);

                        if (tam == -1) {
                            printf("\nN??o conseguiu ler...");
                            exit(1);
                        }
                        resposta[tam] = '\0';
                        /* ======================= SEPARAR RESPOSTA ======================= */
                        sscanf(resposta, "%s %d", b.classificao, &b.prioridade);
                        strcpy(b.sintoma, sintoma);

                        /* ======================= ABRIR FIFO DO CLIENTE ======================= */
                        sprintf(c_fifo_fname, CLIENT_FIFO, b.id_utente);
                        fd_cliente_fifo = open(c_fifo_fname, O_WRONLY);
                        printf("Abri o Fifo do cliente \n");

                        /* ======================= ENVIAR RESPOSTA AO CLIENTE ======================= */
                        n = write(fd_cliente_fifo, &b, sizeof(balcao));
                        if (n == -1) {
                            printf("\nN??o conseguiu escrever...");
                            exit(1);
                        }
                        close(fd_cliente_fifo);

                        t.p_cli[t.ite_cli] = b;
                        ++t.ite_cli;

                        clientes[nClientes] = b;
                        nClientes++;

                        b.flagOcupado = 0;
                        t.p_med->med_ocupado = 0;

                        printf("\nN. clientes: %d\n", t.ite_cli);
                        printf("Enviei %d  bytes ao utente...\n", n);

                        /* ======================= PERCORRER ARRAY MEDICOS ======================= */
                        for(int i = 0; i<t.ite_med; ++i){
                            if(strcmp(b.classificao,medicos[i].especialidade) == 0){
                                if(t.p_med[i].consulta == 0 && t.p_med[i].med_ocupado == 0){
                                    t.p_med[i].cliente = 1;
                                    t.p_med[i].flagB = 2;
                                    t.p_med[i].consulta =1;
                                    t.p_med[i].med_ocupado = 1;
                                    sprintf(c_fifo_fname, CLIENT_FIFO, b.id_utente);
                                    fd_cliente_fifo = open(c_fifo_fname, O_WRONLY);
                                    n_write = write(fd_cliente_fifo, &t.p_med[i], sizeof(balcao));
                                    if (n_write == -1) {
                                        printf("\nN??o consegui escrever no FIFO do cliente\n");
                                        exit(1);
                                    }
                                    close(fd_cliente_fifo);
                                    printf("Enviei %d  bytes ao cliente...\n", n);
                                }else{
                                    strcpy(t.p_med[i].msg,"O medico dessa especialidade esta ocupado\n");
                                    t.p_med[i].consulta =0;
                                    t.p_med[i].flagOcupado = 1;
                                    t.p_med[i].flagB = 0;
                                    sprintf(c_fifo_fname, CLIENT_FIFO, b.id_utente);
                                    fd_cliente_fifo = open(c_fifo_fname, O_WRONLY);
                                    n_write = write(fd_cliente_fifo, &t.p_med[i], sizeof(balcao));
                                    if (n_write == -1) {
                                        printf("\nN??o consegui escrever no FIFO do cliente\n");
                                        exit(1);
                                    }
                                    close(fd_cliente_fifo);
                                    printf("Enviei %d  bytes ao cliente...\n", n);
                                    continue;
                                }
                            }
                        }
                    } else {
                        b.cheio = 0;
                        b.flagN = -1; //condicao para testar
                        strcpy(b.msg, "O balc??o n??o consegue atender mais clientes - volte mais tarde");
                        sprintf(c_fifo_fname, CLIENT_FIFO, b.id_utente);
                        fd_cliente_fifo = open(c_fifo_fname, O_WRONLY);
                        n_write = write(fd_cliente_fifo, &b, sizeof(balcao));
                        if (n_write == -1) {
                            printf("\nN??o consegui escrever no FIFO do cliente\n");
                            exit(1);
                        }
                        close(fd_cliente_fifo);
                        continue;
                    }
                } else {
                    if (t.ite_med <= t.maxMedicos - 1) {

                        if (strcmp(b.msg, "sair\n") == 0) {
                            printf("O especialista [%d] terminou a liga????o\n", b.id_medico);
                            continue;
                        }
                        if (strcmp(b.msg, "adeus\n") == 0) {
                            printf("O especialista [%d] terminou a consulta e esta disponivel para outra\n",b.id_medico);
                            t.p_med->consulta = 0;  b.consulta = 0;
                            continue;
                        }
                        b.registo_medico = 1;

                        sprintf(m_fifo_fname, MEDICO_FIFO, b.id_medico);
                        fd_cliente_fifo = open(m_fifo_fname, O_WRONLY);
                        n_write = write(fd_cliente_fifo, &b, sizeof(balcao));
                        if (n_write == -1) {
                            printf("\nN??o conseguiu escrever...");
                            exit(1);
                        }
                        close(fd_cliente_fifo);

                        t.p_med[t.ite_med] = b;
                        ++t.ite_med;

                        medicos[nMedicos] = b;
                        nMedicos++;

                        t.p_cli->consulta = 0; b.consulta=0;
                        b.cli_ocupado = 0;
                        printf("\nN. medicos: %d\n", t.ite_med);

                        printf("Novo especialista [%d] para a especialidade [%s]\n", b.id_medico, b.especialidade);

                        if (strcmp(b.especialidade, "geral") == 0 && b.filas[0] < 5) {
                            /* ======================= PERCORRER ARRAY DOS CLIENTES ======================= */

                            for (int i = 0; i < t.ite_cli; i++) {
                                if (strcmp(t.p_cli[i].classificao, b.especialidade) == 0) {
                                    printf("H?? cliente para essa especialidade\n");

                                        if(t.p_cli[i].consulta == 0 && t.p_cli[i].cli_ocupado == 0){
                                            t.p_cli[i].cliente = 0;
                                            t.p_cli[i].flagB = 1;
                                            t.p_cli[i].consulta=1;
                                            t.p_cli[i].cli_ocupado =1;
                                            t.p_cli[i].flagOcupado =1;

                                            /* ======================= ABRIR FIFO DO MEDICO ======================= */

                                            sprintf(m_fifo_fname, MEDICO_FIFO, b.id_medico);
                                            fd_cliente_fifo = open(m_fifo_fname, O_WRONLY);
                                            printf("Abri o Fifo do medico \n");

                                            n = write(fd_cliente_fifo, &t.p_cli[i], sizeof(balcao));
                                            if (n == -1) {
                                                printf("\nN??o conseguiu escrever...");
                                                exit(1);
                                            }
                                            close(fd_cliente_fifo);
                                            printf("Enviei %d  bytes...\n", n);

                                    }else{
                                        strcpy(t.p_cli[i].msg,"O utente dessa especialidade ja esta a ser atendido\n");
                                        t.p_cli[i].cliente = 0;
                                        t.p_cli[i].consulta =0;
                                        t.p_cli[i].flagOcupado = 1;
                                        t.p_cli[i].flagB = 0;
                                        sprintf(m_fifo_fname, MEDICO_FIFO, b.id_medico);
                                        fd_cliente_fifo = open(m_fifo_fname, O_WRONLY);
                                        printf("Abri o Fifo do medico \n");

                                        n = write(fd_cliente_fifo, &t.p_cli[i], sizeof(balcao));
                                        if (n == -1) {
                                            printf("\nN??o conseguiu escrever...");
                                            exit(1);
                                        }
                                        close(fd_cliente_fifo);
                                        printf("Enviei %d  bytes...\n", n);
                                    }

                                }
                            }
                            b.filas[0]++;

                        } else  if (strcmp(b.especialidade, "ortopedia") == 0 && b.filas[1] < 5) {
                            /* ======================= PERCORRER ARRAY DOS CLIENTES ======================= */

                            for (int i = 0; i < t.ite_cli; i++) {
                                if (strcmp(t.p_cli[i].classificao, b.especialidade) == 0) {
                                    printf("H?? cliente para essa especialidade\n");

                                    if(t.p_cli[i].consulta == 0 && t.p_cli[i].cli_ocupado == 0){
                                        t.p_cli[i].cliente = 0;
                                        t.p_cli[i].flagB = 1;
                                        t.p_cli[i].consulta=1;
                                        t.p_cli[i].cli_ocupado =1;
                                        t.p_cli[i].flagOcupado =1;

                                        /* ======================= ABRIR FIFO DO MEDICO ======================= */
                                        sprintf(m_fifo_fname, MEDICO_FIFO, b.id_medico);
                                        fd_cliente_fifo = open(m_fifo_fname, O_WRONLY);
                                        printf("Abri o Fifo do medico \n");

                                        n = write(fd_cliente_fifo, &t.p_cli[i], sizeof(balcao));
                                        if (n == -1) {
                                            printf("\nN??o conseguiu escrever...");
                                            exit(1);
                                        }
                                        close(fd_cliente_fifo);
                                        printf("Enviei %d  bytes...\n", n);

                                    }else{
                                        strcpy(t.p_cli[i].msg,"O utente dessa especialidade ja esta a ser atendido\n");
                                        t.p_cli[i].cliente = 0;
                                        t.p_cli[i].consulta =0;
                                        t.p_cli[i].flagOcupado = 1;
                                        t.p_cli[i].flagB = 0;
                                        sprintf(m_fifo_fname, MEDICO_FIFO, b.id_medico);
                                        fd_cliente_fifo = open(m_fifo_fname, O_WRONLY);
                                        printf("Abri o Fifo do medico \n");

                                        n = write(fd_cliente_fifo, &t.p_cli[i], sizeof(balcao));
                                        if (n == -1) {
                                            printf("\nN??o conseguiu escrever...");
                                            exit(1);
                                        }
                                        close(fd_cliente_fifo);
                                        printf("Enviei %d  bytes...\n", n);
                                    }

                                }
                            }
                            b.filas[1]++;

                        } else  if (strcmp(b.especialidade, "neurologia") == 0 && b.filas[3] < 5) {
                            /* ======================= PERCORRER ARRAY DOS CLIENTES ======================= */

                            for (int i = 0; i < t.ite_cli; i++) {
                                if (strcmp(t.p_cli[i].classificao, b.especialidade) == 0) {
                                    printf("H?? cliente para essa especialidade\n");

                                    if(t.p_cli[i].consulta == 0 && t.p_cli[i].cli_ocupado == 0){
                                        t.p_cli[i].cliente = 0;
                                        t.p_cli[i].flagB = 1;
                                        t.p_cli[i].consulta=1;
                                        t.p_cli[i].cli_ocupado =1;
                                        t.p_cli[i].flagOcupado =1;

                                        /* ======================= ABRIR FIFO DO MEDICO ======================= */

                                        sprintf(m_fifo_fname, MEDICO_FIFO, b.id_medico);
                                        fd_cliente_fifo = open(m_fifo_fname, O_WRONLY);
                                        printf("Abri o Fifo do medico \n");

                                        n = write(fd_cliente_fifo, &t.p_cli[i], sizeof(balcao));
                                        if (n == -1) {
                                            printf("\nN??o conseguiu escrever...");
                                            exit(1);
                                        }
                                        close(fd_cliente_fifo);
                                        printf("Enviei %d  bytes...\n", n);

                                    }else{
                                        strcpy(t.p_cli[i].msg,"O utente dessa especialidade ja esta a ser atendido\n");
                                        t.p_cli[i].cliente = 0;
                                        t.p_cli[i].consulta =0;
                                        t.p_cli[i].flagOcupado = 1;
                                        t.p_cli[i].flagB = 0;
                                        sprintf(m_fifo_fname, MEDICO_FIFO, b.id_medico);
                                        fd_cliente_fifo = open(m_fifo_fname, O_WRONLY);
                                        printf("Abri o Fifo do medico \n");

                                        n = write(fd_cliente_fifo, &t.p_cli[i], sizeof(balcao));
                                        if (n == -1) {
                                            printf("\nN??o conseguiu escrever...");
                                            exit(1);
                                        }
                                        close(fd_cliente_fifo);
                                        printf("Enviei %d  bytes...\n", n);
                                    }

                                }
                            }
                            b.filas[3]++;

                        } else  if (strcmp(b.especialidade, "estomatologia") == 0 && b.filas[3] < 5) {
                        /* ======================= PERCORRER ARRAY DOS CLIENTES ======================= */

                        for (int i = 0; i < t.ite_cli; i++) {
                            if (strcmp(t.p_cli[i].classificao, b.especialidade) == 0) {
                                printf("H?? cliente para essa especialidade\n");

                                if(t.p_cli[i].consulta == 0 && t.p_cli[i].cli_ocupado == 0){
                                    t.p_cli[i].cliente = 0;
                                    t.p_cli[i].flagB = 1;
                                    t.p_cli[i].consulta=1;
                                    t.p_cli[i].cli_ocupado =1;
                                    t.p_cli[i].flagOcupado =1;
                                    printf("t.consulta %d\n",t.p_cli[i].consulta);

                                    /* ======================= ABRIR FIFO DO MEDICO ======================= */

                                    sprintf(m_fifo_fname, MEDICO_FIFO, b.id_medico);
                                    fd_cliente_fifo = open(m_fifo_fname, O_WRONLY);
                                    printf("Abri o Fifo do medico \n");

                                    n = write(fd_cliente_fifo, &t.p_cli[i], sizeof(balcao));
                                    if (n == -1) {
                                        printf("\nN??o conseguiu escrever...");
                                        exit(1);
                                    }
                                    close(fd_cliente_fifo);
                                    printf("Enviei %d  bytes...\n", n);

                                }else{
                                    strcpy(t.p_cli[i].msg,"O utente dessa especialidade ja esta a ser atendido\n");
                                    t.p_cli[i].cliente = 0;
                                    t.p_cli[i].consulta =0;
                                    t.p_cli[i].flagOcupado = 1;
                                    t.p_cli[i].flagB = 0;
                                    sprintf(m_fifo_fname, MEDICO_FIFO, b.id_medico);
                                    fd_cliente_fifo = open(m_fifo_fname, O_WRONLY);
                                    printf("Abri o Fifo do medico \n");

                                    n = write(fd_cliente_fifo, &t.p_cli[i], sizeof(balcao));
                                    if (n == -1) {
                                        printf("\nN??o conseguiu escrever...");
                                        exit(1);
                                    }
                                    close(fd_cliente_fifo);
                                    printf("Enviei %d  bytes...\n", n);
                                }

                            }
                        }
                        b.filas[3]++;

                    } else  if (strcmp(b.especialidade, "oftalmologia") == 0 && b.filas[4] < 5) {
                        /* ======================= PERCORRER ARRAY DOS CLIENTES ======================= */

                        for (int i = 0; i < t.ite_cli; i++) {
                            if (strcmp(t.p_cli[i].classificao, b.especialidade) == 0) {
                                printf("H?? cliente para essa especialidade\n");

                                if(t.p_cli[i].consulta == 0 && t.p_cli[i].cli_ocupado == 0){
                                    t.p_cli[i].cliente = 0;
                                    t.p_cli[i].flagB = 1;
                                    t.p_cli[i].consulta=1;
                                    t.p_cli[i].cli_ocupado =1;
                                    t.p_cli[i].flagOcupado =1;
                                    printf("t.consulta %d\n",t.p_cli[i].consulta);

                                    /* ======================= ABRIR FIFO DO MEDICO ======================= */

                                    sprintf(m_fifo_fname, MEDICO_FIFO, b.id_medico);
                                    fd_cliente_fifo = open(m_fifo_fname, O_WRONLY);
                                    printf("Abri o Fifo do medico \n");

                                    n = write(fd_cliente_fifo, &t.p_cli[i], sizeof(balcao));
                                    if (n == -1) {
                                        printf("\nN??o conseguiu escrever...");
                                        exit(1);
                                    }
                                    close(fd_cliente_fifo);
                                    printf("Enviei %d  bytes...\n", n);

                                }else{
                                    strcpy(t.p_cli[i].msg,"O utente dessa especialidade ja esta a ser atendido\n");
                                    t.p_cli[i].cliente = 0;
                                    t.p_cli[i].consulta =0;
                                    t.p_cli[i].flagOcupado = 1;
                                    t.p_cli[i].flagB = 0;
                                    sprintf(m_fifo_fname, MEDICO_FIFO, b.id_medico);
                                    fd_cliente_fifo = open(m_fifo_fname, O_WRONLY);
                                    printf("Abri o Fifo do medico \n");

                                    n = write(fd_cliente_fifo, &t.p_cli[i], sizeof(balcao));
                                    if (n == -1) {
                                        printf("\nN??o conseguiu escrever...");
                                        exit(1);
                                    }
                                    close(fd_cliente_fifo);
                                    printf("Enviei %d  bytes...\n", n);
                                }

                            }
                        }
                        b.filas[4]++;

                    } else {
                            /* ======================= ENVIAR PARA MEDICO QUE NAO H?? UTENTES PARA ESSA ESPECIALIDADE ======================= */

                            b.registo_medico = -1;
                            strcpy(b.msg, "N??o h?? utentes para essa especialdiade ou especialidade invalida!\n");

                            /* ======================= ABRIR FIFO DO MEDICO ======================= */

                            sprintf(m_fifo_fname, MEDICO_FIFO, b.id_medico);
                            fd_cliente_fifo = open(m_fifo_fname, O_WRONLY);
                            printf("Abri o Fifo do medico \n");
                            n = write(fd_cliente_fifo, &t.p_cli, sizeof(balcao));

                            if (n == -1) {
                                printf("N??o consegui escrever par o FIFO do m??dico\n");
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
                            printf("\nN??o consegui escrever no FIFO do m??dico...\n");
                            exit(1);
                        }
                        close(fd_cliente_fifo);
                        printf("Enviei %d  bytes...\n", n);
                        continue;
                    }
                }
            }
        }
        if (FD_ISSET(fd_sinal, &fds)) { //sinal de vida
            int n = read(fd_sinal, &b, sizeof(balcao));

            if (n == -1) {
                printf("\nNao conseguiu ler...");
                exit(1);
            }
            pthread_mutex_lock(&trinco);

            if (b.cliente == 0) {//medico quer sair
                for (int j = 0; j < t.ite_med; ++j) {
                    if (t.p_med[i].id_medico == b.id_medico) {
                        t.p_med[i].temp = 20;//deu sinal de vida, ent??o repoe o temporizador

                    }
                }
            } else {
                if (b.sair == 1) {//cliente quer sair
                    for (int j = 0; j < t.ite_cli; ++j) {
                        if (t.p_cli[j].id_utente == b.id_utente) {
                            for (int k = j; k < t.ite_cli - 1; ++k) {
                                t.p_cli[k] = t.p_cli[k + 1];
                            }
                            --t.ite_cli;
                        }
                    }
                }
            }
            pthread_mutex_unlock(&trinco);
        }
    }while(strcmp(str_com, "sair\n") != 0);
    b.sair = 1;
    t.continua = 0;

    close(fd_server_fifo);
    unlink("server_fifo");

    close(fd_sinal);
    unlink("sinal");

    //pthread_kill(tid[0], SIGUSR2);
    pthread_join(tid[0], NULL);

    //pthread_kill(tid[1], SIGUSR2);
    pthread_join(tid[1], NULL);

    //pthread_kill(tid[2], SIGUSR2);
    pthread_join(tid[2], NULL);

    pthread_mutex_destroy(&trinco);

    return 0;
}
