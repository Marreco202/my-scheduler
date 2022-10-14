#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>

/*
Beatriz Sordero - 
João Victor Godinho Woitschach - 2011401
*/

/*
de tempos em tempos, o interpretador manda para a fila de execucao UM UNICO PROCESSO!!! para o escalonador!

*/

struct el {
    int name, prio, tempo_init, tempo_total, minha_vez, tempo_atual;
};

struct fila{
    struct el *curr;
    struct fila *prox;
};

typedef struct el El;
typedef struct fila Fila;


Fila* create_line(El* insert){

    Fila *new = ((Fila*)malloc(sizeof(Fila)));

    new->curr = insert;
    new->prox = NULL;

    return new;
}


void print_el(El* f){
    printf("******************************\n");
    printf("processo: p%d\n",f->name);
    printf("prioridade: %d\n",f->prio);
    printf("tempo inicial: %d\n",f->tempo_init);
    printf("tempo atual: %d\n",f->tempo_atual);
    printf("tempo total: %d\n",f->tempo_total);
    printf("minha vez: %d\n",f->minha_vez);
    
}

void print_line(Fila* f){//coe maluco

    if(f == NULL){
        printf("fila vazia!\n");
    }
    while(f!= NULL){
        print_el(f->curr);
        f = f->prox;
    }
        printf("******************************\n");
}

void insert_on_line(Fila* f,El* insert){
    
    Fila *ant = NULL;
    int i = 0;

    while(f != NULL){
        //printf("passei pro proximo: %d\n",i);
        ant = f;
        f = f->prox;
        i++;
    }
    //printf("encontrei um espaco pra eu entrar! %d\n",i);
    f = create_line(insert);
    ant->prox = f;
}


El* create_el(int name, int prio, int tempo_init, int tempo_total){

    El* new = (El*)malloc(sizeof(El));
    
    new->name = name;
    new->prio = prio;
    new->tempo_init = tempo_init;
    new->tempo_total = tempo_total;
    new->minha_vez = 0;
    new->tempo_atual = 0;

    return new;
}

int get_tempoExecTotal(Fila* f){

    int tempo_total = 0; //tempo total de execucao da fila

    while(f!= NULL){
        tempo_total += f->curr->tempo_total;
        f = f->prox;
    }
    return tempo_total;
}

void free_line(Fila* f){
    
    Fila *ref; //referencia para dar free

    while(f!=NULL){
        ref = f;
        f = f->prox;
        free(ref->curr);
        free(ref);
    }
}

int main(void){

    FILE* f = fopen("input.txt","r");
    if(f == NULL){
        printf("erro, arquivo nao pode ser aberto\n");
        exit(1);
    }
    printf("Abri o arquivo\n");

    Fila* fila; //fila vazia
    El* qg; //elemento quebra-galho para fazer as inserções na lista 

    
    int lever = 0; //booleana que verifica se o primeiro elemento ja foi inserido

    while(!feof(f)){
        
        int name, prio, tempo_init, tempo_total;
        fscanf(f,"exec p%d, prioridade=%d, inicio_tempo_execucao=%d, tempo_total_ execucao =%d\n",&name,&prio,&tempo_init,&tempo_total);
        qg = create_el(name,prio,tempo_init,tempo_total);

        if(lever == 0){
            fila = create_line(qg);
            lever++;
        }else{
            insert_on_line(fila,qg);
        }

        
    }

    int tempoTotalExec = get_tempoExecTotal(fila); //tempo total de execucao de todos os programas, quando esse tempo acabar, finalizar escalonador e o programa 
    int *runTime = (int*) malloc(sizeof(int)); //ha quanto tempo o programa esta rodando
    *runTime = 0;

    printf("tempo total de execucao: %d\n!!!!!!!!!!!\n",tempoTotalExec);


    /* TIRAR ESSE COMENTARIO

    int escalonador = fork();

    if(escalonador == 0){ //interpretador
        while(runTime != tempoTotalExec){ //CADA ITERACAO REPRESENTA 1 SEGUNDO!!!

            sleep(1);

            *runTime++;
        }
        //faz chamadas ao escalonador
    }else if(escalonador > 0){ //escalonador
        //waitpid para esperar o pai mandar o SIGCONT
        sleep(1);
    }

    //faz fork, e faz chamadas ao escalonador para ele inserir na paradinha do lance

    TIRAR ESSE COMENTARIO */

    print_line(fila);

    //printf("Hello world!\n");
    
    fclose(f);
    free(runTime);
    free_line(fila); //libera os mallocs todos

    return 0;
}