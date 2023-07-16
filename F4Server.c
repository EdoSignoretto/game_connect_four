
/* PROGRAMMA SERVER
 * Definisce le dimensioni del campo da gioco
 * Predisporre l'area di memoria
 * Predisporre i semafori
 * Arbitrare la partita: verificare ad ogni mossa se c'è vincitore
 * Controlla la pressione dei tasti CTRL - C
*/

#include "strutture.h"

/* DEFINZIONI DI STRUTTURE GLOBALI E MEMORIE CONDIVISE */
struct Info * info_struct;
struct Keys * info_keys;
struct Matrix * info_matrix;
struct Players * info_player1;
struct Players * info_player2;

int keys_fd;
int info_fd;
int matrix_fd;
int player1_fd;
int player2_fd;
int semafori_fd;

/* VARIABILE GLOBALE PER PRESSIONE CTRL-C */
int ctrlC = 0;
int conta_ctrlC = 0;

/* FUNZIONE PER GESTIONE CTRL - C DEL SERVER */
void pressione_ctrlC(int sig){
	ctrlC++;
	if (ctrlC == 1)
		printf("\nALERT: hai premuto CTRL-C una volta, alla prossima pressione in qualunque momento il programma Server terminerà\n");
	else{
		printf("\nIL SERVER È TERMINATO\n");
		/* VERIFICA ESISTENZA GIOCATORI */
		if (info_struct->Ngiocatore == 2){
			/* KILL AD ENTRAMBI I GIOCATORI */
			kill(info_player1->pid, SIGTSTP);
			kill(info_player2->pid, SIGTSTP);
			signal(SIGTSTP, SIG_DFL);
		}
		else{
			kill(info_player1->pid, SIGTSTP);
			signal(SIGTSTP, SIG_DFL);
		}
		signal(SIGINT, SIG_DFL);
		exit(1);
	}
}

/* FUNZIONE PER GESTIONE CTRL - C DEL CLIENT */
/* TERMINAZIONE GIOCATORE 1 */
void pressione_ctrlC_giocatore1(int sig){
	printf("\nALERT: il giocatore %s ha abbandonato, %s ha vinto a tavolino\n", info_player1->name, info_player2->name);
	kill(info_player2->pid, SIGXFSZ);
	signal(SIGXFSZ, SIG_DFL);
	/* RIMUOVO LE MEMORIA OCCUPATA */
	shmdt(info_struct);
        shmctl(info_fd, IPC_RMID, NULL);
        shmdt(info_matrix);
        shmctl(matrix_fd, IPC_RMID, NULL);
        shmdt(info_player1);
        shmctl(player1_fd, IPC_RMID, NULL);
        shmdt(info_player2);
        shmctl(player2_fd, IPC_RMID, NULL);
        semctl(semafori_fd, 0, IPC_RMID);
	exit(1);
}

/* TERMINAZIONE GIOCATORE 2 */
void pressione_ctrlC_giocatore2(int sig){
	printf("\nALERT: il giocatore %s ha abbandonato, %s ha vinto a tavolino\n", info_player2->name, info_player1->name);
	kill(info_player1->pid, SIGXFSZ);
        signal(SIGXFSZ, SIG_DFL);
        /* RIMUOVO LE MEMORIA OCCUPATA */
        shmdt(info_struct);
        shmctl(info_fd, IPC_RMID, NULL);
        shmdt(info_matrix);
        shmctl(matrix_fd, IPC_RMID, NULL);
        shmdt(info_player1);
        shmctl(player1_fd, IPC_RMID, NULL);
        shmdt(info_player2);
        shmctl(player2_fd, IPC_RMID, NULL);
        semctl(semafori_fd, 0, IPC_RMID);
        exit(1);
}

