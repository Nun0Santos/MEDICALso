//
// Created by user on 14/11/21.
//
int fd_server_fifo,fd_client_fifo;
/* ------Ver se está a e sexecutar-----
 *  if(access(balcao_FIFO, balcao) == 0){
        printf("Balcão já a executar\n");
        exit(2);
    }

    mkfifo(balcao_FIFO, 0600);
    printf("Criei o FIFO bo balcão...\n");

    // abri o fifo
    fd_server_fifo = open(balcao_FIFO,O_RDWR);


    if((fd_server_fifo) == -1){
        printf("Erro na Abertura\n");
        exit(2);
    }

 */