#include "../header/main_thread.h"


pthread_cond_t condition_cpt = PTHREAD_COND_INITIALIZER; /* Création de la condition sur Cpt */
pthread_mutex_t mutex_cpt = PTHREAD_MUTEX_INITIALIZER; /* Création du mutex lié à Cpt*/


pthread_cond_t condition_SF = PTHREAD_COND_INITIALIZER; /* Création de la condition sur le fichier partagé*/
pthread_mutex_t mutex_SF = PTHREAD_MUTEX_INITIALIZER; /* Création du mutex sur fichier partagé*/

int Count = 0;
long deplacement = 0;

void * watchDog(void * arg);
struct Checkpoint * mean_calculation(int nb_iter);
void * mean_calculation_BackUp(int nb_iter);

int main (){
	pthread_t Primary;
	pthread_t Backup;
	pthread_t Watchdog;
        
        
        pthread_create(& Watchdog, NULL,watchDog,(void*)NULL);
	pthread_create(& Primary, NULL,mean_calculation(10), (void*)NULL);
	pthread_create(& Backup, NULL,mean_calculation_BackUp(10), (void*)NULL);
        
        pthread_join(Primary, NULL);
        pthread_join(Watchdog, NULL);
        pthread_join(Backup, NULL);

	return 0;
}

void * watchDog(void * arg){ 
    struct timespec ts;
    struct timeval tv;
    int rt;
    
    while(1){
        gettimeofday(&tv, NULL);
        ts.tv_sec = time(NULL) + 4000 / 1000;
        ts.tv_nsec = tv.tv_usec * 1000 + 1000 * 1000 * (4000 % 1000);
        ts.tv_sec += ts.tv_nsec / (1000 * 1000 * 1000);
        ts.tv_nsec %= (1000 * 1000 * 1000);
        pthread_mutex_lock (&mutex_cpt); // On verrouille le mutex 
        rt = pthread_cond_timedwait (&condition_cpt, &mutex_cpt, &ts); // On attend que la condition soit remplie 
        printf( "Watchdog Cpt : %d\n", Count);
        if (rt == ETIMEDOUT){
            //lancer backup
            printf("Lancement Opération de Récupération (TimeOut)\n");
            pthread_mutex_lock (&mutex_SF); // On verrouille le mutex 
            pthread_cond_signal (&condition_SF); /* On délivre le signal : condition remplie */
            pthread_mutex_unlock (&mutex_SF); /* On déverrouille le mutex */
            pthread_mutex_unlock (&mutex_cpt); // On déverrouille le mutex 
        }
        else
            pthread_mutex_unlock (&mutex_cpt); // On déverrouille le mutex 
    }
    pthread_exit(NULL); // Fin du thread
    
}

struct Checkpoint * mean_calculation(int nb_iter){
    
    FILE * lecture = NULL;
    
    srand(time(NULL));
    char tableau[100] = "";
    char tableau2[100] = "";
    
    getcwd(tableau, 100);
    getcwd(tableau2, 100);
    strcat(tableau, "/data_sensor");
    lecture = fopen(tableau, "r");
    
    float * mean = (float*)malloc(sizeof(float));
    char  chaine[6] = "";

	//Checkpoint
	long int filePointerCheckpoint = -1;
	struct Checkpoint* checkpoint;

    if (lecture != NULL){
        int end = 0;
        FILE * ecriture = NULL;
        strcat(tableau2, "/exec/share_file.txt");
        ecriture = fopen(tableau2, "w+");
        if (ecriture != NULL){
            fseek(lecture, deplacement, SEEK_SET);
            fseek(ecriture, 0, SEEK_END);
            do{
               *mean = 0;
                for(int i = 0; i<nb_iter; i++){
                    if(NULL==fgets(chaine, 6, lecture)){
                        end = 1;
                    }
                    else{
                    //printf("chaine : %s ", chaine);
					filePointerCheckpoint = ftell(lecture);
                 (*checkpoint).cursor_pointer_input = filePointerCheckpoint;
                 strcpy( (*checkpoint).filename, "data_sensor");
                 (*checkpoint).cursor_pointer_share_file = ftell(ecriture);
                    *mean+=atoi(chaine); 
					if(atoi(chaine) == 0){ // Si c'est 0 , on est tombé sur un caractère pas bon
                    	return (checkpoint);// du coup, on retourne le checkpoint
                    	}
                    }
                }
               if(end!=1){
                *mean/=nb_iter;

                pthread_mutex_lock (&mutex_SF); /* On verrouille le mutex */
                fprintf(ecriture, "%f\n", *mean);
                pthread_mutex_unlock (&mutex_SF); /* On déverrouille le mutex */

                printf("mean value : %f\n", *mean);

                pthread_mutex_lock (&mutex_cpt); /* On verrouille le mutex */
                Count++;
                printf("compteur courant : %d\n", Count);
                pthread_cond_signal (&condition_cpt); /* On délivre le signal : condition remplie */
                pthread_mutex_unlock (&mutex_cpt); /* On déverrouille le mutex */
               }
               sleep(rand()%6);
            }while(end!=1);
        }
        fclose(ecriture);
    }
    else {
        printf("Impossible d'ouvrir le fichier data_sensor");
    }
    fclose(lecture);
    pthread_exit(NULL); // Fin du thread
	return checkpoint;	
}

void * mean_calculation_BackUp(int nb_iter){
    pthread_mutex_lock (&mutex_SF); // On verrouille le mutex 
    pthread_cond_wait (&condition_SF, &mutex_SF); // On attend que la condition soit remplie 
    pthread_mutex_unlock (&mutex_SF); /* On déverrouille le mutex */
    printf("Lancement Module de BackUp");
    mean_calculation(nb_iter);
}