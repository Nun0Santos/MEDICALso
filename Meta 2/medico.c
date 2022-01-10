#include "utilis.h"
#include "func_balcao.h"

int main(int argc, char *argv[]) {
    char m_fifo_fname[25],c_fifo_fname[25], comando[30],resposta[30],str_com[30];
    int fd_server_fifo, fd_cliente_fifo,fd_medico_fifo,res,n,tam;
    fd_set fds;
    struct timeval tempo;


    int estado = 0; // 0 -> Nao registado | 1 -> Registado | 2 -> estou em consulta

    balcao b;
    b.cliente = 0;
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

    do{
        printf("-->\n");
        /* ======================= SELECT ======================= */
        /* ======================= PREPARAR DESCRITORES QUE QUERO MONOTORIZAR ============== */
        FD_ZERO(&fds); //LIMPAR DESCRITORES
        FD_SET(0,&fds); // TECLADO
        FD_SET(fd_cliente_fifo,&fds); // fifo do medico(leitura)
        tempo.tv_sec = 8; // TIMEOUT
        tempo.tv_usec = 0;

        res = select(fd_cliente_fifo+1,&fds,NULL,NULL,&tempo);
        if(res == 0){
           //printf("\nEstou sem fazer nada...\n");
        }
        else if(res > 0 && FD_ISSET(0,&fds)){
            if(estado == 0){ //Nao registado //avisar o utilizador que nao pode fazer nada enquanto  nao chegar confirmacao de registo
                printf("Não estou registado\n");
                break;
            }
            else if(estado == 1){ //Registado //enviar comando para o balcao(se nao estiver numa consulta)
                scanf("%s",comando);
                strcpy(b.msg,comando);

                n = write(fd_server_fifo, &b, sizeof(balcao));
                if (n == -1) {
                    printf("\nNão conseguiu escrever no FIFO do servidor...\n");
                    exit(1);
                }
                if(strcmp(comando,"sair") == 0){
                    break;
                }

            }
            else if(estado == 2){ // Em consulta //enviar comando para cliente (se estiver numa consulta) abrir fifo cliente
                fgets(b.msg,sizeof(b.msg),stdin);


                /* ======================= ABRIR FIFO DO CLIENTE ======================= */
                sprintf(c_fifo_fname, CLIENT_FIFO, b.id_utente);
                fd_cliente_fifo = open(c_fifo_fname, O_RDWR);
                printf("Abri o Fifo do cliente\n");

                /* ======================= ENVIAR AO CLIENTE ======================= */
                b.id_medico=getpid();
                n = write(fd_cliente_fifo, &b, sizeof(balcao)); //fd not name
                if (n == -1) {
                    perror("Erro na escrita do FIFO: ");
                    printf("\nNão conseguiu escrever no FIFO do cliente...\n");
                    exit(1);
                }
                close(fd_cliente_fifo);
                printf("Enviei %d  bytes...\n", n);
                /* ======================= LER DO CLIENTE (MEU FIFO)======================= */
                fd_cliente_fifo = open(m_fifo_fname, O_RDWR);
                tam = read(fd_cliente_fifo, &b, sizeof(balcao));
                if (tam == -1) {
                    perror("Erro: ");
                    printf("\nNão conseguiu ler do meu FIFO...");
                    exit(1);
                }
                printf("Resposta do utente [%d] : %s\n",b.id_utente,b.msg);
            }
        }
        else if(res > 0 && FD_ISSET(fd_cliente_fifo,&fds)){
            n = read(fd_cliente_fifo, &b, sizeof(balcao));
            if (n == -1) {
                printf("\nNão conseguiu ler...");
                exit(1);
            }
            printf("Recebi %d bytes de alguem ...\n",n); //balcao ou cliente

           if(strcmp(b.msg,"Esta ligado ao balcao!") == 0 && b.registo == 0){ //filas cheias
                estado = 0;
                printf("Nao foi possivel o registo devido a filas cheias ou especialidade invalida - tente mais tarde\n");
            }
           else if(strcmp(b.msg,"Esta ligado ao balcao!") == 0 && b.registo == 1) {
                estado = 1;
                printf("Especialista [%d] foi registado\n",b.id_medico);
            }
            else if(strcmp(b.msg,"Esta ligado ao balcao!") == 0 && b.registo == 2){
                estado = 2;
                printf("ID UTENTE: %d\n",b.id_utente);
                printf("NOME UTENTE %s\n",b.nome_utente);

                printf("Consulta vai iniciar\n");
                printf("Dados do Utente : ");
                printf("PID = %d \r Nome= %s \r Sintoma = %s\n",b.id_utente,b.nome_utente,b.sintoma);
                printf("----------------------------------------------------\n");
                printf("Pode escrever alguma coisa ao utente\n");
                printf("ESTADO = %d\n",estado);
            }
            else if(b.cheio == 0){
               printf("\nO balcão não consegue atender mais medicos - volte mais tarde!\n");
               exit(1);
            }
            else if(b.registo == -1){
                printf("%s\n",b.msg);
            }
        }
    }while(strcmp(comando, "sair") != 0);
    close(fd_cliente_fifo);
    unlink(m_fifo_fname);
}

    //1º - Só aceita executar se o balcão estiver em funcionamento;

    // 1.2º - Interagir com o balcao para registar nome e especialidade



    //3º - Fico a aguardar que o balcao me indique que tenho um novo utente para atender


    //4º - Diálogo com o utente (especialista escreve pergunta e aguarda pela resposta)



