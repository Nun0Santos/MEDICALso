//
// Created by user on 03/11/21.
//
#ifndef TP_MEDICO_H
#define TP_MEDICO_H

#define NOME_MAX 26
#define ESPECIALIDADE_MAX 30

typedef struct especialistas{
    char nome[NOME_MAX];
    char especialidade[ESPECIALIDADE_MAX];
    int medico_id; //PID do processo
}especialista;


#endif //TP_MEDICO_H
