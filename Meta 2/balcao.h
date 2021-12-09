//
// Created by user on 03/11/21.
//
#ifndef TP_BALCAO_H
#define TP_BALCAO_H

#define NOME_MAX 26
#define ESPECIALIDADE_MAX 30
#define SINTOMA_MAX 30
//por as structs cliente e medico
typedef struct Utentes utente, *utente_ptr;
struct Utentes{
    char nome[NOME_MAX];
    char sintoma[SINTOMA_MAX];
    int cliente_id; //PID do processo
};

typedef struct especialistas especialista, *especialista_ptr;
struct especialistas{
    char nome[NOME_MAX];
    char especialidade[ESPECIALIDADE_MAX];
    int medico_id; //PID do processo
};


//FUNÇÕES





#endif //TP_BALCAO_H
