#include "utilities.h"

void Pthread_mutex_lock(pthread_mutex_t* mtx) {
    int err;
    if ((err = pthread_mutex_lock(mtx)) != 0) {
        errno = err;
        perror("lock");
        pthread_exit(&errno);
    }
}

void Pthread_mutex_unlock(pthread_mutex_t* mtx) {
    int err;
    if ((err = pthread_mutex_unlock(mtx)) != 0) {
        errno = err;
        perror("unlock");
        pthread_exit(&errno);
    }
}

void Pthread_cond_signal(pthread_cond_t* cond) {
    int err;
    if ((err = pthread_cond_signal(cond)) != 0) {
        errno = err;
        perror("signal");
        pthread_exit(&errno);
    }
}

void Pthread_cond_wait(pthread_cond_t* cond, pthread_mutex_t* mtx) {
    int err;
    if ((err = pthread_cond_wait(cond, mtx)) != 0) {
        errno = err;
        perror("wait");
        pthread_exit(&errno);
    }
}

void Pthread_cond_broadcast(pthread_cond_t* cond) {
    int err;
    if ((err = pthread_cond_broadcast(cond)) != 0) {
        errno = err;
        perror("broadcast");
        pthread_exit(&errno);
    }
}

void Pthread_mutex_init(pthread_mutex_t* mtx) {
    int err;
    if ((err = pthread_mutex_init(mtx, NULL)) != 0) {
        errno = err;
        perror("mutex init");
        pthread_exit(&errno);
    } 
}


void Pthread_cond_init(pthread_cond_t* cond) {
    int err;
    if ((err = pthread_cond_init(cond, NULL)) != 0) {
        errno = err;
        perror("cond init");
        pthread_exit(&errno);
    } 
}


void Pthread_mutex_destroy(pthread_mutex_t *mtx) {
    int err;
    if ((err = pthread_mutex_destroy(mtx)) != 0) {
        errno = err;
        perror("mutex destroy");
        pthread_exit(&errno);
    }
}


void Pthread_cond_destroy(pthread_cond_t* cond) {
    int err;
    if ((err = pthread_cond_destroy(cond)) != 0) {
        errno = err;
        perror("mutex destroy");
        pthread_exit(&errno);
    }
}


void Pthread_create(pthread_t* tid, void *(*start_routine) (void *), int n) {
    int err;
    if ((err = pthread_create(tid, NULL, start_routine, (void *)(intptr_t)n)) != 0) {
        errno = err;
        perror("thread create");
        pthread_exit(&errno);
    } 
}


void Pthread_join(pthread_t tid) {
    int err;
    if ((err = pthread_join(tid, NULL)) != 0) {
        errno = err;
        perror("thread create");
        pthread_exit(&errno);
    }
}



void Mkfifo(char* path, mode_t mode) {
    if (mkfifo(path, mode) == -1 && errno != EEXIST)  {
        int err = errno;
        perror("mkfifo");
        exit(err);
    }
}


void Unlink(char* path) {
    if (unlink(path) == -1) {
        int err = errno;
        perror("unlink");
        exit(err);
    }
}

int Open(char* path, int flag) {
    int fd;
    if ((fd = open(path, flag)) == -1) {
        int err = errno;
        perror("open");
        exit(err);
    }
    return fd;
}

void Close(int fd) {
    if (close(fd) == -1) {
        int err = errno;
        perror("close");
        exit(err);
    }
}

int Read(int fd, void *buf, size_t count) {
    int n;
    if ((n = read(fd, buf, count)) == -1) {
         int err = errno;
         perror("read");
         exit(err);
    }
    return n;
}

int Write(int fd, void *buf, size_t count) {
    int n;
    if ((n = write(fd, buf, count)) == -1) {
         int err = errno;
         perror("write");
         exit(err);
    }
    return n;
}

/* Funzione che genera a caso un numero intero tra 0 e 1. Lo 0 viene associato alla guida A, mentre l'1
viene associato alla guida B */
int seleziona_guida() {
    return  rand() % 2;
}


// Funzione per iniziallizare i valori degli attributi delle guide
void inizializza_guide(Guida* A, Guida* B) {
    for (int i = 0; i < DIM_GRUPPO; i++) {
        A->gruppo[i] = -1;
        B->gruppo[i] = -1;
    }
    A->id = 'A';
    B->id = 'B';
    A->num_gruppo = 0;
    B->num_gruppo = 0;
    A->num_interessati = 0;
    B->num_interessati = 0;
    A->saldo = 0;
    B->saldo = 0;
}