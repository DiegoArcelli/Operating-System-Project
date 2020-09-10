#include "utilities.h"

Guida str_guida[2];

pthread_mutex_t mtx_guide;
pthread_mutex_t mtx_giorno;
pthread_mutex_t mtx_guida[2];
pthread_mutex_t mtx_pipe[2];

pthread_cond_t guide_pronte;
pthread_cond_t attesa_guide;
pthread_cond_t stampa_giorno;
pthread_cond_t attesa_turisti[2];
pthread_cond_t tour[2];

int turisti_pronti = 0;
int count_guide_pronte = 2;
int flag_guide = 2;
int giorno = 0;
int flag_giorno = 0;
int count_arrivi[2] = {0, 0};
volatile sig_atomic_t fine = 0;
int ultimo_giro = 0;
int flag_ultimo_giro_guide[2] = {0, 0};

// Gestore della SIGINT e della SIGQUIT
void gestore_segnali(int signum) {
    if (signum == SIGINT) {
        /* La SIGINT attiva un flag che permette alle due guide di potersi
        sincronizzare per effettuare l'ultimo tour (quindi non termina immediatamente
        il processo)*/
        fine = 1;
        Write(STDOUT_FILENO, "\n", 2);
    } else if (signum == SIGQUIT) {
        /* La SIQUIT fa terminare immediatamente il processo, ma prima elimina le pipe e
        stampa il saldo totale delle guide (la printf non è una funzione safe ma comunque
        il processo viene terminato)*/
        printf("[AGENZIA] Saldo totale guide: %d euro\n", str_guida[0].saldo + str_guida[1].saldo);
        fflush(stdout);
        Unlink("pipe_guida_A");
        Unlink("pipe_guida_B");
        _exit(1);
    }

}


