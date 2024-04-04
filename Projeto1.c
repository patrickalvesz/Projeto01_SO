#define _GNU_SOURCE
#include <stdlib.h>
#include <malloc.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <sched.h>
#include <stdio.h>
#include <pthread.h>

// 64kB stack
#define FIBER_STACK 102464

// Estrutura para representar uma conta
struct c {
 int saldo;
 pthread_mutex_t lock; // Mutex para cada conta
};

typedef struct c conta;

conta from, to; // Contas
int valor; // Valor a ser transferido

// Função executada pela thread
int transferencia(void *arg)
{
 pthread_mutex_lock(&from.lock); // Bloqueia a conta from
 if (from.saldo >= valor){ // Se houver saldo suficiente
   from.saldo -= valor; // Retira o valor da conta from
   to.saldo += valor; // Adiciona o valor à conta to
 }
 pthread_mutex_unlock(&from.lock); // Desbloqueia a conta from

 printf("Transferência concluída com sucesso!\n");
 printf("Saldo de c1: %d\n", from.saldo);
 printf("Saldo de c2: %d\n", to.saldo);

 return 0;
}

int main()
{
 void* stack;
 pid_t pid;
 int i;

 // Aloca a pilha
 stack = malloc( FIBER_STACK );
 if ( stack == 0 )
 {
   perror("malloc: could not allocate stack");
   exit(1);
 }

 // Inicializa os mutexes
 pthread_mutex_init(&from.lock, NULL);
 pthread_mutex_init(&to.lock, NULL);

 // Todas as contas começam com saldo 100
 from.saldo = 100;
 to.saldo = 100;

 printf( "Transferindo 10 para a conta c2\n" );
 valor = 10;

 while(from.saldo > 0) { // Continua transferindo enquanto houver saldo
   // Chama a função clone para criar a thread
   pid = clone( &transferencia, (char*) stack + FIBER_STACK,
   SIGCHLD | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_VM, 0 );
   if ( pid == -1 )
   {
     perror( "clone" );
     exit(2);
   }
   waitpid(pid, NULL, 0); // Espera a transferência terminar antes de iniciar a próxima
 }

 // Libera a pilha
 free( stack );

 printf("Transferências concluídas e memória liberada.\n");

 // Destruir os mutexes
 pthread_mutex_destroy(&from.lock);
 pthread_mutex_destroy(&to.lock);

 return 0;
} 
