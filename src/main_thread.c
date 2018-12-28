#include <signal.h>

#include "../header/main_thread.h"


pthread_cond_t condition_cpt = PTHREAD_COND_INITIALIZER; /* Création de la condition sur Cpt */
pthread_mutex_t mutex_cpt = PTHREAD_MUTEX_INITIALIZER; /* Création du mutex lié à Cpt*/


pthread_cond_t condition_SF = PTHREAD_COND_INITIALIZER; /* Création de la condition sur le fichier partagé*/
pthread_mutex_t mutex_SF = PTHREAD_MUTEX_INITIALIZER; /* Création du mutex sur fichier partagé*/

fpos_t *reading_cursor;
int nb_iter =1000;

void * watchDog(void * arg);
void * mean_calculation(void * arg);
void * mean_calculation_BackUp(void * arg);
struct timespec timer_creation(int number);

int main (){
    pthread_t Primary;
    pthread_t Backup;
    pthread_t Watchdog;

    int s, s1, s2;

    reading_cursor = (fpos_t*)malloc(sizeof(fpos_t));
   
    pthread_mutex_init(&mutex_cpt,NULL);
    pthread_mutex_init(&mutex_SF,NULL);
    pthread_cond_init(&condition_SF,NULL);
    pthread_cond_init(& condition_cpt,NULL);

    s = pthread_create(& Watchdog, NULL,watchDog,(void*)NULL);
    if(s!=0)
        printf("problem with WD\n");
    else 
        printf(" no problem with WD\n");

    s1 = pthread_create(& Primary, NULL,mean_calculation,(void*)NULL);
    if(s1!=0)
        printf("problem with P\n");
    else 
        printf(" no problem with P\n");
    s2 = pthread_create(& Backup, NULL,mean_calculation_BackUp,(void*)NULL);
    if(s2!=0)
        printf("problem with BU\n");
    else 
        printf(" no problem with BU\n");
    
    //pthread_join(Watchdog, NULL);
    pthread_join(Primary, NULL);
    pthread_join(Backup, NULL);
    return 0;
}

void * watchDog(void * arg){
    struct timespec ts;
    int rt;
    
    while(1){
        ts = timer_creation(4);
        pthread_mutex_lock (&mutex_cpt); // On verrouille le mutex 
        rt = pthread_cond_timedwait (&condition_cpt, &mutex_cpt, &ts); // On attend que la condition soit remplie 
        pthread_mutex_unlock (&mutex_cpt); // On déverrouille le mutex 
        if (rt == ETIMEDOUT){
            //lancer backup
            printf("Lancement Opération de Récupération (TimeOut)\n");
            pthread_mutex_lock (&mutex_SF); // On verrouille le mutex 
            pthread_cond_signal (&condition_SF); /* On délivre le signal : condition remplie */
            pthread_mutex_unlock (&mutex_SF); /* On déverrouille le mutex */
            sleep(1);
            pthread_mutex_lock (&mutex_cpt); // On verrouille le mutex 
            pthread_cond_wait (&condition_cpt, &mutex_cpt); // On attend que la condition soit remplie 
            pthread_cond_signal (&condition_cpt); /* On délivre le signal : condition remplie */
            pthread_mutex_unlock (&mutex_cpt); // On déverrouille le mutex 
        }
    }
    pthread_exit(NULL); // Fin du thread
}

