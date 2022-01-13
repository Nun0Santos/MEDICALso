//
// Created by user on 03/11/21.
//
#ifndef TP_UTILIS_H
#define TP_UTILIS_H
#include "global.h"


// ESTRUTAR QUE MEDICO E UTENTE ENVIAM PARA O BALCAO
typedef struct {
    char nome_utente[NOME_MAX], nome_medico[NOME_MAX];
    char sintoma[SINTOMA_MAX];
    char especialidade[ESPECIALIDADE_MAX];
    char classificao[ESPECIALIDADE_MAX];
    int filas[MAX_FILAS];
    pid_t id_utente, id_medico; //PID do processo
    int  prioridade;
    char msg[256];
    int  cliente; //Saber se e cliente ou medico | 1 -> Cliente | 0 -> Medico
    int cheio; // 1 - livre 0- cheio
    int sair;
    int temp;
    int flagB; // 1 balcao 0 nao e balcao
    int registo_medico;
    int registo_utente; // 1 ->
    int saiCli;
    int estado_utente; // 0 -> n esta em consulta 1-> consulta
    int estado_medico; //0 -> n esta em consulta 1-> consulta
    int flagA,flagC; //para a segunda resposta
    int consulta; // 0 -> não esta  1 -> está
    pthread_mutex_t *m;  // Partilhado


}balcao;

/*BALCAO */
typedef struct THbalcao{
    int continua,tempo;
    int maxClientes, maxMedicos;
    balcao p_cli[5];
    balcao p_med[5];
    int ite_cli , ite_med;//"iteradores"

}thbalcao;


#endif //TP_UTILIS_H
