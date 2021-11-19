//
// Created by user on 07/11/21.
//
#include "balcao.h"
#include "utilis.h"




int main() {
    char linha[100];
    int res,tam,exstat,balcaoToClassificador[2],ClassificadorToBalcao[2];
    int MaxClientes, MaxMedicos;

    char *MaxClientes_str, *MaxMedicos_str;
    char resposta[30];

    pipe(balcaoToClassificador);
    pipe(ClassificadorToBalcao);


    printf("PID = %d\n",getpid());
    MaxClientes_str = getenv("MAXCLIENTES");
    if (MaxClientes_str){
       MaxClientes = atoi(MaxClientes_str);
    }
    else{
        printf("Erro ao ler MAXCLIENTES\n");
        return -1;
    }
    printf("MAXCLIENTE = %d\n",MaxClientes);

    MaxMedicos_str = getenv("MAXMEDICOS");
    if (MaxMedicos_str){
        MaxMedicos = atoi(MaxMedicos_str);
    }
    else{
        printf("Erro ao ler MaxMedicos\n");
        return -1;
    }

    printf("MAXMEDICOS = %d\n",MaxMedicos);

   utente* ptrUtentes;
   especialista* ptrEspecialistas;

   ptrUtentes = malloc(sizeof(utente) * MaxClientes);

   if (ptrUtentes == NULL) {
       printf("Erro na alocacao dos utentes!\n");
       return -1;
   }

   ptrEspecialistas = malloc(sizeof(especialista) * MaxMedicos);

   if (ptrEspecialistas == NULL) {
       printf("Erro na alocacao dos medicos especialistas!\n");
       return -1;
   }
   /*for (int i = 0; i < MaxClientes; i++) {
       ptrUtentes[i].cliente_id = 1;
       strcpy(ptrUtentes[i].nome, "Manel");
       strcpy(ptrUtentes[i].sintoma, "Dor no peito");
       printf("id: %d \t sintoma: %s \t nome: %s\n", ptrUtentes[i].cliente_id, ptrUtentes[i].sintoma, ptrUtentes[i].nome);
   }*/
    /*for (int i = 0; i < MaxMedicos; ++i) {
        ptrEspecialistas[i].medico_id = 1;
        strcpy(ptrEspecialistas[i].especialidade, "oftalmologia");
        strcpy(ptrEspecialistas[i].nome,  "Alexandre");
        printf("id: %d \t especialidade: %s \t  nome: %s\n", ptrEspecialistas[i].medico_id, ptrEspecialistas[i].especialidade, ptrEspecialistas[i].nome);
    }*/
        res = fork();
        if (res == 0) {
            //printf("Sou o filho %d\n", getpid());
            close(0);
            dup(balcaoToClassificador[0]);
            close(balcaoToClassificador[0]);
            close(balcaoToClassificador[1]);

            close(1);
            dup(ClassificadorToBalcao[1]);
            close(ClassificadorToBalcao[0]);
            close(ClassificadorToBalcao[1]);

            execlp("./classificador","./classificador", NULL); //VER ESPACOS
            fprintf(stderr, "Comando nao encontrado\n");
            exit(123);
            return 3;
        }
        //pai aguarda pelo comando e depois continua
        close(balcaoToClassificador[0]);
        close(ClassificadorToBalcao[1]);

        do{
            printf("\n--> ");
            fgets(linha,sizeof(linha)-1,stdin);

            write(balcaoToClassificador[1],linha,strlen(linha));
            tam = read(ClassificadorToBalcao[0],resposta,sizeof(resposta)-1);
            resposta[tam] = '\0';
            printf("%s",resposta);

        }while(strcmp(linha,"#fim\n") != 0);
        wait(&exstat);
        exit(0);
}	

// 1º - Verificar se já existe outro balcão a correr : (comando ps ax diz-me todos os processos que estão a decorrer)


//2º - Interagir em SIMULTÂNEO com clientes, médicos e classificador :  (função execl permite-me executar um progrma)

//3º - Recebe do médico um sinal de vida a cada 20 segundos (sleep(20)), : Usar sinais (kill -> Envia um sinal para um processo |  signal -> recebe o sinal )

//4º - Executa internamente o classificador e interage com ele para obter as classificações : (o comando bg permite-me correr um programa em background e continuar a usar a linha de comandos)

//5º-  Recebe informações dos clientes: utente chegou com estes sintomas; utente desistiu;

//6º - Recebe informações dos médicos: novo especialista para especialidade X, saída de especialista, fim de consulta;

//7º - Informa o cliente que o utente vai iniciar consulta no especialista X, dando as informações necessárias

//8º - Informa o médico que o especialista vai iniciar consulta ao utente X, dando as informações necessárias

//9º - Deteta que os médicos deixar de enviar o sinal de vida e age em conformidade : (Agir em conformidade é encerrar o programa médicos e todos os outros programas?)




