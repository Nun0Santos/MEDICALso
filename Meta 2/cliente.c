
#include "cliente.h"
#include "utilis.h"
#include "balcao.h"



int main(int argc, char *argv[]){
        utente a;

        //int fd_client_fifo; // file descriptors para os FIFOS
        //char cli_fifo[32]; 				   //FIFO do Cliente
        int c_fifo_fd;

        if(access(bal_FIFO, F_OK) != 0){
        printf("O balcao nao esta a executar\n");
        exit(2);
    }
        if( argc < 2){
            printf("Faltam argumentos!\n");
            return -1;

        }

    /*======================FIFO do Cliente (aberto para leitura e escrita)===========================*/




        printf("Indique os seus sintomas: ");
        scanf("%s\n",a.sintoma);
        printf("Utente [PID] %d\n",getpid());

    return 0;
}





    //1º - Só aceita executar se o balcão estiver em funcionamento : (Enviar um sinal para o balca para ver se está a correr ?)


    //1.2º - Enviar sintomas ao balcao


    //2º - Obter do balcao: area de especialidade, prioridade, numero de utentes nesse instante à frente dele nessa especialidade, numero de especialista dessa area


    //3º - Aguardar que seja encaminhado para um especialista (programa medico)

    //4º - Diálogo com o especialista (programa médico)





