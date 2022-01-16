//
// Created by user on 16/01/22.
//

#ifndef META_2_CLIENTE_H
#define META_2_CLIENTE_H
#include "utilis.h"

typedef struct Cliente{
    int cliente;
    char nome[100];
    char especialidade[100];
    char sintomas[100];
    int prioridade;
    int n_fila;
    int n_especilistas;

}cliente;

#endif //META_2_CLIENTE_H
