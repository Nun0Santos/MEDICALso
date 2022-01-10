//
// Created by user on 14/11/21.
//
#include "func_balcao.h"
#define CMDS_MAX 10


int  verificaComandos(char *comando) {
    char *cmds[CMDS_MAX] = {"utentes", "especialistas", "delutX", "delespX", "freqN", "encerra"};
}

void utentes (balcao *array_utente, int n_utentes) {
    if (n_utentes != 0) {
        for (int i = 0; i < n_utentes; i++) {
            printf("Utente [%d] com a especialidade %s e prioridade %d\n", array_utente->id_utente,array_utente->classificao, array_utente->prioridade);
        }
    } else {
        printf("Ainda não há clientes!\n");
    }
}
void especialistas(balcao array_especialista, int n_especialistas){
    if(n_especialistas != 0){
        for(int i = 0; i < n_especialistas; i++){
            printf("Especialista [%d] com a especialidade %s \n",array_especialista.id_medico,array_especialista.especialidade);
        }
    }
    else{
        printf("Ainda não há médicos especialistas!\n");
    }
}

balcao delutX(){
    printf("cheguei ao delutX\n");

}
balcao delespX(){


}

void freqN(){
    printf("Chegeui ao freqN");
}
void encerra(){
    unlink("server_fifo");
    exit(EXIT_SUCCESS);
}

