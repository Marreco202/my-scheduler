#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/stat.h>

#define PATH "./testes/programs/"
#define START 0
#define CONT 1
#define STOP 2
#define TERM 3

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

typedef struct el El;

struct fila{
    El *curr;
    struct fila *prox;
};

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

void copy_el(El* from, El* to){

    to->name = from->name;
    to->prio = from->prio;
    to->tempo_atual = from->tempo_atual;
    to->tempo_init = from->tempo_init;
    to->tempo_total = from->tempo_total;
    to->minha_vez = from->minha_vez;

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

    if(f == NULL){
        return NULL;
    }

    while(f!=NULL){
        if(f->curr->tempo_init == tempo_atual){
            El* target = (El*) malloc(sizeof(El));
            copy_el(f->curr,target);
            return target;
        }
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
                //free(buf->curr);
                //free(buf);
                return f;
            }else{
                ant->prox = buf->prox; // skip 1
                //free(buf->curr);
                //free(buf);
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
        //free(ref->curr);
        //free(ref);
    }
}

void hands_down(Fila *f){// COPIAR E COLAR NO CODIGO ANTIGO! ||TESTADO E FUNCIONANDO!!! || abaixa as maos de todos os elementos da linked list
    while(f!=NULL){
        f->curr->minha_vez = 0;
        f = f->prox;
    }
}

El* find_prio(Fila *RR){// TROCAR PARA O ARQUIVO NORMAL||TESTADO E FUNCIONANDO || procura pelo proximo elemento da round robin de maior prioridade 

    int lowest_prio; //maior prioridade encontrada ate entao

    Fila* candidatos ; //linked list de todos os candidatos de maiores prioridades 
    int lever = 0;

    Fila *loop1, *loop2, *loop3;
    loop1 = RR, loop2 = RR;

    El* escolhido; //escolhido para ser o da vez vide a RR atual

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
            
            El* qg = loop2->curr;

            if(lever == 0){
                candidatos = create_line(qg);
                lever++;
            }
            else insert_on_line(candidatos,qg);
        }
        loop2 = loop2->prox;
    }
    loop3 = candidatos;

    //printf("FILA DE CANDIDATOS: \n");
    //print_line(loop3);

    while(loop3 != NULL){ //verifica se ha alguem com a mao abaixada (ou seja, se alguem de maior prioridade nao tiver sido selecionado nesta rodada)
        if(get_line_size(candidatos) == 1){
            candidatos->curr->minha_vez = 1;
            return candidatos->curr;
        }

        else if(loop3->curr->minha_vez == 0){

            escolhido = loop3->curr;
            escolhido->minha_vez = 1;

            return escolhido;
        }
        loop3 = loop3->prox;
    }
    /*
    if(loop3 == NULL){
        printf("ERRO NA FIND PRIO\n");
        exit(1);
    }
    */
    hands_down(candidatos); //se todos estiverem com as maos levantadas, abaixa todas elas...

    escolhido = candidatos->curr;
    //  copy_el(candidatos->curr,escolhido); //... e seleciona o primeiro da LL arbitrariamente || ISSO PODE GERAR RESULTADOS INESPERADOS, PQ EU NAO ESTOU DEFININDO EXATAMENTE QUEM DEVE ENTRAR QUE HORA NO PROGRAMA!!!
    //free_line(candidatos); || NAO PODEMOS DAR FREE NESSES CARAS AQUI, PQ ELES SAO REFERNCIAS AOS ELEMENTOS DA LL ORIGINAL

    //printf("NOVA FILA RR: \n");
    //print_line(RR);

    return escolhido;
}

char* program_name(int int_name){

        //msg("******************");

        char* ponto_barra = (char*) malloc(sizeof(char)*4); //  ./ + null
        ponto_barra[0] = '.';
        ponto_barra[1] = '/';
        ponto_barra[2] = ' ';
        ponto_barra[3] = '\0';

        strcpy(ponto_barra, "./");
        //printf("ponto barra: %s\n",ponto_barra);

        char* numero = (char*) malloc(sizeof(char)*2);
        numero[0] = int_name + '0';
        numero[1] = '\0';

        //printf("numero: %s\n",numero);

        strcat(ponto_barra,numero);
        //free(numero);


        //printf("nome do programa: %s\n",ponto_barra);

        //msg("******************");

        return ponto_barra;
    }

int search_name(int* v, int tam, int target){

    for(int i = 0; i<tam; i++)
        if(v[i] == target)
            return v[i];
    return -1; //nao encotrou o nome
}

int get_index(int *v, int tam, int target){

    for(int i = 0; i<tam; i++)
        if(v[i] == target)
            return i;
    return -1; //nao foi encontrado o nome target
}

void msg(char* s){
    printf("%s\n",s);
}

void status(El* now, int state){

    if(state == START)
        printf("PROCESSO p%d FOI INICIADO. %d SEGUNDOS RESTANTES...\n",now->name, now->tempo_total - now->tempo_atual);
    else if(state == CONT)
        printf("PROCESSO p%d FOI CONTINUADO. %d SEGUNDOS RESTANTES...\n",now->name, now->tempo_total - now->tempo_atual);
    else if(state == STOP)
        printf("PROCESSO p%d FOI INTERROMPIDO. %d SEGUNDOS RESTANTES...\n",now->name, now->tempo_total - now->tempo_atual);
    else if(state == TERM)
        printf("PROCESSO p%d FOI TERMINADO. %d SEGUNDOS RESTANTES...\n",now->name, now->tempo_total - now->tempo_atual);
}