int main(int argc, char * argv[]){
	/* VERIFICA DEI PARAMETRI INSERITI */
        if (argc <= 4){
                printf("\nALERT: Non sono stati inseriti i parametri correttamente\n");
                printf("Lanciare nuovamente il programma con ad esempio questa struttura: ./F4Server 5 5 X O\n");
		printf("Dimensioni e simboli possono essere inseriti a piacere\n");
                exit(1);
        }

	/* DEFINIZIONE DELLE FUNZIONI PER I SEGNALI */
	signal(SIGINT, pressione_ctrlC);
	signal(SIGUSR1, pressione_ctrlC_giocatore1);
	signal(SIGUSR2, pressione_ctrlC_giocatore2);

	/* GENERAZIONE DELLA MEMORIA CONDIVISA PER LE CHIAVI */
	keys_fd = shmget(2727, sizeof(struct Keys), 0777 | IPC_CREAT);
	if (keys_fd == -1){
		perror("Si è generato un errore nella creazione della memoria condivsa per le chiavi");
	}

	/* ATTACCO ALLA MEMORIA CONDIVISA */
	info_keys = (struct Keys *)shmat(keys_fd, NULL, 0);

	/* DEFINZIONE DELLE CHIAVI */
	info_keys->key_info_match = 2728;
	info_keys->matrix_key = 2729;
	info_keys->key_player1 = 2626;
	info_keys->key_player2 = 5365;
	info_keys->sem_key = 2829;

	/* GENERAZIONE DELLA MEMORIA CONDIVISA STRUTTURA INFORMAZIONI */
	info_fd = shmget(info_keys->key_info_match, sizeof(struct Info), 0777 | IPC_CREAT);
        if (info_fd == -1) {
                perror("Si è generato un errore nella creazione della memoria condivisa");
                exit(1);
        }
	/* ATTACCO ALLA MEMORIA CONDIVISA */
	info_struct = (struct Info *)shmat(info_fd, NULL, 0);
	info_struct->server_pid = getpid();

   	/* DEFINIZIONE DEI CAMPI INSERITI DALL'UTENTE */
	int next = 1;
	info_struct->rows = atoi(argv[1]);
	while(next != 0){
		if (info_struct->rows < 5){
			printf("Hai inserito un numero di righe non valido\n");
			printf("Rinserisci numero di righe maggiore di 5: ");
			scanf("%i", &info_struct->rows);
		}else
			next = 0;
	}
	next = 1;
	info_struct->colums = atoi(argv[2]);
	while(next != 0){
		if (info_struct->colums < 5){
			printf("Hai inserito un numero di colonne non valido\n");
			printf("Rinserisci numero di  colonne maggiore di 5: ");
			scanf("%i", &info_struct->colums);
		}else
			next  = 0;
	}
   	info_struct->symbol1[0] = *argv[3];
   	info_struct->symbol2[0] = *argv[4];

  	/* STAMPA DEI PARAMETRI INSERITI NELLA STRUTTURA */
   	printf("Numero di righe: %d\n", info_struct->rows);
   	printf("Numero di colonne: %d\n", info_struct->colums);
   	printf("Simbolo giocatore 1: %c\n", info_struct->symbol1[0]);
   	printf("Simbolo giocatore 2: %c\n", info_struct->symbol2[0]);

	/* GENERAZIONE MEMORIA CONDIVISA PER MATRICE */
	matrix_fd = shmget(info_keys->matrix_key, 16, 0777 | IPC_CREAT);
        if (matrix_fd == -1){
                perror("Si è generato un errore nella creazione della memoria condivisa per la matrice");
                exit(1);
        }

        /* ATTACCO ALLA MEMORIA CONDIVISA */
        info_matrix = (struct Matrix *) shmat(matrix_fd, NULL, 0);

	/* CREAZIONE DELLA MATRICE [ARRAY BIDIMENSIONALE] */
        for (int i = 0; i < info_struct->rows; i++){
                for(int j = 0; j < info_struct->colums; j++){
                        info_matrix->matrix[i][j] = ' ';
                }
        }

	/* GENERAZIONE DELLA MEMORIA CONDIVISA STRUTTURA GIOCATORE 1 */
        player1_fd = shmget(info_keys->key_player1, sizeof(struct Players), 0777 | IPC_CREAT);
        if (player1_fd == -1){
                perror("Si è generato un errore nella creazione della memoria condivisa della struttura giocatori");
                exit(1);
        }
        /* ATTACCO ALLA MEMORIA CONDIVISA DEL GIOCATORE 1 */
        info_player1 = (struct Players *)shmat(player1_fd, NULL, 0);
	info_player1->pid = 0;

	/* GENERAZIONE DELLA MEMORIA CONDIVISA STRUTTURA GIOCATORE 2 */
        player2_fd = shmget(info_keys->key_player2, sizeof(struct Players), 0777 | IPC_CREAT);
        if (player2_fd == -1) {
                perror("Si è generato un errore nella creazione della memoria condivisa della struttura giocatori");
                exit(1);
        }

        /* ATTACCO ALLA MEMORIA CONDIVISA DEL GIOCATORE 2 */
        info_player2 = (struct Players *)shmat(player2_fd, NULL, 0);
	info_struct->Ngiocatore = 1;
	info_struct->vincitore = 0;
	info_struct->superVincita = 0;

	/* GENERAZIONE DELLA MEMORIA CONDIVISA PER SEMAFORI */
	semafori_fd = semget(2829, 3, IPC_CREAT | 0777);
	if (semafori_fd == -1 ){
		perror("Errore nella creazione della memoria condivisa per i semafori");
		exit(1);
	}

	/* INIZIALIZZAZIONE SEMAFORI */
	struct sembuf sem;
	sem.sem_op = -1;
	sem.sem_num = 0;
	sem.sem_flg = 0;

	/* DECREMENTO PER ATTESA GIOCATORE 1 */
	do{
		conta_ctrlC = ctrlC;
		printf("Il gioco sta attendendo il giocatore 1...\n");
		semop(semafori_fd, &sem, 1);
	}while(conta_ctrlC != ctrlC && info_struct->Ngiocatore == 0);

	printf("GIOCATORE 1 TROVATO!\n");
	printf("Nome giocatore 1: %s\n", info_player1->name);
	printf("Il simbolo del giocatore 1 sarà: %c\n", info_struct->symbol1[0]);

	/* DECREMENTO PER ATTESA GIOCATORE 2 */
        do{
		conta_ctrlC = ctrlC;
		printf("Il gioco sta attendendo il giocatore 2...\n");
        	semop(semafori_fd, &sem, 1);
	}while (conta_ctrlC != ctrlC && info_struct->Ngiocatore == 1);

        printf("GIOCATORE 2 TROVATO!\n");
        printf("Nome giocatore 2: %s\n", info_player2->name);
	printf("Il simbolo del giocatore 2 sarà: %c\n", info_struct->symbol2[0]);

	/* INIZIO PARTITA */
	info_struct->vittoria = 0;
	int mosse = 0;
	while(info_struct->vittoria != 1){
		/* INIZIALIZZAZIONE SEMAFORI PER ALTERNANZA GIOCATORI */
		sem.sem_flg = 0;

		/* MOSSA GIOCATORE 1 => SBLOCCO*/
		sem.sem_op = 1;
		sem.sem_num = 1;
		do{
			conta_ctrlC = ctrlC;
			semop(semafori_fd, &sem, 1);
		}while(conta_ctrlC != ctrlC);

		/* BLOCCO DEL SERVER */
		sem.sem_op = -1;
		sem.sem_num = 0;
		do{
                        conta_ctrlC = ctrlC;
                        semop(semafori_fd, &sem, 1);
                }while(conta_ctrlC != ctrlC);

		/* SERVER SBLOCCATO DAL GIOCATORE 1 + BLOCCO DEL GIOCATORE 1 */
		/* VERIFICA VINCITORE DOPO MOSSA GIOCATORE 1 */
		printf("\nGiocatore 1 ha terminato la mossa\n");
		mosse++;
		if (mosse == (info_struct->rows * info_struct->colums)){
			info_struct->vittoria = 1;
			info_struct->pareggio = 1;
			printf("La partita è terminata in pareggio\n");
			break;
		}

		/* VERIFICA VINCITORE ORIZZONTALE */
                int count = 0;
                for (int temp_r = 0; temp_r < info_struct->rows; temp_r++){
                        for(int temp_c = 0; temp_c < info_struct->colums -1; temp_c++){
				if (info_matrix->matrix[temp_r][temp_c] != ' '){
                                	if (info_matrix->matrix[temp_r][temp_c] == info_matrix->matrix[temp_r][temp_c + 1]){
                                        	count++;
                                	}else
                                        	count = 0;
                                	if (count == 3){
						if( temp_c < info_struct->colums - 2)
							if (info_matrix->matrix[temp_r][temp_c + 1] == info_matrix->matrix[temp_r][temp_c + 2])
								info_struct->superVincita = 1;
                                        	printf("\n\nTROVATO UN VINCITORE: PARTITA TERMINATA\n\n");
						if ( info_matrix->matrix[temp_r][temp_c] == info_struct->symbol1[0] ){
                                                        printf("Il vincitore è il giocatore 1: %s\n", info_player1->name);
                                                }else
                                                        printf("Il vincitore è il giocatore 2: %s\n", info_player2->name);
                                        	info_struct->vittoria = 1;
						info_struct->vincitore = 1;
						temp_r = info_struct->rows;
                                        	break;
                                	}

				}
                        }
			count = 0;
                }

		if(info_struct->vittoria == 1)
			break;

                /* VERIFICA VINCITORE VERTICALE */
                count = 0;
                for(int temp_c = 0; temp_c < info_struct->colums; temp_c++){
                        for(int temp_r = 0; temp_r < info_struct->rows - 1; temp_r++){
				if (info_matrix->matrix[temp_r][temp_c] != ' '){
	                                if (info_matrix->matrix[temp_r][temp_c] == info_matrix->matrix[temp_r + 1][temp_c]){
        	                                count++;
                	                }else
                        	                count = 0;
                                	if (count == 3){
                                        	printf("\n\nTROVATO UN VINCITORE: PARTITA TERMINATA\n\n");
						if ( info_matrix->matrix[temp_r][temp_c] == info_struct->symbol1[0] ){
                                                        printf("Il vincitore è il giocatore 1: %s\n", info_player1->name);
                                                }else
                                                        printf("Il vincitore è il giocatore 2: %s\n", info_player2->name);
	                                        info_struct->vittoria = 1;
						info_struct->vincitore = 1;
						temp_c = info_struct->colums;
                	                        break;
                        	        }
				}
                        }
			count = 0;
                }

		if (info_struct->vittoria == 1)
			break;

		/* MOSSA GIOCATORE 2 => SBLOCCO */
		sem.sem_num = 2;
		sem.sem_op = 1;
		do{
                        conta_ctrlC = ctrlC;
                        semop(semafori_fd, &sem, 1);
                }while(conta_ctrlC != ctrlC);

		/* BLOCCO SERVER */
		sem.sem_op = -1;
		sem.sem_num = 0;
		do{
                        conta_ctrlC = ctrlC;
                        semop(semafori_fd, &sem, 1);
                }while(conta_ctrlC != ctrlC);

		/* SERVER SBLOCCATO DAL GIOCATORE 2 + BLOCCO DEL GIOCATORE 2 */
        	/* VERIFICA VINCITORE DOPO MOSSA GIOCATORE 2 */
 	      	printf("\nGiocatore 2 ha terminato la mossa\n");

		mosse++;
                if (mosse == (info_struct->rows * info_struct->colums)){
                        info_struct->vittoria = 1;
			info_struct->pareggio = 1;
			printf("La partita è terminata in pareggio\n");
                        break;
                }

        	/* VERIFICA VINCITORE ORIZZONTALE */
                count = 0;
                for (int temp_r = 0; temp_r < info_struct->rows; temp_r++){
                        for(int temp_c = 0; temp_c < info_struct->colums -1; temp_c++){
				if (info_matrix->matrix[temp_r][temp_c] != ' '){
	                                if (info_matrix->matrix[temp_r][temp_c] == info_matrix->matrix[temp_r][temp_c + 1]){
        	                                count++;
                	                }else
                        	                count = 0;
                                	if (count == 3){
						if( temp_c < info_struct->colums - 2)
                                                        if (info_matrix->matrix[temp_r][temp_c + 1] == info_matrix->matrix[temp_r][temp_c + 2])
                                                                info_struct->superVincita = 1;
	                                        printf("\n\nTROVATO UN VINCITORE: PARTITA TERMINATA\n\n");
						if ( info_matrix->matrix[temp_r][temp_c] == info_struct->symbol1[0] ){
                                                        printf("Il vincitore è il giocatore 1: %s\n", info_player1->name);
                                                }else
                                                        printf("Il vincitore è il giocatore 2: %s\n", info_player2->name);
        	                                info_struct->vittoria = 1;
						info_struct->vincitore = 2;
						temp_r = info_struct->rows;
                        	                break;
					}
                                }
                        }
			count = 0;
                }

		if (info_struct->vittoria == 1)
			break;

                /* VERIFICA VINCITORE VERTICALE */
                count = 0;
                for(int temp_c = 0; temp_c < info_struct->colums; temp_c++){
                        for(int temp_r = 0; temp_r < info_struct->rows - 1; temp_r++){
				if (info_matrix->matrix[temp_r][temp_c] != ' '){
	                                if (info_matrix->matrix[temp_r][temp_c] == info_matrix->matrix[temp_r + 1][temp_c]){
        	                                count++;
                	                }else
                        	                count = 0;
                                	if (count == 3){
	                                        printf("\n\nTROVATO UN VINCITORE: PARTITA TERMINATA\n\n");
						if ( info_matrix->matrix[temp_r][temp_c] == info_struct->symbol1[0] ){
							printf("Il vincitore è il giocatore 1: %s\n", info_player1->name);
						}else
							printf("Il vincitore è il giocatore 2: %s\n", info_player2->name);
        	                                info_struct->vittoria = 1;
						info_struct->vincitore = 2;
						temp_c = info_struct->colums;
                        	                break;
                                	}
				}
                        }
			count = 0;
                }
	}

	printf("\nIL PROGRAMMA  E' TERMINATO\n");
	/* LIBERO MEMORIA OCCUPATA */
	shmdt(info_struct);
	shmctl(info_fd, IPC_RMID, NULL);
	shmdt(info_matrix);
	shmctl(matrix_fd, IPC_RMID, NULL);
	shmdt(info_player1);
	shmctl(player1_fd, IPC_RMID, NULL);
	shmdt(info_player2);
	shmctl(player2_fd, IPC_RMID, NULL);
	semctl(semafori_fd, 0, IPC_RMID);
	return (0);
}


