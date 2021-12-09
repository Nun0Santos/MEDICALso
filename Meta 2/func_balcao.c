//
// Created by user on 14/11/21.
//
#include "func_balcao.h"
#include "medico.h"
#include "cliente.h"
#include "balcao.h"
#include "utilis.h"
#define CMDS_MAX 10


int  verificaComandos(char *comando) {
    char *cmds[CMDS_MAX] = {"utentes", "especialistas", "delutX", "delespX", "freqN", "encerra", "sintoma"};

    for (int i = 0; i < CMDS_MAX; i++) {
        if (strcmp(comando, cmds[i]) == 0) {
            //utentes();
        }
        if (strcmp(comando, "especialistas") == 0) {
            //especialistas();
        }
        if (strcmp(comando, cmds[i]) == 0) {
            delutX();
        }
        if (strcmp(comando, "delespX") == 0) {
            delespX();
        }
        if (strcmp(comando, cmds[i]) == 0) {
            freqN();
        }
        if (strcmp(comando, "encerra") == 0) {
            encerra();
        }
        if (strcmp(comando, "sintoma") == 0) {
            sintoma();
        } else {
            printf("Comando invalido\n");
            return 0;
        }
    }

}

void utentes (utente_ptr array_utente, int n_utentes){

}
void especialistas(especialista_ptr array_especialista, int n_especialistas){

}

utente_ptr delutX(){
printf("Chegeui ao delutX");

}
especialista_ptr delespX(){

}
void freqN(){
    printf("Chegeui ao freqN");

}
void encerra(){

}
char sintoma(){
    /*char comando[30],resposta[30];
    int tam = 0;
        do{

            printf("\n--> ");
            fgets(comando,sizeof(comando)-1,stdin);
            write(balcaoToClassificador[1], comando, strlen(comando);
            tam = read(ClassificadorToBalcao[0],resposta,sizeof(resposta)-1);
            resposta[tam] = '\0';
            printf("%s",resposta);

        }while(strcmp(comando,"#fim\n") != 0);*/

}
