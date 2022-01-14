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
    int filas[MAX_FILAS],prioridade,sair,temp;
    pid_t id_utente, id_medico; //PID do processo
    char msg[256];
    int  cliente; //Saber se e cliente ou medico | 1 -> Cliente | 0 -> Medico
    int cheio; // 1 - livre 0- cheio
    int registo_medico,registo_utente;
    int consulta; // 0 -> não esta  1 -> está
    int flagB; // 1 balcao 0 nao e balcao
    int med_ocupado,cli_ocupado; // 0-> esta | 1-> n esta
    int flagG,flagO,flagN,flagE,flagOF; //flagG -> geral | flagO->Ortopedia | flagN->Neurologia | flagE->Estomatologia | flagOF->oftalmologia
    pthread_mutex_t *m;  // Partilhado

}balcao;
//prioridade 1 tem mais especialidade

/*BALCAO */
typedef struct THbalcao{
    int continua,tempo;
    int maxClientes, maxMedicos;
    balcao p_cli[5];
    balcao p_med[5];
    int ite_cli , ite_med;//"iteradores"
    pthread_mutex_t *m;  // Partilhado

}thbalcao;


#endif //TP_UTILIS_H
