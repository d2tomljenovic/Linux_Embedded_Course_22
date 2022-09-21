#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>


char getch(void);
char *niz;

static sem_t semCommandChange;
static sem_t semFinishSignal;

static pthread_mutex_t bufferAccess;

int printRandoms(int lower, int upper, int count)
{
    int i;
    int num = (rand() % (upper - lower + 1)) + lower;
    return num;
}

void NormalRandoms()
{
    int lenght = printRandoms(1, 50, 1);
    int k;
    niz = (char*)calloc(lenght+1, sizeof(char));
    for (int i = 0; i<lenght; i++){
    	while(1){
    		k = printRandoms(48, 90, 1);
    		if(!(k > 57 && k < 65)){
    			break;
    		}
    	}
    	niz [i] = k;
    }
    niz[lenght] = '\0';
    printf("lenght = %d, niz = %s", lenght, niz);
}


typedef struct
{
  int id;
  int data;
}Command;

Command com;

char test[5][20] =
{
 "STV ARNO",
 "NEMAM5",
 "IDEJU",
 "12 ZA 34",
 "OV0"
 };
 
 //todo u zavisnosti o enkodiranju u char deviceu promjeni char odgovor
 char odgovor[5][20] =
{
 "STV ARNO",
 "NEMAM5",
 "IDeJU",
 "12 ZA 34",
 "OV0"
 };

int compareString(char *test, char *odgovor)
{
	int rez = 1;
	int i = 0;
	while (test[i] != '\0'){
		if(test[i] != odgovor[i])
		{
			rez = 0;
			break;
		}
		i++;
	}
	return rez;
}

void printText()
{
	printf("\nIzaberite komandu:\n1:Normalni rezim rada.\n2:Test rezim rada.\n3:Test rezim rada sa ubacivanjem greske.\n4:Zaustavljanje slanja podataka.\n5:Prekid rada cele aplikacije.\n");
}

void printText1()
{
	for(int i=0;i < 5;i++)
	{
	printf("Niz%d %s \n", i, test[i]);
	}
}

/* Thread 1*/
void* th1 (void *param)
{
    char c;

    while (1)
    {
        if (sem_trywait(&semFinishSignal) == 0)
        {
            break;
        }

    	printText();
        c = getch();
	if( c < '0' || c > '5')
	{
		printf("Neispravan unos.");
		continue;
	}
	
	switch(c)
	{
	case '1':
		com.id = 1;
		sem_post(&semCommandChange);
	break;
	case '3':
	case '2':
		com.id = c - 48;
		printText1();
		c = getch();
		if( c < '0' || c > '5')
		{
			printf("Neispravan unos.");
			break;
		}
		com.data = c - 48 -1;
		printf("Izabaran %d.",(com.data + 1));
		sem_post(&semCommandChange);
		break;
	case '4':
		com.id = 4;
		sem_post(&semCommandChange);
	break;
        case '5':
       	
           /* Terminate thread; Signal the semaphore twice
           in order to notify both threads. */
           sem_post(&semCommandChange);
           sem_post(&semFinishSignal);
           sem_post(&semFinishSignal);
        break;
	}
    }

    return 0;
}

/* Thread 2 */
void* th2 (void *param)
{
    char c;
    int state = 0;
    int data = 0;
    int commid = 1;
	
    sem_wait(&semCommandChange);
    
    state = com.id;
    data = com.data;
    

    while (1)
    {
        if (sem_trywait(&semFinishSignal) == 0)
        {
            break;
        }
        
        switch(state)
        {
        case 1:
           NormalRandoms();
           sleep(1);
        break;
        case 2:
           printf("\n %d com2 %s \n",commid,test[data]);
           if(compareString(test[data], odgovor[data])){
           	printf("Isti su");
           	}
           	else{
           	printf("Nisu isti");
           	}
           sleep(1);
        break;
        case 3:
           printf("\n %d com3 %s \n",commid,test[data]);
           sleep(1);
        break;
        case 4:
           sem_wait(&semCommandChange);
           state = com.id;
           data = com.data;
           commid++;
        break;
        }
        
        if(sem_trywait(&semCommandChange) == 0)
        {
        	state = com.id;
        	data = com.data;
        	commid++;
	}

    }

    return 0;
}

int main (void)
{

    /* Thread IDs. */
    pthread_t hTh1;
    pthread_t hTh2;

    /* Create semEmpty, semFull and semFinishSignal semaphores. */
    sem_init(&semFinishSignal, 0, 0);
    sem_init(&semCommandChange, 0, 0);

    /* Initialise mutex. */
    pthread_mutex_init(&bufferAccess, NULL);

    /* Create threads: the producer and the consumer. */
    pthread_create(&hTh1, NULL, th1, 0);
    pthread_create(&hTh2, NULL, th2, 0);

    /* Join threads (wait them to terminate) */
    pthread_join(hTh1, NULL);
    pthread_join(hTh2, NULL);

    /* Release resources. */
    sem_destroy(&semFinishSignal);
    sem_destroy(&semCommandChange);
    pthread_mutex_destroy(&bufferAccess);

    printf("\n");

    return 0;
}
