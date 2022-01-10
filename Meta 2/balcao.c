//
// Created by user on 07/11/21.
//
#include "func_balcao.h"
#include "utilis.h"


int main() {
    char comando[100], especialidade[256], sintoma[256], resposta[256];
    int res, tam = 0, n, prioridade = 0, MaxClientes, MaxMedicos, nClientes = 0, nMedicos = 0,n_write;
    char *MaxClientes_str, *MaxMedicos_str,str_com[40]; //Var ambiente

    int balcaoToClassificador[2],ClassificadorToBalcao[2];
    pipe(balcaoToClassificador);
    pipe(ClassificadorToBalcao); //Pipe retorno

    int fd_cliente_fifo, fd_server_fifo;
    char c_fifo_fname[50]; // Nome do fifo do cliente
    char m_fifo_fname[50]; // Nome do fifo do medico
    char *sintomas[5] ={"geral","ortopedia","estamatologia","neurologia","oftalmologia"};

    fd_set fds;
    struct timeval tempo;

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

    /* ======================= Variáveis Ambiente ======================= */
    MaxClientes_str = getenv("MAXCLIENTES");
    if (MaxClientes_str) {
        MaxClientes = atoi(MaxClientes_str);
    } else {
        printf("Erro ao ler MAXCLIENTES\n");
        encerra();
        return -1;
    }
    printf("\nMAXCLIENTE = %d\n", MaxClientes);

    MaxMedicos_str = getenv("MAXMEDICOS");
    if (MaxMedicos_str) {
        MaxMedicos = atoi(MaxMedicos_str);
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

    do{
        /* ======================= SELECT ======================= */
        printf("Comando?\n");

        /* ======================= PREPARAR DESCRITORES QUE QUERO MONOTORIZAR ============== */
        FD_ZERO(&fds);
        FD_SET(0, &fds);
        FD_SET(fd_server_fifo, &fds);

        tempo.tv_sec = 8;
        tempo.tv_usec = 0;

        res = select(fd_server_fifo + 1, &fds, NULL, NULL, &tempo); //Primiero campo e o descritor mais alto +1
        if (res == 0) {
           // printf("Estou sem nada para fazer\n");
        } else if (res > 0 && FD_ISSET(0, &fds)) {
            /* ======================= TECLADO ======================= */
            scanf("%s", comando);
            if(strcasecmp(comando,"sair") == 0){
                encerra();
            }
            else if(strcmp(comando,"utentes") == 0){
                utentes(&clientes[5],nClientes);
            }
            else if(strcmp(comando,"especialistas") == 0){
                especialistas(medicos[5],nMedicos);
            }


        } else if (res > 0 && FD_ISSET(fd_server_fifo, &fds)) {
            /* ======================= RECEBE STRUCT BALCAO ======================= */
            n = read(fd_server_fifo, &b, sizeof(balcao));
            if (n == -1) {
                printf("\nNão conseguiu ler...");
                exit(1);
            }
            printf("Recebi %d bytes ....\n", n);

            if (n == sizeof(balcao)) {
                if (b.cliente == 1) {
                    if (nClientes <= MaxClientes -1 ) {
                        b.cheio =1;

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
                        printf("b.cheio -> %d\n",b.cheio);
                        n = write(fd_cliente_fifo, &b, sizeof(balcao));
                        if (n == -1) {
                            printf("\nNão conseguiu escrever...");
                            exit(1);
                        }

                        close(fd_cliente_fifo);
                        clientes[nClientes] = b;
                        nClientes++;
                        printf("STRUCT B PID: %d\n",b.id_utente);
                        printf("DEBUG -> PID: %d\n",clientes[0].id_utente);
                        printf("\nN. clientes: %d\n", nClientes);
                        printf("Enviei %d  bytes ao utente...\n", n);

                    }else {
                        printf("cli-> %d",nClientes);
                        b.cheio = 0;
                        strcpy(b.msg,"O balcão não consegue atender mais clientes");
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
                    if (nMedicos <= MaxMedicos -1 ) {
                        b.registo = 1;
                        b.cheio=1;

                        if(b.registo == 1){
                            if(strcmp(b.msg,"sair") == 0){
                                printf("O especialista [%d] terminou a ligação\n",b.id_medico);
                                continue;
                            }
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
                        medicos[nMedicos] = b;
                        nMedicos++;
                        printf("\nN. medicos: %d\n", nMedicos);
                        printf("Novo especialista [%d] para a especialidade [%s]\n", b.id_medico, b.especialidade);

                        if(strcmp(b.especialidade, "geral") == 0 && b.filas[0] < 5) {
                           printf("Entrei no geral\n");
                            /* ======================= PERCORRER ARRAY DOS CLIENTES ======================= */
                            for (int i = 0; i < nClientes; i++) {
                                if (strcmp(clientes[i].classificao,b.especialidade) == 0) {
                                    printf("Há cliente para essa especialidade\n");
                                    clientes[i].registo = 2;
                                    strcpy(clientes[i].msg, "Esta ligado ao balcao!");

                                    /* ======================= ABRIR FIFO DO MEDICO ======================= */
                                    sprintf(m_fifo_fname, MEDICO_FIFO, b.id_medico);
                                    fd_cliente_fifo = open(m_fifo_fname, O_WRONLY);
                                    printf("Abri o Fifo do medico \n");

                                    n = write(fd_cliente_fifo, &clientes[i], sizeof(balcao));

                                    if (n == -1) {
                                        printf("\nNão conseguiu escrever...");
                                        exit(1);
                                    }
                                    close(fd_cliente_fifo);
                                    printf("Enviei %d  bytes...\n", n);
                                }
                            }
                            b.filas[0]++;
                        }

                       else if(strcmp(b.especialidade,  "ortopedia") == 0 && b.filas[1] < 5) {
                            printf("Entrei na ortopedia\n");
                            /* ======================= PERCORRER ARRAY DOS CLIENTES ======================= */
                            for (int i = 0; i < nClientes; i++) {
                                if (strcmp(clientes[i].classificao,b.especialidade) == 0) {
                                    printf("Há cliente para essa especialidade\n");
                                    clientes[i].registo = 2;
                                    strcpy(clientes[i].msg, "Esta ligado ao balcao!");

                                    /* ======================= ABRIR FIFO DO MEDICO ======================= */
                                    sprintf(m_fifo_fname, MEDICO_FIFO, b.id_medico);
                                    fd_cliente_fifo = open(m_fifo_fname, O_WRONLY);
                                    printf("Abri o Fifo do medico \n");

                                    n = write(fd_cliente_fifo, &clientes[i], sizeof(balcao));

                                    if (n == -1) {
                                        printf("\nNão conseguiu escrever...");
                                        exit(1);
                                    }
                                    close(fd_cliente_fifo);
                                    printf("Enviei %d  bytes...\n", n);
                                }
                            }
                            b.filas[1]++;
                        }
                       else if(strcmp(b.especialidade, "estamatologia") == 0 && b.filas[2] < 5) {
                            printf("Entrei na estamatologia\n");
                            /* ======================= PERCORRER ARRAY DOS CLIENTES ======================= */
                            for (int i = 0; i < nClientes; i++) {
                                if (strcmp(clientes[i].classificao,b.especialidade) == 0) {
                                    printf("Há cliente para essa especialidade\n");
                                    clientes[i].registo = 2;
                                    strcpy(clientes[i].msg, "Esta ligado ao balcao!");

                                    /* ======================= ABRIR FIFO DO MEDICO ======================= */
                                    sprintf(m_fifo_fname, MEDICO_FIFO, b.id_medico);
                                    fd_cliente_fifo = open(m_fifo_fname, O_WRONLY);
                                    printf("Abri o Fifo do medico \n");

                                    n = write(fd_cliente_fifo, &clientes[i], sizeof(balcao));

                                    if (n == -1) {
                                        printf("\nNão conseguiu escrever...");
                                        exit(1);
                                    }
                                    close(fd_cliente_fifo);
                                    printf("Enviei %d  bytes...\n", n);
                                }
                            }
                            b.filas[2]++;
                        }
                       else if(strcmp(b.especialidade,"neurologia") == 0 && b.filas[3] < 5) {
                            printf("Entrei na neurologia\n");
                            /* ======================= PERCORRER ARRAY DOS CLIENTES ======================= */
                            for (int i = 0; i < nClientes; i++) {
                                if (strcmp(clientes[i].classificao,b.especialidade) == 0) {
                                    printf("Há cliente para essa especialidade\n");
                                    clientes[i].registo = 2;
                                    strcpy(clientes[i].msg, "Esta ligado ao balcao!");

                                    /* ======================= ABRIR FIFO DO MEDICO ======================= */
                                    sprintf(m_fifo_fname, MEDICO_FIFO, b.id_medico);
                                    fd_cliente_fifo = open(m_fifo_fname, O_WRONLY);
                                    printf("Abri o Fifo do medico \n");

                                    n = write(fd_cliente_fifo, &clientes[i], sizeof(balcao));

                                    if (n == -1) {
                                        printf("\nNão conseguiu escrever...");
                                        exit(1);
                                    }
                                    close(fd_cliente_fifo);
                                    printf("Enviei %d  bytes...\n", n);
                                }
                            }
                            b.filas[3]++;
                        }
                       else if(strcmp(b.especialidade, "oftalmologia") == 0 && b.filas[4] < 5) {
                            printf("Entrei na oftalmologia\n");
                            /* ======================= PERCORRER ARRAY DOS CLIENTES ======================= */
                            for (int i = 0; i < nClientes; i++) {
                                if (strcmp(clientes[i].classificao,b.especialidade) == 0) {
                                    printf("Há cliente para essa especialidade\n");
                                    clientes[i].registo = 2;
                                    strcpy(clientes[i].msg, "Esta ligado ao balcao!");

                                    /* ======================= ABRIR FIFO DO MEDICO ======================= */
                                    sprintf(m_fifo_fname, MEDICO_FIFO, b.id_medico);
                                    fd_cliente_fifo = open(m_fifo_fname, O_WRONLY);
                                    printf("Abri o Fifo do medico \n");

                                    n = write(fd_cliente_fifo, &clientes[i], sizeof(balcao));

                                    if (n == -1) {
                                        printf("\nNão conseguiu escrever...");
                                        exit(1);
                                    }
                                    close(fd_cliente_fifo);
                                    printf("Enviei %d  bytes...\n", n);
                                }
                            }
                            b.filas[4]++;
                        }
                       else {
                           /* ======================= ENVIAR PARA MEDICO QUE NAO HÁ UTENTES PARA ESSA ESPECIALIDADE ======================= */
                           b.registo = -1;
                           strcpy(b.msg,"Não há utentes para essa especialdiade");
                           /* ======================= ABRIR FIFO DO MEDICO ======================= */
                           sprintf(m_fifo_fname, MEDICO_FIFO, b.id_medico);
                            fd_cliente_fifo = open(m_fifo_fname, O_WRONLY);
                           printf("Abri o Fifo do medico \n");
                           n = write(fd_cliente_fifo, &b, sizeof(balcao));

                            if(n == -1){
                               printf("Não consegui escrever par o FIFO do médico\n");
                               exit(1);
                           }
                           close(fd_cliente_fifo);
                           printf("Enviei %d  bytes...\n", n);
                       }
                    } else {
                        /* ======================= ENVIAR PARA MEDICO QUE NAO E POSSIVEL REGISTAR ======================= */
                        b.registo = 0;
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
    }while(strcmp(str_com, "sair\n") != 0);
    close(fd_server_fifo);
    unlink(bal_FIFO);
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

