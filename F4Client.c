/* PROGRAMMA CLIENT
 * Si occupa di far giocare singolo giocatore
 * Raccogliendo la mossa del giocatore
 * Visualizzare rettangolo di gioco ad ogni mossa
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
int pid_player1;
int pid_player2;

/* FUNZIONE PER GESTIONE CTRL - C DEL CLIENT */
void pressione_ctrlC(int sig){
	printf("\nALERT: Hai deciso di abbandonare la partita!\n");
	if (pid_player1 == getpid()){
		kill(info_struct->server_pid, SIGUSR1);
		exit(1);
	}
	else{
		kill(info_struct->server_pid, SIGUSR2);
		exit(1);
	}
}

void ctrlC_client(int sig){
	printf("\nALERT: hai vinto a tavolino perché l'avversario ha abbandonato la partita!\n");
	exit(1);
}

void ctrlC_server(int sig){
	printf("\nIl programma SERVER ha abbandonato. La partita è terminata senza vincitori!\n");
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
	/* DEFINZIONE DEI SEGNALI */
	signal(SIGINT, pressione_ctrlC);
	signal(SIGXFSZ, ctrlC_client);
	signal(SIGTSTP, ctrlC_server);

	/* GENERAZIONE DELLA MEMORIA CONDIVISA PER LE CHIAVI */
	keys_fd = shmget(2727, sizeof(struct Keys), 0777);
	if(keys_fd == -1){
		perror("\nErrore nell'accesso alla struttura della chiavi");
		exit(1);
	}

	/* ATTACCO ALLA MEMORIA CONDIVISA */
	info_keys = (struct Keys *)shmat(keys_fd, NULL, 0);

	/* MEMORIA CONDIVISA STRUTTURA INFORMAZIONI */
	info_fd = shmget(info_keys->key_info_match, sizeof(struct Info), 0777);
	if (info_fd == -1){
		perror("Errore nell'accesso alla memoria condivisa della struttura");
		exit(1);
	}

   	/* LETTURA DEI PARAMENTRI DA TASTIERA
    	 * nomeUtente del giocatore
	 * nomeUtente * mi segnale che giocatore sarà il computer
   	 */

	/* VERIFICA SE NOME UTENTE INSERITO */
	if (argc <= 1){
		printf("ALERT: Non è stato inserito un nome utente\n");
		printf("Lanciare nuovamente il programma con questa struttura: ./F4Client nomeUtente\n");
		exit(1);
	}

	/* STAMPA DEL REGOLAMENTO DI GIOCO */
	printf("E' stato lanciato il gioco FORZA 4\n\nQuesto è il regolamento del gioco:\n");
	printf(" - Utente può scegliere se giocare contro il computer o contro un altro utente\n");
	printf(" - A turno i giocatori dovranno inserire un gettone in una colonna della matrice di gioco\n");
	printf("   - Alternanza dell'inserimento dei gettoni è gestita dal Server del gioco\n");
	printf("   - Se un giocatore è il computer l'inserimento sarà casuale\n");
	printf(" - Per vincere l'utente deve posizionare 4 gettoni vicini orizzontalmente o verticalmente\n");
	printf("   - Se un utente riesci a formare una colonna o riga da 5 gettoni compie una supervincita\n");
	printf("   - Se termina lo spazio di gioco e nessun utente è riuscito a vincere, finisce il pareggio\n\n");

	/* STAMPA NOME UTENTE */
	printf("Il nome utente del giocatore é: ");
 	for(int i = 1; i <= argc-1; i++){
    		printf("%s", argv[i]);
		char temp = *argv[i];
		if(temp  == '*')
			printf("\nQuesto utente è un utente automatico\n");
	}
	printf("\n");

	/* DEFINZIONE DELLA STRUTTURA GIOCATORI */
	struct Players * info_player1;
	struct Players * info_player2;

	/* GENERAZIONE DELLA MEMORIA CONDIVISA STRUTTURA GIOCATORI */
        player1_fd = shmget(info_keys->key_player1, sizeof(struct Players), 0777);
	player2_fd = shmget(info_keys->key_player2, sizeof(struct Players), 0777);
        if (player1_fd == -1){
                perror("Si è generato un errore nella creazione della memoria condivisa della struttura giocatori");
                exit(1);
        }
	if (player2_fd == -1){
		perror("Si è generato un errore nella creazione della memoria condivisa della struttura Players");
		exit(1);
	}

        /* ATTACCO ALLA MEMORIA CONDIVISA */
        info_player1 = (struct Players *)shmat(player1_fd, NULL, 0);
	info_player2 = (struct Players *)shmat(player2_fd, NULL, 0);

        /* DEFINIZIONE DEI CAMPI INSERITI DALL'UTENTE */
	if (info_player1->pid == 0){
		strcpy(info_player1->name, argv[1]);
		info_player1->pid = getpid();
	}else{
		strcpy(info_player2->name, argv[1]);
		info_player2->pid = getpid();
	}
	pid_player1 = info_player1->pid;
	pid_player2 = info_player2->pid;

	/* DEFINIZIONE GIOCATORE AUTOMATICO */
	for(int i = 1; i <= argc - 1; i++){
		char temp = *argv[i];
		if(temp == '*'){
			if (getpid() == info_player1->pid)
				info_player1->automatico = 1;
			else
				info_player2->automatico = 2;
		}
	}

	/* ACCESSO ALLA MEMORIA CONDIVISA */
	info_struct = (struct Info *)shmat(info_fd, NULL, 0);

	/* STAMPA DELL'INFORMAZIONI DI GIOCO */
	printf("Numero di righe della matrice di gioco: %d\n", info_struct->rows);
	printf("Numero di colonne della matrice di gioco: %d\n", info_struct->colums);
	printf("Simbolo giocatore 1: %c\n", info_struct->symbol1[0]);
	printf("Simbolo giocatore 2: %c\n", info_struct->symbol2[0]);

	/* GENERAZIONE DELLA MEMORIA CONDIVISA PER SEMAFORI */
        semafori_fd = semget(info_keys->sem_key, 2, 0777);
        if (semafori_fd == -1){
                perror("Errore nella creazione della memoria condivisa per i semafori");
                exit(1);
        }

        /* INIZIALIZZAZIONE SEMAFORI */
        struct sembuf sem;
	sem.sem_op = -1;
	sem.sem_num = 0;
	sem.sem_flg = 0;

	/* ACCESSO MEMORIA CONDIVISA DELLA MATRICE */
	matrix_fd = shmget(info_keys->matrix_key, 16, 0777);
	if (matrix_fd == -1){
		perror("Errore nell'accesso alla memoria condivisa della matrice");
		exit(1);
	}
	info_matrix = (struct Matrix *)shmat(matrix_fd, NULL, 0);

	/* RICERCA GIOCATORE 1 */
	if (info_struct->Ngiocatore == 1){
		printf("\nSei il giocatore 1\n");
		printf("In attesa del giocatore 2..\n");
		info_struct->Ngiocatore = 2;
		sem.sem_op = 1;

		semop(semafori_fd, &sem, 1);

		sem.sem_num = 1;
		sem.sem_op = -1;
                semop(semafori_fd, &sem, 1);
	}else{
		/* RICERCA GIOCATORE 2 */
		printf("\nSei il giocatore 2\n");
		printf("Sta per iniziare la partita..\n\n");
		sem.sem_op = 1;

                semop(semafori_fd, &sem, 1);

		sem.sem_num = 2;
		sem.sem_op = -1;
                semop(semafori_fd, &sem, 1);
	}

	/* CICLO WHILE PER ALTERNANZA GIOCATORI */
	int i, j, pos, temp_rows, temp_colums;
	while (info_struct->vittoria != 1){

		/* STAMPA MATRICE PER ENTRAMBI I GIOCATORI */
		printf("STATO DELLA MATRICE\n");
		for (i = 0; i < info_struct->rows; i++){
			for(j = 0; j < info_struct->colums; j++){
                                printf("------");
                        }
                        printf("-\n");
                        for(j = 0; j < info_struct->colums; j++){
                                printf("|     ");
                        }
                        printf("|\n");
                        for(j = 0; j < info_struct->colums; j++){
                                printf("|  %c  ", info_matrix->matrix[i][j]);
                        }
                        printf("|\n");
                        for(j = 0; j < info_struct->colums; j++){
                                printf("|     ");
                        }
                        printf("|\n");
                }
                for(j = 0; j < info_struct->colums; j++){
                        printf("------");
                }
                printf("-\n");

		/* ASSEGNAMENTO SIMBOLO PER INSERIMENTO */
		char simbolo;
		if (getpid() == info_player1->pid){
			simbolo = info_struct->symbol1[0];
			printf("SIMBOLO: %c\n", simbolo);
		}else{
			simbolo = info_struct->symbol2[0];
			printf("SIMBOLO: %c\n", simbolo);
		}

		/* GENERAZIONE DEL GETTONE */
		int value;
		if(getpid () == info_player1->pid){
			if(info_player1->automatico == 1){
				srand(time(0));
				temp_colums = rand() % info_struct->colums;
				printf("Colonna di inserimento del gettone generata casualmento\n");
				value = 1;
				while (value != 0){
					if (info_matrix->matrix[0][temp_colums] != ' '){
						srand(time(0));
						temp_colums = rand() % info_struct->colums;
					}else
						value = 0;
				}
			}else{
				printf("Inserisci la colonna dove inserire il tuo gettone: ");
       				scanf("%i", &pos);
       				temp_colums = pos - 1;
				value = 1;
				while (value != 0){
					if (info_matrix->matrix[0][temp_colums] != ' ' || pos > info_struct->colums){
						printf("Hai inserito il gettone in una colonna piena o non esistente\n");
						printf("Sei stato fortunato puoi inserire nuovamente: ");
						scanf("%i", &pos);
						temp_colums = pos - 1;
					}
					else
						value = 0;
				}
			}
		}
		if(getpid() == info_player2->pid){
			if(info_player2->automatico == 2){
				srand(time(0));
				temp_colums = rand() % info_struct->colums;
				printf("Colonna di inserimento del gettone generata casualmente\n");
				value = 1;
				while (value != 0){
					if (info_matrix->matrix[0][temp_colums] != ' '){
						srand(time(0));
						temp_colums =  rand() % info_struct->colums;
					}else
						value = 0;
				}
			}else{
				printf("Inserisci la colonna dove inserire il tuo gettone: ");
				scanf("%i", &pos);
				temp_colums = pos - 1;
				value = 1;
				while (value != 0){
					if (info_matrix->matrix[0][temp_colums] != ' ' || pos > info_struct->colums){
						printf("Hai inserito il gettone in una colonna piena o non esistente\n");
						printf("Sei stato fortunato puoi inserire nuovamente: ");
						scanf("%i", &pos);
						temp_colums = pos - 1;
					}else
						value = 0;
				}
			}
		}

		/* INSERIMENTO DEL GETTONE GENERATO */
		value = 1;
		int count = 1;
		temp_rows = info_struct->rows - count;
		while (value != 0){
			if (info_matrix->matrix[temp_rows][temp_colums] != ' '){
				count++;
				temp_rows = info_struct->rows - count;
			}else{
        			info_matrix->matrix[temp_rows][temp_colums] = simbolo;
				value = 0;
			}
		}

		/* STAMPA DELLA MATRICE L' INSERIMENTO */
        	for (int i = 0; i < info_struct->rows; i++){
                	for(j = 0; j < info_struct->colums; j++){
                       		printf("------");
               		}
               		printf("-\n");
               		for(j = 0; j < info_struct->colums; j++){
                       		printf("|     ");
               		}
               		printf("|\n");

                	for(j = 0; j < info_struct->colums; j++){
                       		printf("|  %c  ", info_matrix->matrix[i][j]);
               		}
               		printf("|\n");

			for(j = 0; j < info_struct->colums; j++){
                       		printf("|     ");
               		}
               		printf("|\n");
       		}
       		for(j = 0; j < info_struct->colums; j++){
             	       	printf("------");
               	}
               	printf("-\n");

		/* CONTROLLO PAREGGIO */
		if (info_struct->pareggio == 1)
			break;

		/* INIZIALIZZO SEMAFORI PER ALTERNANZA GIOCATORI */
		/* ALTERNANZA GIOCATORE 1 */
		if (getpid() == info_player1->pid){
			/* SBLOCCO DEL SERVER */
			sem.sem_flg = 0;
			sem.sem_op = 1;
			sem.sem_num = 0;

                        semop(semafori_fd, &sem, 1);

			/* BLOCCO IL GIOCATORE 1 */
			sem.sem_num = 1;
			sem.sem_op = -1;
                        semop(semafori_fd, &sem, 1);
		}
		else if(getpid() == info_player2->pid){
			/* SBLOCCO IL SERVER */
			sem.sem_flg = 0;
			sem.sem_op = 1;
			sem.sem_num = 0;

                        semop(semafori_fd, &sem, 1);

			/* BLOCCO IL GIOCATORE 2 */
			sem.sem_num = 2;
			sem.sem_op = -1;
                        semop(semafori_fd, &sem, 1);
		}
	}
	printf("PARTITA TERMINATA, IL GIOCO SI E' CONCLUSO\n");
	if(info_struct->vincitore == 1){
		if(getpid() == info_player1->pid){
			printf("HAI VINTO %s\n", info_player1->name);
			if(info_struct->superVincita == 1)
				printf("HAI FATTO UNA SUPER VINCITA, RIGA DA 5 GETTONI\n");
		}else
			printf("HAI PERSO %s\n", info_player2->name);
	}else if(info_struct->vincitore == 2){
		if(getpid() == info_player1->pid)
			printf("HAI PERSO %s\n", info_player1->name);
		else{
			printf("HAI VINTO %s\n", info_player2->name);
			if(info_struct->superVincita == 1)
				printf("HAI FATTO UNA SUPER VINCITA, RIGA DA 5 GETTONI\n");
		}
	}else
		printf("LA PARTITA E' TERMINATA IN PAREGGIO\n");

	return 0;
}