void* guida(void* arg) {

    int id = (intptr_t)arg;
    char mess[BUFF_SIZE];
    char pipe_path[20];
    if (id == 0) {
        strcpy(pipe_path, "pipe_guida_A");
        strcpy(mess, "4 euro");
    } else {
        strcpy(pipe_path, "pipe_guida_B");
        strcpy(mess, "10 euro");
    }



    while (1) {

        // Le guide inizializzano i valori degli attributi per il nuovo tour

        for (int i = 0; i < str_guida[id].num_gruppo; i++) {
            str_guida[id].gruppo[i] = -1; // il valore -1 indica che il posto è vuoto
        }
        str_guida[id].num_gruppo = 0;
        str_guida[id].num_interessati = 0;

        /* La guida si mette in attesa sulla varabile condizionale guide_pronte finchè tutti  i
        sono pronnti per inziziare il nuovo tour della giornata */

        Pthread_mutex_lock(&mtx_guide);
        while(turisti_pronti < NUM_TURISTI) {
            Pthread_cond_wait(&guide_pronte, &mtx_guide);
        }


        /* L'ultima delle due guide ad usicere dalla wait reiposta i flag condivisi tra le guide */
        flag_guide--;
        if (flag_guide == 0) {
            turisti_pronti = 0;
            flag_guide = 2;
        }

        /* Segmento di codice che regola la stampa a schermo del numero del giorno. La prima guida ad
        arrivara stampa il giorno e imposta flag_giorno ad 1 per segnalare all'altra guida che ha già
        effettuato la stampa. La seconda ad arrivare reimposta flag_giorno a 0 per l'iterazione successiva */

        Pthread_mutex_lock(&mtx_giorno);
        fflush(stdout);
        if (flag_giorno == 0) {
            ++flag_giorno;
            printf("\n[GIORNO %d]\n", ++giorno);
            fflush(stdout);
        } else {
            flag_giorno = 0;
        }
        Pthread_mutex_unlock(&mtx_giorno);

        /* Quando viene ricevuto il segnale SIGINT e la variabile fine viene impostata ad 1, entrambe
        le guide aggiornano un loro flag personale per prepararsi al tour finale. Quando entrambe le
        guide hanno settato il flag allora l'agenzia stampa l'inizio dell'ultimo tour e la variabile
        ulitmo giro (che serve a regolare l'uscita dal loop) viene impostata da 1. Si attende che
        entrambe le guide abbiano settato  */ 

        if (fine == 1) {
            if (flag_ultimo_giro_guide[id] == 0) {
                flag_ultimo_giro_guide[id] = 1;
            }
            if (flag_ultimo_giro_guide[id] + flag_ultimo_giro_guide[!id] == 2) {
                ultimo_giro = 1;
                printf("[AGENZIA]: Sta iniziando l'ultimo tour\n");
                fflush(stdout);
            }
        }

        // La guida segnala ai turisti che stanno per iniziare i tour
        Pthread_cond_broadcast(&attesa_guide);
        Pthread_mutex_unlock(&mtx_guide);

        printf("[GUIDA %c]: Pronto per ricevere turisti\n", str_guida[id].id);
        fflush(stdout);
        
        /* Ognuna delle due guide si mette in attesta sulla corrispettiva variabile condizionale
        attesa_turisti fintanto che si forma un gruppo di 4 turisti per il tuor. Siccome è possibile
        che il numero di tursti che vogliono effetuare il tour con una guida sia minore di 4 si gestisce 
        tale eventualita verificando se la somma tra il numero turisti che hanno espresso il loro
        interesse per la guida A con il numero di turisti che hanno espresso il loro interesse per la
        guida B è 10. Se lo è allora la guida inizia il tour con meno di 4 turisti. */

        Pthread_mutex_lock(&mtx_guida[id]);
        while (str_guida[id].num_gruppo < DIM_GRUPPO) {
            Pthread_mutex_lock(&mtx_guide);
            if (str_guida[id].num_interessati + str_guida[!id].num_interessati == NUM_TURISTI) {
                Pthread_mutex_unlock(&mtx_guide);
                break;
            } else {
                Pthread_mutex_unlock(&mtx_guide);
                Pthread_cond_wait(&attesa_turisti[id], &mtx_guida[id]);
            }
        }
        printf("[GUIDA %c]: Numero turisti %d\n", str_guida[id].id, str_guida[id].num_gruppo);
        fflush(stdout);
        Pthread_mutex_unlock(&mtx_guida[id]);


        /* La guida inizia la comunicazione tramite named pipe con i turisti presenti nel suo gruppo
        mettendosi in comunicazione con un turista alla volta. La corretta sincronizzazione dell'invio
        e della ricezione dei messaggi con il turista è garantita dal fatto che la read e la write
        sono entrambe bloccanti */

        for (int i = 0; i < str_guida[id].num_gruppo; i++) {
            char buff[BUFF_SIZE];
            int wfd = Open(pipe_path, O_WRONLY);
            Write(wfd, mess, BUFF_SIZE);
            Close(wfd);
            int rfd = Open(pipe_path, O_RDONLY);
            Read(rfd, buff, BUFF_SIZE);
            Close(rfd);
            if (id == 0) {
                printf("[GUIDA %c]: Il turista %d ha effettuato il pagamento di 4 euro\n", str_guida[id].id , str_guida[id].gruppo[i]);
                fflush(stdout);
                str_guida[id].saldo += 4;
            } else {
                printf("[GUIDA %c]: Il turista %d ha effettuato il pagamento di 10 euro\n", str_guida[id].id , str_guida[id].gruppo[i]);
                fflush(stdout);
                str_guida[id].saldo += 10;
            }
        }

        /* Terminate le comunicazioni la guida A inizia il tour attendendo 3 secondi e la guida B attendendo
        5 secondi*/
        
        if (id == 0) {
            sleep(3);
        } else {
            sleep(5);
        }

        // Al termine del tour la guida stampa i soldi che ha guadagnato in tutti i tour
        printf("[GUIDA %c]: Bilancio: %d euro\n", str_guida[id].id , str_guida[id].saldo);
        fflush(stdout);

        /* Sveglia i turisti del gruppo che sono in attesa sulla variabile condizionale tour della
        corrispettiva guida */

        Pthread_cond_broadcast(&tour[id]);

        if (ultimo_giro == 1) {
            printf("[GUIDA %c]: Tour chiusi\n", str_guida[id].id);
            fflush(stdout);
            break;
        }
    }
    pthread_exit((void*)0);
}


