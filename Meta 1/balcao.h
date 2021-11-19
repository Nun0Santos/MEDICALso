//
// Created by user on 03/11/21.
//
#ifndef TP_BALCAO_H
#define TP_BALCAO_H
#include "medico.h"
#include "cliente.h"
#include "utilis.h"



//FUNÇÕES

     utente utentes (); //Lista os utentes em fila de ESPERA indicando qual a especialidade e qual a prioridade, e também os utentes atualmente a serem atendidos, em que especialidade e por qual especialista;

     especialista especialistas(); //Indica a lista de especialistas conhecidos e estado atual (à espera, a atender o utente X)

     utente delutX(); //Remove um utente em espera, informando-o através do seu programa cliente (que depois encerra). Esse utente sai do sistema. Válido apenas para utentes que ainda não começaram a ser atendidos;

     especialista delespX(); //Remove o especialista X, informando-o através do seu programa médico (que depois encerra). Válido apenas para especialistas que não estejam a atender nenhum utente no momneto.

    void freqN(); //Passa a apresentar a ocupação das filas de espera de N em N segundos;

    void encerra(); // Encerra o sistema, avisando os clientes,  os médicos e o classificador -> Usar Sinais free



#endif //TP_BALCAO_H
