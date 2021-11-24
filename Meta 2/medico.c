#include "medico.h"
#include "utilis.h"

int main(int argc, char *nome[NOME_MAX], char *especialidade[ESPECIALIDADE_MAX]){

    especialista  a;

    if(argc < 3){
        printf("Faltam argumentos!\n");
        return  -1;
    }
    printf("Nome: %s\n",a.nome[1]);
    printf("Especialidade: %s\n",a.especialidade[2]);

    return 0;
}

    //1º - Só aceita executar se o balcão estiver em funcionamento;

    // 1.2º - Interagir com o balcao para registar nome e especialidade



    //3º - Fico a aguardar que o balcao me indique que tenho um novo utente para atender


    //4º - Diálogo com o utente (especialista escreve pergunta e aguarda pela resposta)




