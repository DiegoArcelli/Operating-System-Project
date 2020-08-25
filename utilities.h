#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>

#define NUM_TURISTI 10
#define NUM_GUIDE 2
#define DIM_GRUPPO 4
#define BUFF_SIZE 10


// Struct per le informazioni relative alla guide
typedef struct {
    // Carattere che identifica la guida
    char id;
    // Array che memorizza gli id dei turisti che effettuano il tour
    int gruppo[DIM_GRUPPO];
    // Array che memorizza gli id dei turisti che provano a prenotarsi per il tour
    int prenotazioni[NUM_TURISTI];
    // Contatore del il numero dei turisti prendono parte al tour
    int num_gruppo; 
    // Contatore del il numero dei turisti che provano a prenotarsi per il tour
    int num_interessati; 
    // Euro guadagnati dalla guida in tutti i tour
    int saldo; 
} Guida;

void Pthread_mutex_lock(pthread_mutex_t *mtx);
void Pthread_mutex_unlock(pthread_mutex_t *mtx);
void Pthread_cond_signal(pthread_cond_t *cond);
void Pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t* mtx);
void Pthread_cond_broadcast(pthread_cond_t *cond);
void Pthread_mutex_init(pthread_mutex_t *mtx);
void Pthread_cond_init(pthread_cond_t* cond);
void Pthread_mutex_destroy(pthread_mutex_t *mtx);
void Pthread_cond_destroy(pthread_cond_t* cond);
void Pthread_create(pthread_t* tid, void *(*start_routine) (void *), int n);
void Pthread_join(pthread_t tid);
void Pthread_exit(void *retval);
void Mkfifo(char* path, mode_t mode);
void Unlink(char* path);
int Open(char* path, int flag);
void Close(int fd);
int Read(int fd, void *buf, size_t count);
int Write(int fd, void *buf, size_t count);
int seleziona_guida();
void inizializza_guide();