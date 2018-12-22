#include <pthread.h>
#include <stdio.h>
#include "Count.h"
#include "Watchdog.h"
#include <unistd.h>
#include <stdlib.h>

void * watchDog(void)
{   /*
	int tampon = 0;
	while(1)
	{
		pthread_mutex_lock(&mutex);
		if(GetCount()<=tampon)
		{
			printf("Oh probleme");
			//Lancer le thread Backup
		}
	tampon = GetCount();
	pthread_mutex_unlock(&mutex);
	}
     */
}



