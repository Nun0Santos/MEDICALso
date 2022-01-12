//
// Created by user on 16/12/21.
//

#ifndef META_2_GLOBAL_H
#define META_2_GLOBAL_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdbool.h>
#include <sys/select.h>
#include <signal.h>


#define NOME_MAX 26
#define ESPECIALIDADE_MAX 30
#define SINTOMA_MAX 256
#define MAX_FILAS 5

/* Nome do FIFO do Servidor */
#define bal_FIFO "server_fifo"

/* Nome do FIFO para cada cliente, %d será substituído pelo PID com sprintf */
#define CLIENT_FIFO "client_fifo_%d"

/* Nome do FIFO para cada medico, %d será substituído pelo PID com sprintf */
#define MEDICO_FIFO "medico_fifo_%d"

#define FIFO_SINAL "sinal"

#endif //META_2_GLOBAL_H
