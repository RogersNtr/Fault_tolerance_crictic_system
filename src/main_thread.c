#include "../header/main_thread.h"

void * myFunction(void * arg){
	while(1){
		printf("mon thread\n");
		sleep(1);
	}
}

void * rand_print(void * arg){
	int * nb_it = (int*) malloc(sizeof(int)); 
	*nb_it = rand()%6;
	for(int i = 0; i<*nb_it; i++){
		printf("mon thread %ld\n", (long)pthread_self());
		sleep(1);
	}
	return nb_it;
}

int main (){
	srand(time(NULL));
	pthread_t myThread1;
	pthread_t myThread2;
	pthread_create(& myThread1, NULL,mean_calculation(10), NULL);
	//pthread_create(& myThread2, NULL,rand_print, NULL);
	float * nb_it;
	int * nb_it2;
	pthread_join(myThread1, ((void*)&nb_it));
	//pthread_join(myThread2, ((void*)&nb_it2));
	//printf("nb_it2 =%i \n", *nb_it2);
	printf("mean  = %f \n", *nb_it);

	return 0;
}

/*
int main1 (){
	pthread_t myThread1;
	pthread_create(& myThread1, NULL, myFunction, NULL);
	while(1){
		printf("main thread\n");
		sleep(1);
	}
}*/