#include "cliente.h"
#include "utilis.h"



int main(int argc, char *nome[NOME_MAX]){
        utente a;

        /*if(access(balcao_FIFO, F_OK) != 0){
        printf("O balcao nao esta a executar\n");
        exit(2);
    }*/
        if( argc < 2){
            printf("Faltam argumentos!\n");
            return -1;

        }

        printf("Indique os seus sintomas: ");
        scanf("%s",a.sintoma);
        printf("Utente [PID] %d\n",getpid());
         printf("%s",a.nome);
    return 0;
}





    //1º - Só aceita executar se o balcão estiver em funcionamento : (Enviar um sinal para o balca para ver se está a correr ?)


    //1.2º - Enviar sintomas ao balcao


    //2º - Obter do balcao: area de especialidade, prioridade, numero de utentes nesse instante à frente dele nessa especialidade, numero de especialista dessa area


    //3º - Aguardar que seja encaminhado para um especialista (programa medico)

    //4º - Diálogo com o especialista (programa médico)





