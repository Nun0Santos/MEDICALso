//
// Created by user on 03/11/21.
//

#ifndef TP_UTILIS_H
#define TP_UTILIS_H
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>

#define NOME_MAX 26
#define SINTOMA_MAX 30

struct balcao{
    int id;
    int n_clientes;
    int n_medicos;
};

typedef struct utentes {
    char nome[NOME_MAX];
    char sintoma[SINTOMA_MAX];
    int cliente_id; //PID do processo
}utente;

#endif //TP_UTILIS_H
