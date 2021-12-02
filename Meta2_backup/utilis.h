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

#define bal_FIFO "server_fifo"
#define medico_FIFO "medico_fifo"
//#define  MAIL_BALCAO test

//partilhar mail do balcao

struct balcao{
    int id;
    int n_clientes;
    int n_medicos;
};

#endif //TP_UTILIS_H
