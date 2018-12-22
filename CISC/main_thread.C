#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<unistd.h>


void * test_function( ){
   /*  while(1){
        printf("thread fonction");
        sleep(10);
    }  */
    int* nb_iteration = (int*) malloc(sizeof(int));
    * nb_iteration = rand()%6;
    for(int i=0;i<*nb_iteration;i++){
        printf("thread fonction %lu\n", (long)pthread_self());
        sleep(1);
  }
  return nb_iteration;
}


int main(int argc, char const *argv[])
{
    /* code */
    srand(time(NULL));
    
    //print_nber_thread2 = rand(); //---// ---2 
    pthread_t thread_test1;
    pthread_t thread_test2;
    pthread_create(&thread_test1, NULL, test_function, NULL); /*Premier NULL: Atthribut par
    défaut du thread.*/ 
    pthread_create(&thread_test2, NULL, test_function, NULL);

    int *nb_iteration = NULL;
    int *nb_iteration1 = NULL;
    pthread_join(thread_test1, (void*)&nb_iteration);
    pthread_join(thread_test2, (void*)&nb_iteration1);
    
    printf("nb_it= %lu\n", (long)*nb_iteration);
    printf("nb_it2= %lu\n", (long)*nb_iteration1);
    return 0;
}