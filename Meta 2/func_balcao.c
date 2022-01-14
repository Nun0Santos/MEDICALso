//
// Created by user on 14/11/21.
//
#include "func_balcao.h"
#define CMDS_MAX 10


int  verificaComandos(char *comando) {
    char *cmds[CMDS_MAX] = {"utentes", "especialistas", "delutX", "delespX", "freqN", "encerra"};
}

void utentes (balcao array_utente[], int n_utentes) {
    if (n_utentes != 0) {
        for (int i = 0; i < n_utentes; i++) {
            printf("Utente [%d] com a especialidade %s e prioridade %d\n", array_utente[i].id_utente,array_utente[i].classificao, array_utente[i].prioridade);
        }
    } else {
        printf("Ainda não há clientes!\n");
    }
}
void especialistas(balcao array_especialista[], int n_especialistas){
    if(n_especialistas != 0){
        for(int i = 0; i < n_especialistas; i++){
            printf("Especialista [%d] com a especialidade %s \n",array_especialista[i].id_medico,array_especialista[i].especialidade);
        }
    }
    else{
        printf("Ainda não há médicos especialistas!\n");
    }
}

balcao delutX(balcao array_utente[],int pid,int n_utentes){
    printf("ANTES:\n");
    for(int i = 0; i<n_utentes; i++){
        printf("[%d]PID: %d\n",i,array_utente[i].id_utente);
    }
    for(int i=0; i<n_utentes-1; i++){
            if(pid == array_utente[i].id_utente){
                for(int j = i; j< n_utentes-1; j++){
                    array_utente[j] = array_utente[j+1];
		    n_utentes --;
                }
            }
        }
        printf("DEPOIS DE ELIMINAR\n");
        for(int i = 0; i<n_utentes; i++){
            printf("[%d]PID: %d\n",i,array_utente[i].id_utente);
        }
}
balcao delespX(){


}

void freqN(){
    printf("Chegeui ao freqN");
}
void encerra(){


    unlink("sinal");
    unlink("server_fifo");
    exit(EXIT_SUCCESS);
}

