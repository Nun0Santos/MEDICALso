//
// Created by user on 14/11/21.
//
#include "func_balcao.h"
#include "medico.h"
#include "cliente.h"
#include "balcao.h"
#include "utilis.h"
#define CMDS_MAX 10


int  verificaComandos(char *comando){
	    //char  cmds[CMDS_MAX] = {"utentes","especialistas","delutX","delespX","freqN","encerra","sintoma"};
	    printf("Cheguei aqui");
        for(int i = 0; i < CMDS_MAX; i++){
            if(strcmp(comando,"utentes") == 0){
                    utentes();
            }
            if(strcmp(comando,"especialistas") == 0){
                especialistas();
            }
            if(strcmp(comando,"delutX") == 0){
                delutX();
            }
            if(strcmp(comando,"delespX") == 0){
                delespX();
            }
            if(strcmp(comando,"freqN") == 0){
                freqN();
            }
            if(strcmp(comando,"encerra") == 0){
                encerra();
            }
            if(strcmp(comando,"sintoma") == 0){
                sintoma();
            }
            else{
                printf("Comando invalido");
                return 0;
            }
        }
}
utente utentes ();
especialista especialistas();
utente delutX();
especialista delespX();
void freqN();
void encerra();
char sintoma();
