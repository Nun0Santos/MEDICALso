//
// Created by user on 03/11/21.
//
#ifndef TP_CLIENTE_H
#define TP_CLIENTE_H
#define NOME_MAX 26
#define SINTOMA_MAX 30

typedef struct utentes {
    char nome[NOME_MAX];
    char sintoma[SINTOMA_MAX];
    int cliente_id; //PID do processo
    int medico_id;
}utente;















#endif //TP_CLIENTE_H
