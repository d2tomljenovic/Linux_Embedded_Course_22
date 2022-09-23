#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <fcntl.h>

char getch(void);
int sz1, sz2;
int flag = 0;
int flag2 = 0;

static sem_t semCommandChange;
static sem_t semFinishSignal;

static pthread_mutex_t bufferAccess;

typedef struct
{
  int cnt;
  char *path;
  char *modRada;
  char *izborTestnogNiza;
}Argumenti;

Argumenti ulazniArgumenti;

typedef struct
{
  char *niz;
  int cnt;
}CommandData;

CommandData commDataWrite;
CommandData commDataRead;

typedef struct
{
  int id;
  int data;
}Command;

Command com;

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
    char *niz;
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
    commDataWrite.niz= niz;
    commDataWrite.cnt= lenght;
    printf("<<<<Random niz za upis= %s, dužina random niza: %d.\n", niz, lenght);
}

void FileHandlerWrite(char *niz, int cnt)
{
	int filedevice;
	if(ulazniArgumenti.cnt > 1){
		filedevice = open(ulazniArgumenti.path, O_WRONLY);
	}
	else{
		filedevice = open("/dev/mydevice", O_WRONLY);
	}
	if(filedevice == -1)
	{
		printf("Error while opening device for writting\n");
		
		return -1;
	}
	else{
		sz1 = write(filedevice, niz, cnt);
	}
	close(filedevice);
}

void FileHandlerRead()
{
	int filedevice;
	char *c = (char *) calloc(100, sizeof(char));
	if(ulazniArgumenti.cnt >1){
		filedevice = open(ulazniArgumenti.path, O_RDONLY);
	}
	else{
		filedevice = open("/dev/mydevice", O_RDONLY);
	}
	if(filedevice == -1)
	{
		printf("Error while opening device for reading\n");
		
		return -1;
	}
	else{
		sz2 = read(filedevice, c, sz1);
		c[sz2] = '\0';
	}
	close(filedevice);
	commDataRead.niz = c;
	commDataRead.cnt = sz2+1;
	printf(">>>>Ispis niza iz devicea: %s\n", c);
}

char test[5][20] =
{
 "STV ARNO",
 "NEMAM5",
 "IDEJU",
 "12 ZA 34",
 "OV0"
 };
 
 //todo u zavisnosti o enkodiranju u char deviceu promjeni char odgovor
char odgovor[5][71] =
{
 "* * *   -   * * -       * -   - * -   - *   - - -",
 "- *   *   - -   * -   - -   * * * * *",
 "* *   - * *   *   * - - -   -",
 "* - - - -   * * - - -       - - * *   * -       * * * - -   * * * * -",
 "- - -   * * -   - - - - -"
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
	printf("Niz %d %s \n", i+1, test[i]);
	}
}

/* Thread 1*/
void* th1 (void *param)
{
    char c1;
    char c2;

    while (1)
    {
        if (sem_trywait(&semFinishSignal) == 0)
        {
            break;
        }

    	printText();
        //c = getch();
        if (ulazniArgumenti.cnt > 2 && flag == 0){
        	c1 = *ulazniArgumenti.modRada;
       		flag = 1;
        	}
        else{
        	c1 = getch();
        }
	if( c1 < '0' || c1 > '5')
	{
		printf("Neispravan unos c-a.\n");
		continue;
	}
	
	switch(c1)
	{
	case '1':
		com.id = 1;
		sem_post(&semCommandChange);
	break;
	case '3':
	case '2':
		com.id = c1 - 48;
		printText1();
		//c = getch();
		if (ulazniArgumenti.cnt > 2 && flag2 == 0){
        		c2 = *ulazniArgumenti.izborTestnogNiza;
       			flag2 = 1;
        	}
        	else{
        		c2 = getch();
        	}
		if( c2 < '0' || c2 > '5')
		{
			printf("Neispravan unos.\n");
			break;
		}
		com.data = c2 - 48 -1;
		printf("Izabaran %d.\n",(com.data + 1));
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
           FileHandlerWrite(commDataWrite.niz, commDataWrite.cnt);
           FileHandlerRead();
           sleep(5);
	   flag = 0;
        break;
        case 2:
           FileHandlerWrite(test[data], strlen(test[data]));
           FileHandlerRead();
           //printf("\n %d com2 %s \n",commid,test[data]);
           if(compareString(test[data], commDataRead.niz)){
           	printf("Upisani testni niz i odgovor su isti.\n");
           	}
           	else{
           	printf("Upisani testni niz i odgovor nisu isti.\n");
           	}
           sleep(3);
           state = 4;
           flag2 = 0;
        break;
        case 3:
           test[data][2] = 'a';
           FileHandlerWrite(test[data], strlen(test[data]));
           FileHandlerRead();
           //printf("\n %d com2 %s \n",commid,test[data]);
           if(compareString(test[data], commDataRead.niz)){
           	printf("Upisani testni niz i odgovor su isti.\n");
           	}
           	else{
           	printf("Upisani testni niz i odgovor nisu isti.\n");
           	}
           sleep(1);
           state = 4;
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

int main (int argc, char *argv[])
{
    
    ulazniArgumenti.cnt = argc;
    ulazniArgumenti.path = argv[1];
    ulazniArgumenti.modRada = argv[2];
    ulazniArgumenti.izborTestnogNiza = argv[3];
    
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