void* turista(void* arg) {

    int id_turista = (intptr_t)arg;


    while (1) {

        char buff[BUFF_SIZE];
        char pipe_path[20];
        char mess[BUFF_SIZE];
        int pos;

        /* Il turista sceglie con quale guida provare a partecipare al tuor e imposta il path della
        named pipe per la comunicazione in base alla scelta */ 
        int scelta = seleziona_guida();
        if (scelta == 0) {
            strcpy(pipe_path, "pipe_guida_A");
            strcpy(mess, "4 euro");
        } else {
            strcpy(mess, "10 euro");
            strcpy(pipe_path, "pipe_guida_B");
        }

        /* Il turista memorizza nella variabile pos il suo ordine di arrivo per la guida scelta. La
        variabile pos serve a gestire l'accesso al tour: i primi quattro turisti ad arrivare (quindi
        quelli con pos < 4) saranno quelli che parteciperanno al tour della loro guida. Il turista però
        non inserisce ancora il sui id nell'array del gruppo della guida scelta, poichè a questo punto
        del codice è possibile che la guida stia ancora effettuando il tour precedente. È stata scelta
        questa modalità di prenotazione dei turisti per avvantaggiare i turisti che non sono riusciti a
        rientrare nel gruppo per il tour: in questo modo, mentre gli altri turisti stanno effettuando il
        tour, loro possono già prenotarsi per il tour della giornata successiva. In questo modo si evita
        che un turista non riesca mai a partecipare ad un tour.
        Altra nota: sebbene la scelta di quale turista parteciperà al tour e quale no viene presa in
        qesto punto si è scelto di effettuare la stampa di questa informazione successivamente, quando
        le guide hanno terminato il tour in corso e sono pronte per partire con il successivo, per
        rendere più leggibile l'output del programma */

        Pthread_mutex_lock(&mtx_guida[scelta]);
        pos = count_arrivi[scelta]++;
        Pthread_mutex_unlock(&mtx_guida[scelta]);
        

        /* Il turista segnala alle guide in attesa sulla variabile condizionale guide_pronte di essere pronto
        e si mette in attesa sulla variabile condizionale attesa_guide. Verrà poi risvegliato da un delle
        due guide quando tutti e 10 i turisti saranno pronti */
        Pthread_mutex_lock(&mtx_guide);
        if (turisti_pronti < NUM_TURISTI) {
            turisti_pronti++;
            Pthread_cond_broadcast(&guide_pronte);
            Pthread_cond_wait(&attesa_guide, &mtx_guide);
        }
        Pthread_mutex_unlock(&mtx_guide);


        /* In base al loro ordine di arrivo (memorizzato nella variabile pos) i turisti o si inseriscono
        nel gruppo per il tour o vengono esclusi e riprovano il giorno sucessivo */
        Pthread_mutex_lock(&mtx_guida[scelta]);
        Pthread_mutex_lock(&mtx_guide);
        str_guida[scelta].num_interessati++;
        Pthread_mutex_unlock(&mtx_guide);
        if (pos < DIM_GRUPPO) {

            /* Se il turista riesce a prendere parte al tour inserisce il suo id nell'array gruppo della
            guida scelta. Nota: la variabile num_gruppo della struct Guida, oltre ha memorizzare il numero
            di turisti attualmente presenti nel gruppo, viene anche utilizzata dai turisti come indice
            dell'array per inserirsi nel gruppo */

            str_guida[scelta].gruppo[str_guida[scelta].num_gruppo++] = id_turista;
            printf("[TURISTA %d]: Mi sono prenoatato per la guida %c\n", id_turista, str_guida[scelta].id);
            fflush(stdout);
            count_arrivi[scelta]--;
            Pthread_cond_signal(&attesa_turisti[scelta]);
            Pthread_cond_signal(&attesa_turisti[!scelta]);
            Pthread_mutex_unlock(&mtx_guida[scelta]);

            /* I turisiti del gruppo si mettono, uno alla volta, in comunicazione della guida. Il turista
            il lock lo rilascia solo dopo aver ricevuto ed inivato il messaggio alla guida. La corretta sequenza
            delle due operazioni è garantita dal fatto che la read e la write sono bloccanti */

            Pthread_mutex_lock(&mtx_pipe[scelta]);
            int rfd = Open(pipe_path, O_RDONLY);
            Read(rfd, buff, BUFF_SIZE);
            Close(rfd);
            int wfd = Open(pipe_path, O_WRONLY);
            Write(wfd, mess, BUFF_SIZE);
            Close(wfd);
            Pthread_mutex_unlock(&mtx_pipe[scelta]);
            
            /* Il turista si mette in attesa sulla variabile condizionale tour, finchè la sua non termina la sleep
            ed effettua la broadcast per svegliare i turisti */

            Pthread_mutex_lock(&mtx_guida[scelta]);
            Pthread_cond_wait(&tour[scelta], &mtx_guida[scelta]);
            printf("[TURISTA %d]: Tour con guida %c finito\n", id_turista, str_guida[scelta].id);
            fflush(stdout);
            Pthread_mutex_unlock(&mtx_guida[scelta]);

        } else {
            
            /* I turisti che non sono risuciti a prenotarsi lo stampano a schermo, attendono due secondi
            e provano a riprenotarsi per il giorno successivo */

            printf("[TURISTA %d]: Posti finiti per guida %c\n", id_turista, str_guida[scelta].id);
            fflush(stdout);
            count_arrivi[scelta]--;
            Pthread_cond_signal(&attesa_turisti[scelta]);
            Pthread_cond_signal(&attesa_turisti[!scelta]);
            Pthread_mutex_unlock(&mtx_guida[scelta]);
        }

        sleep(2);

        /* La variabile ultimo giorno viene settata ad 1 quando, dopo aver ricevuto il segnale SIGINT
        le due guide si sincronizzano per effettuare l'ultimo tour, */

        if (ultimo_giro == 1) {
            break;
        }

    }
    pthread_exit((void*)0);
}


