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

    utente* ptrUtentes;
    especialista* ptrEspecialistas;

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

            execlp("./classificador","./classificador", NULL);
            fprintf(stderr, "Comando nao encontrado\n");
            exit(123);
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

// 1?? - Verificar se j?? existe outro balc??o a correr : (comando ps ax diz-me todos os processos que est??o a decorrer)

//2?? - Interagir em SIMULT??NEO com clientes, m??dicos e classificador :  (fun????o execl permite-me executar um progrma)

//3?? - Recebe do m??dico um sinal de vida a cada 20 segundos (sleep(20)), : Usar sinais (kill -> Envia um sinal para um processo |  signal -> recebe o sinal )

//4?? - Executa internamente o classificador e interage com ele para obter as classifica????es : (o comando bg permite-me correr um programa em background e continuar a usar a linha de comandos)

//5??-  Recebe informa????es dos clientes: utente chegou com estes sintomas; utente desistiu;

//6?? - Recebe informa????es dos m??dicos: novo especialista para especialidade X, sa??da de especialista, fim de consulta;

//7?? - Informa o cliente que o utente vai iniciar consulta no especialista X, dando as informa????es necess??rias

//8?? - Informa o m??dico que o especialista vai iniciar consulta ao utente X, dando as informa????es necess??rias

//9?? - Deteta que os m??dicos deixar de enviar o sinal de vida e age em conformidade : (Agir em conformidade ?? encerrar o programa m??dicos e todos os outros programas?)




