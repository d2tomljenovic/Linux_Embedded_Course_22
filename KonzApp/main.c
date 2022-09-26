#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <fcntl.h>
#include<sys/ioctl.h>

#define SET_LED _IOW(235,'0',int*)
#define SET_UNIT _IOW(235,'1',int*)

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
		filedevice = open("/dev/morse_device", O_WRONLY);
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
	char *c;
	c = (char *) calloc(501, sizeof(char));
	if(ulazniArgumenti.cnt >1){
		filedevice = open(ulazniArgumenti.path, O_RDONLY);
	}
	else{
		filedevice = open("/dev/morse_device", O_RDONLY);
	}
	if(filedevice == -1)
	{
		printf("Error while opening device for reading\n");
		
		return -1;
	}
	else{
		sz2 = read(filedevice, c, 501);
		c[sz2] = '\0';
	}
	close(filedevice);
	commDataRead.niz = c;
	commDataRead.cnt = sz2+1;
	printf(">>>>Ispis niza iz devicea: %s\n", c);
}

void FileHandlerIOCTL_SetLed(int *a)
{
	int filedevice;
	if(ulazniArgumenti.cnt >1){
		filedevice = open(ulazniArgumenti.path, O_RDONLY);
	}
	else{
		filedevice = open("/dev/morse_device", O_RDONLY);
	}
	if(filedevice == -1)
	{
		printf("Error while opening device for reading\n");
		
		return -1;
	}
	else{
		sz2 = ioctl(filedevice, SET_LED, (int*)a);
		
	}
	close(filedevice);
}

void FileHandlerIOCTL_SetUnit(int *a)
{
	int filedevice;
	if(ulazniArgumenti.cnt >1){
		filedevice = open(ulazniArgumenti.path, O_RDONLY);
	}
	else{
		filedevice = open("/dev/morse_device", O_RDONLY);
	}
	if(filedevice == -1)
	{
		printf("Error while opening device for reading\n");
		
		return -1;
	}
	else{
		sz2 = ioctl(filedevice, SET_UNIT, (int*)a);
		
	}
	close(filedevice);
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
 "* * *x-x* * -X* -x- * -x- *x- - -x",
 "- *x*x- -x* -x- -x* * * * *x",
 "* *x- * *x*x* - - -x-x",
 "* - - - -x* * - - -X- - * *x* -X* * * - -x* * * * -x",
 "- - -x* * -x- - - - -x"
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
	printf("\nIzaberite komandu:\n1:Normalni rezim rada.\n2:Test rezim rada.\n3:Test rezim rada sa ubacivanjem greske.\n4:Zaustavljanje slanja podataka.\n5:Prekid rada cele aplikacije.\n6:Izbor LED diode.\n7:Duljina unit-a.\n");
}

void printText1()
{
	for(int i=0;i < 5;i++)
	{
	printf("Niz %d %s \n", i+1, test[i]);
	}
}

void printText2()
{
	printf("Izaberi diodu: 0-Crvena, 1-Zelena\n");
}

/* Thread 1*/
void* th1 (void *param)
{
    char c1;
    char c2;
    int a;
    char unitStr[20] = {0};

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
	if( c1 < '0' || c1 > '7')
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
		if (ulazniArgumenti.cnt > 3 && flag2 == 0){
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
        case '6':
           printText2();
           c1 = getch();
           a = (int)(c1)-48;
           printf ("%d\n", a);
           FileHandlerIOCTL_SetLed(&a);
           printf ("%d\n", a);
	break;
	case '7':
	   printf("Upisi duljinu unit-a u ms:\n");
	   scanf("%s", unitStr) ;
  	   sscanf(unitStr, "%d", &a);
  	   printf ("%d\n", a);
           FileHandlerIOCTL_SetUnit(&a);
           printf ("%d\n", a);
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
           sleep(2);
	   flag = 0;
        break;
        case 2:
           FileHandlerWrite(test[data], strlen(test[data]));
           FileHandlerRead();
           if(compareString(odgovor[data], commDataRead.niz)){
           	printf("Upisani testni niz i odgovor su isti.\n");
           	}
           	else{
           	printf("Upisani testni niz i odgovor nisu isti.\n");
           	}
           sleep(2);
           state = 4;
           flag2 = 0;
        break;
        case 3:
           test[data][2] = 'a';
           FileHandlerWrite(test[data], strlen(test[data]));
           FileHandlerRead();
           if(compareString(odgovor[data], commDataRead.niz)){
           	printf("Upisani testni niz i odgovor su isti.\n");
           	}
           	else{
           	printf("Upisani testni niz i odgovor nisu isti.\n");
           	}
           sleep(2);
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
