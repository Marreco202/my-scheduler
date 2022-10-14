#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>

/*
João Victor Godinho Woitschach - 2011401
Maria Beatriz Sobreira - 2010953
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
    new->minha_vez = 0; //quando esse elemento e zero, significa que ele pode dar 
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

El* next_off(Fila* f, int tempo_atual){//TESTAR PARA VER SE FUNCIONA || seleciona o proximo a sair da LL
    
    while(f!=NULL){
        if(f->curr->tempo_init == tempo_atual)
            return f->curr;
        f = f->prox;
    }

    return NULL; //nao foi encontrado elemento a ser inserido
}

Fila* remove_of_line(Fila*f , El* target){// TROCAR PARA O CODIGO MAIn||TESTADO E FUNCIONAL! || remove da fila o elemento target, e retorna uma nova fila sem aquele elemento

    Fila* ant = NULL; //referencia ao ponteiro para o anterior
    Fila* buf = f;

    while(buf!=NULL){
        if(buf->curr->name == target->name){
            if(ant == NULL){ //o elemento a ser retirado é o primeiro da LL
                f = f->prox;
                //printf("hello there! %p\n",buf->curr);
                free(buf->curr);
                free(buf);
                return f;
            }else{
                ant->prox = buf->prox; // skip 1
                free(buf->curr);
                free(buf);
                return f;
            }
        }
        ant = buf;
        buf = buf->prox;
    }
    return f; //nao foi encontrado o elemento target
}

int get_line_size(Fila *f){
    int tam = 0;
    while(f!=NULL){
        tam++;
        f = f->prox;
    }

    return tam;
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

void hands_down(Fila *f){// COPIAR E COLAR NO CODIGO ANTIGO! ||TESTADO E FUNCIONANDO!!! || abaixa as maos de todos os elementos da linked list
    while(f!=NULL){
        f->curr->minha_vez = 0;
        f = f->prox;
    }
}

void copy_el(El* from, El* to){

    to->name = from->name;
    to->prio = from->prio;
    to->tempo_atual = from->tempo_atual;
    to->tempo_init = from->tempo_init;
    to->tempo_total = from->tempo_total;
    to->minha_vez = from->minha_vez;

}

El* find_prio(Fila *RR){// TROCAR PARA O ARQUIVO NORMAL||TESTADO E FUNCIONANDO || procura pelo proximo elemento da round robin de maior prioridade 

    int lowest_prio; //maior prioridade encontrada ate entao

    Fila* candidatos ; //linked list de todos os candidatos de maiores prioridades 
    int lever = 0;

    Fila *loop1, *loop2, *loop3;
    loop1 = RR, loop2 = RR;

    El* escolhido = (El*) malloc(sizeof(El));

    while(loop1 != NULL){ //procura pela maior prioridade

        if(lever == 0){
            lowest_prio = loop1->curr->prio;
            lever++;

        } else if (lever > 0 && loop1->curr->prio < lowest_prio){
            lowest_prio = loop1->curr->prio; 

        }
        loop1 = loop1->prox;
    }

    lever = 0;
    while(loop2 != NULL){//a partir da maior prioridade, cria uma LL de candidatos
        
        if(loop2->curr->prio == lowest_prio){
            
            El* qg = (El*) malloc(sizeof(El));
            copy_el(loop2->curr,qg);

            if(lever == 0){
                candidatos = create_line(qg);
                lever++;
            }
            else insert_on_line(candidatos,qg);
        }
        loop2 = loop2->prox;
    }
    loop3 = candidatos;

    printf("FILA DE CANDIDATOS: \n");
    print_line(loop3);

    while(loop3 != NULL){ //verifica se ha alguem com a mao abaixada (ou seja, se alguem de maior prioridade nao tiver sido selecionado nesta rodada)
        if(loop3->curr->minha_vez == 0){

            copy_el(loop3->curr,escolhido); //PODE DAR ERRO || copia o conteudo do no da LL candidatos
            candidatos = remove_of_line(candidatos,loop3->curr);
            free_line(candidatos); //libera a LL candidatos
            return escolhido;
        }
        loop3 = loop3->prox;
    }

    if(loop3 == NULL){
        printf("ERRO NA FIND PRIO\n");
        exit(1);
    }

    hands_down(candidatos); //se todos estiverem com as maos levantadas, abaixa todas elas...

    copy_el(candidatos->curr,escolhido); //... e seleciona o primeiro da LL arbitrariamente || ISSO PODE GERAR RESULTADOS INESPERADOS, PQ EU NAO ESTOU DEFININDO EXATAMENTE QUEM DEVE ENTRAR QUE HORA NO PROGRAMA!!!
    free_line(candidatos);

    printf("NOVA FILA RR: \n");
    print_line(RR);

    return escolhido;
}


void go_robin(Fila *RR){/* TESTAR || funcao do escalonador efetivamente */

    int pid = fork();
    El* next;
    
    next = find_prio(RR);
    


    if(pid > 0){ 
        
        //da o exec baseado no nome do processo
        //  este processo pode ja estar sendo executado, entao eu tenho que verificar se ele ja levou um exec;
        //      se ele ja tiver levado, SIGCONT
        //      se nao, exec
        
        sleep(1); //aguarda 1 segundo 
        //SIGSTOP

        exit(0); //processo filho fez o programa alvo executar durante 1 segundo, fim de seu proposito

    }else{ //espera pela exec do processo
        waitpid(pid,0,0);
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

    while(!feof(f)){ /*      LEITURA DO ARQUIVO       */
        
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

    /*          INICIO DO PROGRAMA            */

    int tempoTotalExec = get_tempoExecTotal(fila); //tempo total de execucao de todos os programas, quando esse tempo acabar, finalizar escalonador e o programa 
    int *runTime = (int*) malloc(sizeof(int)); //ha quanto tempo o programa esta rodando
    *runTime = 0;
    int tam  = get_line_size(fila); //tamanho total da linked list, e da quantidade de elementos possiveis a serem passados por parametro para o escalonador
    lever = 0; //booleana para verificar se ja ha pelo menos 1 elemento na RR

    El* target = NULL; //alvo  a ser retirado da LL e inserido da RR
    Fila *RR; // lista round-robin, com todos os processos na fila de prontos no momento

    printf("tempo total de execucao: %d\n!!!!!!!!!!!\n",tempoTotalExec);


    // TIRAR ESSE COMENTARIO

    int escalonador = fork();

    El* next; //elemento a ser inserido neste segundo na lista


    while(runTime != tempoTotalExec){ //CADA ITERACAO REPRESENTA 1 SEGUNDO!!!
        while(next_off(fila,runTime) != NULL){//enquanto houverem elementos a serem inseridos neste segundo:
            target = next_off(fila,runTime); //pega o elemento alvo
            fila = remove_of_line(fila,target); //remove ele da LL

            if(lever == 0){//se nao houver nenhum elemento na RR;
                RR = create_line(target);
            }else{ //se houver ao menos um elemento na RR
                insert_on_line(RR,target);
            }
        }

        go_robin(RR);

        sleep(1);

        *runTime++;//+1 segundo
    }
        /*      faz chamadas ao escalonador         */

    //faz fork, e faz chamadas ao escalonador para ele inserir na paradinha do lance

    //TIRAR ESSE COMENTARIO 

    print_line(fila);

    //printf("Hello world!\n");
    
    fclose(f);
    free(runTime);
    free_line(fila); //libera os mallocs todos

    return 0;
}
