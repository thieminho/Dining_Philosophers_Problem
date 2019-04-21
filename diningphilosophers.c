#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <stdio.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/ipc.h>

#define LICZBAFILOZOFOW 5
#define CZASJEDZENIA 1
#define CZASMYSLENIA 3

#define FILOZOF numerprocesu
#define LEWY (numerprocesu == 0) ? LICZBAFILOZOFOW : numerprocesu - 1
#define PRAWY (numerprocesu+1)% LICZBAFILOZOFOW

#define MYSLENIE 0
#define GLODNY 1
#define JEDZENIE 2



int semset;
int numerprocesu;
char* stany;
int* zjedzone;
int posilki[]= {1,2,3,4,5};


void sem_init(int semnr, int value) {
    int s;
    s = semctl(semset, semnr, SETVAL, value);
}

void podnies(int semnr) {
    struct sembuf semops;
    int s;
    semops.sem_num = semnr;
    semops.sem_op = 1;
    semops.sem_flg = 0;
    s = semop(semset, &semops, 1);
}


void opusc(int semnr) {
    struct sembuf semops;
    int s;
    semops.sem_num = semnr;
    semops.sem_op = -1;
    semops.sem_flg = 0;
    s = semop(semset, &semops, 1);
}

void testuj(int p){
	if (stany[p] == GLODNY && stany[LEWY] != JEDZENIE && stany[PRAWY] != JEDZENIE )
        if((stany[LEWY]==GLODNY&&zjedzone[p]<=zjedzone[LEWY]) ||(stany[PRAWY]==GLODNY&&zjedzone[p]<=zjedzone[PRAWY])){
            stany[p]=JEDZENIE;
            printf("Filozof %d je\n", p );
            podnies(p);
            }
        else if(stany[LEWY]!=GLODNY && stany[PRAWY]!=GLODNY){
      		 stany[p]=JEDZENIE;
      		 printf("Filozof %d je\n", p );
      		 podnies(p);
            }
}

void podnieswidelce(){
    opusc(LICZBAFILOZOFOW);
    stany[FILOZOF]=GLODNY;
    printf("Filozof %d jest glodny\n", numerprocesu);
    testuj(FILOZOF);
    podnies(LICZBAFILOZOFOW);
    opusc(FILOZOF);
}

void odloz(){
    opusc(LICZBAFILOZOFOW);
	stany[FILOZOF]=MYSLENIE;
    printf("Filozof %d mysli\n", FILOZOF);
	testuj(LEWY);
	testuj(PRAWY);
    podnies(LICZBAFILOZOFOW);
}

void child(){

    while(1){
  	sleep(CZASMYSLENIA);
  	podnieswidelce();
    zjedzone[numerprocesu] += posilki[rand() % 5];
  	sleep(CZASJEDZENIA);
  	for (int i=0; i<5; ++i)
        printf ( " %d  ",zjedzone[i]);
    printf("\n");
  	odloz();
  }
}


void parent(){
    wait(NULL);
}

int main()
{

    int shmID;
    shmID = shmget(IPC_PRIVATE, LICZBAFILOZOFOW, IPC_CREAT | 0600);
    stany = (char*) shmat(shmID, NULL, 0);

    if ((long) shmID == -1 || (long) shmat == -1)
        exit(1);


    int shmID2;
    shmID2 = shmget(IPC_PRIVATE, LICZBAFILOZOFOW, IPC_CREAT | 0600);
    zjedzone = (int*) shmat(shmID2, NULL, 0);

    if ((long) shmID2 == -1 || (long) shmat == -1)
        exit(1);



    semset = semget(IPC_PRIVATE, LICZBAFILOZOFOW + 1, IPC_CREAT | 0600);
    if (semset == -1)
        exit(1);



    int i;
    for (i = 0; i < LICZBAFILOZOFOW; i++)
        sem_init(i,0);
    sem_init(i,1);
    for (i = 0; i < LICZBAFILOZOFOW; i++)
        stany[i]=MYSLENIE;

    for (i = 0; i < LICZBAFILOZOFOW; i++)
        zjedzone[i]=0;



    int pid;
    for(numerprocesu = 0; numerprocesu < LICZBAFILOZOFOW; numerprocesu++){
        pid = fork();
        if (pid == 0) break;
      }

    if (pid == 0)
        child();
    else if (pid > 0)
        parent();
    else
        exit(1);


    return 0;
}
