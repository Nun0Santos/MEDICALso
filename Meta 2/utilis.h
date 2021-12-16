//
// Created by user on 03/11/21.
//

#ifndef TP_UTILIS_H
#define TP_UTILIS_H
#include "global.h"

/* Nome do FIFO do Servidor */
#define bal_FIFO "server_fifo"

/* Nome do FIFO para cada cliente, %d será substituído pelo PID com sprintf */
#define CLIENT_FIFO "client_fifo_%d"

/* Nome do FIFO para cada medico, %d será substituído pelo PID com sprintf */
#define MEDICO_FIFO "medico_fifo_%d"
#define TAM_MAX 50


#define NOME_MAX 26
#define ESPECIALIDADE_MAX 30
#define SINTOMA_MAX 30

typedef struct Utentes utente, *utente_ptr;
struct Utentes{
    char nome[NOME_MAX];
    char sintoma[SINTOMA_MAX];
    pid_t cliente_id; //PID do processo
};

typedef struct especialistas especialista, *especialista_ptr;
struct especialistas{
    char nome[NOME_MAX];
    char especialidade[ESPECIALIDADE_MAX];
    pid_t medico_id; //PID do processo
    bool disponivel; //Perguntar se pode utilizar
};
typedef struct dados_pipes {
    char qual[10];
    pthread_t tid;   /* ID da thread */
    int fd;
}ThrDados;


/*typedef struct balcao balcao;
struct balcao{
    pid_t pid_cliente,pid_medico;
    int id;
    int n_clientes;
    int n_medicos;
};*/


#endif //TP_UTILIS_H