void * mean_calculation(void * arg){
    
    int time_fault;
    FILE * lecture = NULL;
    printf("lancement Primary\n");
    srand(time(NULL));
    char tableau[100] = "";
    char tableau2[100] = "";
    getcwd(tableau, 100);
    getcwd(tableau2, 100);
    strcat(tableau, "/data_sensor");
    
    float * mean = (float*)malloc(sizeof(float));
    char  chaine[20] = "";
    fpos_t *tmp= (fpos_t*)malloc(sizeof(fpos_t));
    int end = 0;
    FILE * ecriture = NULL;
    strcat(tableau2, "/exec/share_file.txt");
    ecriture = fopen(tableau2, "w+");
    fclose(ecriture);
    do{
        pthread_mutex_lock (&mutex_SF); /* On verrouille le mutex */
        lecture = fopen(tableau, "r");
        ecriture = fopen(tableau2, "a+");
        fsetpos(lecture, reading_cursor);
        fgetpos(ecriture, tmp);
        *mean = 0;
        for(int i = 0; i<nb_iter; i++){
            if(NULL==fgets(chaine, 20, lecture)){
                end = 1;
            }
            else{
            //Simulation faute en valeurs
            if(rand()%3000==1)
                *mean-=150*nb_iter;  
            else
                *mean+=atoi(chaine);  
            }
        }
       if(end!=1){
        *mean/=nb_iter;
        if(*mean<400){
            printf("mean value : %f\n", *mean);
            printf("Valeurs étranges : vérification par Back up...\n");
            fclose(ecriture);
            fclose(lecture);
            pthread_cond_signal (&condition_SF); /* On délivre le signal : condition remplie */
            pthread_cond_signal (&condition_cpt); /* On délivre le signal : condition remplie */
            //pthread_mutex_unlock (&mutex_SF); 
            pthread_cond_wait (&condition_SF, &mutex_SF); // On attend que la condition soit remplie 
            //pthread_mutex_lock (&mutex_SF); /* On verrouille le mutex */
            ecriture = fopen(tableau2, "r");
            
            fsetpos(ecriture, tmp);
            while(fgets(chaine, 20, ecriture)!= NULL);
            printf("mean : %f, chaine : %f\n", *mean, atof(chaine));
            end = (*mean!=atof(chaine));
            if(end){
                
                fclose(ecriture);
                printf("Primary défaillant\n");
                pthread_mutex_unlock (&mutex_SF); /* On déverrouille le mutex */
                pthread_exit(NULL);
            }
        }
        else {
            fprintf(ecriture, "%f\n", *mean);
            fgetpos(lecture, reading_cursor);
            fclose(ecriture);
            fclose(lecture);
            pthread_mutex_unlock (&mutex_SF); /* On déverrouille le mutex */

            printf("mean value : %f\n", *mean);

            pthread_mutex_lock (&mutex_cpt); /* On verrouille le mutex */
            pthread_cond_signal (&condition_cpt); /* On délivre le signal : condition remplie */
            pthread_mutex_unlock (&mutex_cpt); /* On déverrouille le mutex */
            }
       }
        else {
                fclose(ecriture);
                fclose(lecture);
                pthread_mutex_unlock (&mutex_SF);
            }
        //Simulation faute temporelle
        time_fault = rand()%100;
        if(time_fault == 1)
           sleep(10);
        else sleep(1);
    }while(end!=1);
    printf("Primary ended\n");
    pthread_exit(NULL); // Fin du thread
}

void * mean_calculation_BackUp(void * arg){
    struct timespec ts;
    int rt;
    FILE * lecture = NULL;
    srand(time(NULL));
    char tableau[100] = "";
    char tableau2[100] = "";
    getcwd(tableau, 100);
    getcwd(tableau2, 100);
    strcat(tableau, "/data_sensor");
    float * mean = (float*)malloc(sizeof(float));
    char  chaine[6] = "";
    int end = 0;
    FILE * ecriture = NULL;
    strcat(tableau2, "/exec/share_file.txt");
    do{
        pthread_mutex_lock (&mutex_SF); // On verrouille le mutex 
        pthread_cond_wait (&condition_SF, &mutex_SF); // On attend que la condition soit remplie     
        printf("Lancement Module de BackUp\n");
        do{
            lecture = fopen(tableau, "r");
            ecriture = fopen(tableau2, "a");
            fsetpos(lecture, reading_cursor);
            *mean = 0;
            for(int i = 0; i<nb_iter; i++){
                if(NULL==fgets(chaine, 6, lecture)){
                    end = 1;
                }
                else{
                //printf("chaine : %s ", chaine);
                *mean+=atoi(chaine);  
                }
            }
            if(end!=1){
                *mean/=nb_iter;
                fprintf(ecriture, "%f\n", *mean);
                fgetpos(lecture, reading_cursor);
                pthread_cond_signal (&condition_SF); /* On délivre le signal : condition remplie */
                printf("BU mean value : %f\n", *mean);
           }
            fclose(ecriture);
            fclose(lecture);
            pthread_mutex_unlock (&mutex_SF);
            
            ts = timer_creation(2);
            pthread_mutex_lock (&mutex_cpt); // On verrouille le mutex 
            rt = pthread_cond_timedwait (&condition_cpt, &mutex_cpt, &ts); // On attend que la condition soit remplie            }while(end!=1);
            pthread_mutex_unlock (&mutex_cpt); // On déverrouille le mutex 
         }while(rt == ETIMEDOUT && end!=1);
    }while(end!=1);
    printf("Backup ended\n");
    pthread_exit(NULL); // Fin du thread
}

struct timespec timer_creation(int number){
    struct timeval tv;
    gettimeofday(&tv, NULL);
    struct timespec ts;
    ts.tv_sec = time(NULL) + number*1000 / 1000;
    ts.tv_nsec = tv.tv_usec * 1000 + 1000 * 1000 * (number*1000 % 1000);
    ts.tv_sec += ts.tv_nsec / (1000 * 1000 * 1000);
    ts.tv_nsec %= (1000 * 1000 * 1000);
    return ts;
}