void go_robin(Fila *RR, int* pids, int* p_names, int tam){/* TESTAR || funcao do escalonador efetivamente */

    El* next;
    printf("INICIO DE GO ROBIN\n");

    //int lever = 0;
    next = find_prio(RR);

    char* to_run = program_name(next->name); // "./<numero_do_programa>"
    int i = get_index(p_names,tam,next->name);
    
    int child_process_pid;

    if(pids[i] == 0){ //caso o processo nao tenha sido iniciado...
        child_process_pid= fork();

        if(child_process_pid == 0){//novo processo

            //printf("PROCESSO p%d ALOCADO EM PIDS\n",next->name);
            status(next,START);
            execlp(to_run,to_run,NULL);

        }else if(child_process_pid > 0){//processo RR main aguarda 1 segundo, e sigstop
            next->tempo_atual++;
            pids[i] = child_process_pid;

            //printf("PAI DE p%d AGUARDANDO SUA EXECUCAO\n",next->name);
            sleep(1);
            if(next->tempo_atual >= next->tempo_total){//caso o tempo tenha acabado, sigterm

                //printf("PAI DE p%d ENVIA SIGTERM PARA p%d (pid: %d)\n",next->name,next->name,pids[i]);
                status(next,TERM);
                kill(pids[i],SIGTERM);
                RR = remove_of_line(RR,next);
                

            } else{//caso nao tenha acabado, manda o um SIGSTOP
                
                //printf("PAI DE p%d ENVIA SIGSTOP PARA p%d (pid: %d)\n",next->name,next->name,pids[i]);
                status(next,STOP);
                kill(pids[i],SIGSTOP);
                
            }
        }

    }else if(pids[i] > 0){//caso o processo tenha se iniciado

        //printf("PROCESSO p%d JA EXISTIA EM PIDS",next->name);

        kill(pids[i],SIGCONT);
        status(next,CONT);
        next->tempo_atual++;
        sleep(1);

        if(next->tempo_atual >= next->tempo_total){
            //printf("PAI DE p%d ENVIA SIGTERM PARA p%d (pid: %d)\n",next->name,next->name,pids[i]);
            kill(pids[i],SIGTERM);
            status(next,TERM);
            RR = remove_of_line(RR,next);
        }else{
            //printf("PAI DE p%d ENVIA SIGSTOP PARA p%d (pid: %d)\n",next->name,next->name,pids[i]);
            kill(pids[i],SIGSTOP);
            status(next,STOP);
        }

    }
    
    printf("FIM DA EXECUCAO DE RR PARA p%d\n",next->name);

}

void zero_array(int *v, int tam){ //limpa o lixo de memoria de um array 
    for(int i = 0; i<tam; i++) v[i] = 0;
}

void get_names(int* v, Fila* f){ //transfere os nomes da fila para um array de int

    int i = 0;

    while(f != NULL){
        v[i] = f->curr->name;
        i++;
        f = f->prox;
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
    int runTime = 0;  //ha quanto tempo o programa esta rodando
    int tam  = get_line_size(fila); //tamanho total da linked list, e da quantidade de elementos possiveis a serem passados por parametro para o escalonador
    lever = 0; //booleana para verificar se ja ha pelo menos 1 elemento na RR

    El* target = NULL; //alvo  a ser retirado da LL e inserido da RR
    Fila *RR; // lista round-robin, com todos os processos na fila de prontos no momento

    print_line(fila);

    printf("tempo total de execucao: %d\n!!!!!!!!!!!\n",tempoTotalExec);

    int id1 = shmget(IPC_PRIVATE,sizeof(int) * tam , IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR); //alocando memoria compartilhada

    int* pids = shmat(id1,0,0); //lista de todos os pids que foram inicializados. usamos isso para mandar os sinais aos processos || attach na memoria compartilhada
    int* p_names = (int*) malloc(sizeof(int)*tam);

    zero_array(p_names,tam); 
    zero_array(pids,tam);

    get_names(p_names,fila);//passa os nomes da LL para um array de nomes



    while(runTime < tempoTotalExec){ //CADA ITERACAO REPRESENTA 1 SEGUNDO!!!
        printf("--> SEGUNDO %d\n",runTime);

        while(next_off(fila,runTime) != NULL){//enquanto houverem elementos a serem inseridos neste segundo:
            target = next_off(fila,runTime); //pega uma COPIA do endereco alvo

            fila = remove_of_line(fila,target); //remove um elemento da LL com o MESMO NOME do elemento alvo
            
            if(lever == 0){//se nao houver nenhum elemento na RR;
                RR = create_line(target);
                lever++;
            }else{ //se houver ao menos um elemento na RR
                insert_on_line(RR,target);
            }

        }
        //print_line(RR);

        go_robin(RR,pids,p_names,tam);

        //sleep(1);

        runTime++;//+1 segundo
    }
    
    for(int i = 0; i<tam; i++){
        kill(pids[i],SIGKILL);
        printf("MATEI O PROGRAMA de pid: %d\n",pids[i]);
    }


    //faz fork, e faz chamadas ao escalonador para ele inserir na paradinha do lance

    //print_line(fila);

    //printf("Hello world!\n");
    
    shmctl(id1,IPC_RMID,0); //libera a memória alocada previamente a partir do id associado a ela
    fclose(f);
    //free(runTime);
    //free_line(fila); //libera os mallocs todos

    return 0;
}