int main(int argc, char* argv[]) {
    
    srand(time(0));

    pthread_t thread_A;
    pthread_t thread_B;
    pthread_t thread_turisti[NUM_TURISTI];

    // Assegna il nuovo gestore ai segnali SIGINT e SIGQUIT
    struct sigaction new_action;
    memset(&new_action, 0, sizeof(struct sigaction));
    new_action.sa_handler = gestore_segnali;
    sigaction(SIGQUIT, &new_action, NULL);
    sigaction(SIGINT, &new_action, NULL);

    // Inizializzazione di mutex e variabili condizionali
    Pthread_mutex_init(&mtx_guide);
    Pthread_mutex_init(&mtx_guida[0]);
    Pthread_mutex_init(&mtx_guida[1]);
    Pthread_mutex_init(&mtx_pipe[0]);
    Pthread_mutex_init(&mtx_pipe[1]);
    Pthread_mutex_init(&mtx_giorno);
    Pthread_cond_init(&tour[0]);
    Pthread_cond_init(&tour[1]);
    Pthread_cond_init(&guide_pronte);
    Pthread_cond_init(&attesa_turisti[0]);
    Pthread_cond_init(&attesa_turisti[1]);

    /* Vengono create due named pipe, una per gestire le comunicazione con la guida A e l'altra 
    per le comunicazioni con la guida B */
    Mkfifo("pipe_guida_A", 0666);
    Mkfifo("pipe_guida_B", 0666);
    inizializza_guide(&str_guida[0], &str_guida[1]);

    // Creazione dei thread
    Pthread_create(&thread_A, guida, 0);
    Pthread_create(&thread_B, guida, 1);
    for (int i = 0; i < NUM_TURISTI; i++) {
        Pthread_create(&thread_turisti[i], turista, i);
    }
    
    Pthread_join(thread_A);
    Pthread_join(thread_B);
    for (int i = 0; i < NUM_TURISTI; i++) {
        Pthread_join(thread_turisti[i]);
    }

    printf("[AGENZIA] Saldo totale guide: %d euro\n", str_guida[0].saldo + str_guida[1].saldo);

    // Si distruggono mutex e variaili condizionali e si eliminano le pipe
    Unlink("pipe_guida_A");
    Unlink("pipe_guida_B");
    Pthread_mutex_destroy(&mtx_guide);
    Pthread_mutex_destroy(&mtx_guida[0]);
    Pthread_mutex_destroy(&mtx_guida[1]);
    Pthread_mutex_destroy(&mtx_pipe[0]);
    Pthread_mutex_destroy(&mtx_pipe[1]);
    Pthread_mutex_destroy(&mtx_giorno);
    Pthread_cond_destroy(&tour[0]);
    Pthread_cond_destroy(&tour[1]);
    Pthread_cond_destroy(&guide_pronte);
    Pthread_cond_destroy(&attesa_turisti[0]);
    Pthread_cond_destroy(&attesa_turisti[1]);
    
    return 0;
}