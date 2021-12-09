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
#include <fcntl.h>
/* Nome do FIFO do Servidor */
#define bal_FIFO "server_fifo"
/* Nome do FIFO para cada cliente, %d será substituído pelo PID com sprintf */
#define CLIENT_FIFO "client_fifo_%d"
/* Nome do FIFO para cada medico, %d será substituído pelo PID com sprintf */
#define MEDICO_FIFO "medico_fifo_%d"
#define TAM_MAX 50

typedef struct balcao balcao;
struct balcao{
    pid_t pid_cliente,pid_medico;
    int id;
    int n_clientes;
    int n_medicos;
};
/* Estrutura da mensagem correspondente a um pedido cliente -> Servidor */
typedef struct{
    pid_t pid_cliente,pid_medico;
    char frase[TAM_MAX];
}pergunta_c;

typedef struct {
    char frase[TAM_MAX];
}resposta_c;

#endif //TP_UTILIS_H
