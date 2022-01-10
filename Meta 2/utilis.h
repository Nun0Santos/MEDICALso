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
    int teste_id_utente;
    char estado;
    int cheio; // 1 - livre 0- cheio
    int  prioridade;
    char msg[256];
    char resp[256];
    pid_t id_utente, id_medico; //PID do processo
    int  cliente; //Saber se e cliente ou medico | 1 -> Cliente | 0 -> Medico
    int registo; //medico
    int registo_utente;
    int filas[MAX_FILAS];
    bool disponivel; //ver se especialista esta disponivel

    /*int MaxClientes;
    int MaxMedicos;

    int nClientes;
    int nMedicos;*/
}balcao;




#endif //TP_UTILIS_H
