/* DEFINIZIONE DEL FILE */
#ifndef strutture
#define strutture

#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <sys/sem.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>

/* STRUTTURA GENERALE PER LE INFORMAZIONI DELLA PARTITA */
struct Info{
	/*Dimensioni della matrice => inserite dall'utente */
	int rows;
	int colums;
	/*Simbolo dei giocatori => inseriti dall'utente */
	char symbol1[1];
	char symbol2[2];
	/*Chiave della partita*/
	int key_matrix;
	int Ngiocatore;
	int vittoria;
	int vincitore;
	int superVincita;
	int pareggio;
	int server_pid;
};

/* STRUTTURA GENERALE PER GIOCATORI */
struct Players{
	char name[20];
	int pid;
	int automatico;
};

struct Keys{
	int key_info_match;
	int matrix_key;
	int key_player1;
	int key_player2;
	int sem_key;
};

struct Matrix{
        char matrix[15][15];
};

#endif
