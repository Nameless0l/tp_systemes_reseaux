//
// Created by nicol on 02/11/2026.
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include "../includes/segdef.h"

void Initializations(int * shmid, int * semid) {
    //Récupération de l'identifiant du segment
    *shmid =shmget(cle,0,0);
    if(*shmid == -1) {
        perror("shmid");
        exit(0);
    }
    //Récupération des sémaphores
    *semid = semget(cle,0,0);
    if(*semid == -1) {
        perror("semid");
        exit(0);
    }
    //Initialisation générateur
    init_rand();
}

void acq_sem1(int semid, int sem) {
    struct sembuf sop;
    sop.sem_num = sem;
    sop.sem_op = -1; //Décroire de 1 le compteur revient ici à réserver la ressource
    sop.sem_flg = 0;
    if(semop(semid, &sop, 1) == -1) { //semop crée le wait non actif !
        perror("semop");
        exit(1);
    }
}

void wait_sem1(int semid, int sem) {
    struct sembuf sop;
    sop.sem_num = sem;
    sop.sem_op = 0; //0 est une option spéciale permettant d'attendre que le compteur du sémaphore soit nul (vraiment C Linux...)
    sop.sem_flg = 0;
    if(semop(semid, &sop, 1) == -1) { //semop crée le wait non actif !
        perror("semop");
        exit(1);
    }
}

void lib_sem1(int semid, int sem) {
    struct sembuf sop;
    sop.sem_num = sem;
    sop.sem_op = +1; //Augmenter de 1 le compteur revient ici à libérer la ressource
    sop.sem_flg = 0;
    if(semop(semid, &sop, 1) == -1) { //semop crée le wait non actif !
        perror("semop");
        exit(1);
    }
}


void initializeShmsegTabWithRandomValues(struct shmseg * shmseg, int shmid) {
    shmseg = shmat(shmid, NULL, 0);
    if(shmseg == (void *)-1) {
        perror("shmseg");
        exit(1);
    }

    for(int i = 0; i < maxval; i++) {
        shmseg->tab[i] = getrand();
    }

    if(shmdt(shmseg) == -1) {
        perror("shmdt");
        exit(1);
    }
}

long getClientResult(struct shmseg * shmseg, int shmid) {
    shmseg = shmat(shmid, NULL, 0); //Ouverture mémoire partagé du côté de ce process
    if(shmseg == (void *)-1) {
        perror("shmseg");
        exit(1);
    }

    long result_client = 0;
    for(int i = 0; i < maxval; i++) {
        result_client += shmseg->tab[i];
    }
    result_client = result_client / maxval;

    if(shmdt(shmseg) == -1) {            //Fermeture mémoire partagé du côté de ce process
        perror("shmdt");
        exit(1);
    }
    return result_client;
}

long getServerResult(struct shmseg * shmseg, int shmid) {
    shmseg = shmat(shmid, NULL, 0); //Ouverture mémoire partagé du côté de ce process
    if(shmseg == (void *)-1) {
        perror("shmseg");
        exit(1);
    }

    long result_server = shmseg->result;  //Acessible uniquement pendant lecture server

    if(shmdt(shmseg) == -1) {             //Fermeture mémoire partagé du côté de ce process
        perror("shmdt");
        exit(1);
    }

    return result_server;
}

//lib_sem(semid -> id du groupe de semaphore ,sem -> identifiant de chacunes de celle-ci)
//leurs rôles seront déduisibles par les defines du .h

//Processus du client
//Testé avec for i in {1..100}; do     ./TP_IPC & done
int main(void) {
    int shmid;
    int semid;
    long result_server;
    long result_client;
    struct shmseg * shmseg; //je suppose que les programmes se sont conventionnés à ce buff

    Initializations(&shmid, &semid);
    while(true){
        acq_sem1(semid,seg_dispo); //Mise à 0 du semaphore par ce process
        //-> écriture du tableau de données à envoyer et calcul de result_client
        initializeShmsegTabWithRandomValues(shmseg, shmid);
        result_client = getClientResult(shmseg, shmid);


        acq_sem1(semid,seg_init);  //Mise à 0 du semaphore par ce process
        wait_sem1(semid, res_ok);  //Attend passage à 0 côté serveur

        //->lecture résultat
        result_server = getServerResult(shmseg, shmid);

        lib_sem1(semid, seg_init); //Mise à 1 du semaphore par ce process
        acq_sem1(semid,res_ok);    //Attend passage à 1 côté serveur
        lib_sem1(semid, res_ok);   //(on triche on demandant res_ok et en le relâchant immédiatement)
        lib_sem1(semid, seg_dispo);//Mise à 1 du semaphore par ce process

        //->affichage du résultat calculé par le client et serveur
        printf("Resultat serveur : %ld \n\r",result_server);
        printf("Resultat client : %ld \n\r",result_client);

    }

}
