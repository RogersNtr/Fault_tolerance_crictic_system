#include <signal.h>
#include "../header/main_thread.h"


pthread_cond_t condition_cpt = PTHREAD_COND_INITIALIZER; /* Création de la condition sur Cpt */
pthread_mutex_t mutex_cpt = PTHREAD_MUTEX_INITIALIZER; /* Création du mutex lié à Cpt*/


pthread_cond_t condition_SF = PTHREAD_COND_INITIALIZER; /* Création de la condition sur le fichier partagé*/
pthread_mutex_t mutex_SF = PTHREAD_MUTEX_INITIALIZER; /* Création du mutex sur fichier partagé*/

fpos_t *reading_cursor;/*curseur pour se positioner dans les fichiers*/
int sliding_window =10;/* fenêtre glissante pour le calcul de la moyenne (nombre de valeur)*/
float current_mean = 0; //minimum avant crash
int count_affichage = 0; //Compte le nombre de passages dans le primaire et le backup.

/*en-têtes des fontion appelées*/
void * watchDog(void * arg);
void * mean_calculation(void * arg);
void * mean_calculation_BackUp(void * arg);
void set_mean(float mean_val);
struct timespec timer_creation(int number);

/**
 * Thread principal (main())
 */
int main (){
    /*Création Threads*/
    pthread_t Primary;
    pthread_t Backup;
    pthread_t Watchdog;

    /*retours sur créations*/
    int s, s1, s2;

    /*initialisation curseur de positionnement*/
    reading_cursor = (fpos_t*)malloc(sizeof(fpos_t));
    
    /*initialisation des mutex*/
    pthread_mutex_init(&mutex_cpt,NULL);
    pthread_mutex_init(&mutex_SF,NULL);
    pthread_cond_init(&condition_SF,NULL);
    pthread_cond_init(& condition_cpt,NULL);

    /*Lancement des Threads associés à leur fonction*/
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
    
    /*mise en attente du main pour laisser évoluer les threads*/
    pthread_join(Primary, NULL);
    pthread_join(Backup, NULL);
    return 0;
}

/**
 * Fonction associée au Thread WatchDog, cette fonction se met en attente de
 * réponse des threads Primary ou BackUp. Si pas de réponse en 4 sec, le 
 * watchdog envoit un signal au Backup pour le reveiller.
 * @param arg 
 * @return 
 */
void * watchDog(void * arg){
    /*création structure temporelle*/
    struct timespec ts;
    
    /*création retour temporisation*/
    int rt;
    
    while(1){
        ts = timer_creation(4);
        pthread_mutex_lock (&mutex_cpt); // On verrouille le mutex 
        rt = pthread_cond_timedwait (&condition_cpt, &mutex_cpt, &ts); // On attend que la condition soit remplie 
        pthread_mutex_unlock (&mutex_cpt); // On déverrouille le mutex 
        /*Si pas de réponse au bout de ts sec*/
        if (rt == ETIMEDOUT){
            //lancer backup
            printf("Time out, Launching Back up\n\n");
            pthread_mutex_lock (&mutex_SF); // On verrouille le mutex 
            pthread_cond_signal (&condition_SF); /* On délivre le signal : condition remplie, lancement backup */
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
/**
 * Fonction associée au Thread Primary. Cette fonction calcule la moyenne sur la
 * fenêtre glissante. Lorsque cette fonction admet une faute temporelle ou en 
 * valeur, la fonction du Backup s'enclenche automatiquement.
 * @param arg
 * @return 
 */
void * mean_calculation(void * arg){
    /*création simulation faute temporelle*/
    printf("\n====>Lancement Primary\n");
    int time_fault;
    FILE * lecture = NULL;
    float * mean = (float*)malloc(sizeof(float));
    fpos_t *tmp= (fpos_t*)malloc(sizeof(fpos_t));
    int end = 0;
    srand(time(NULL));
    /*Récupération noms fichiers partagés*/
    char tableau[100] = "";
    char tableau2[100] = "";
    getcwd(tableau, 100);
    getcwd(tableau2, 100);
    strcat(tableau, "/data_sensor");
    char  chaine[20] = "";
    FILE * ecriture = NULL;
    strcat(tableau2, "/exec/share_file.txt");
    
    /*A chaque première utilisation on réinitialise le fichier d'écriture*/
    ecriture = fopen(tableau2, "w+");
    fclose(ecriture);
    
    /*Boucle de calcul de la moyenne, jusqu'à arriver en fin de fichier*/
    do{
        pthread_mutex_lock (&mutex_SF); /* On verrouille le mutex */
        lecture = fopen(tableau, "r");
        ecriture = fopen(tableau2, "a+");
        
        /*Positionnement dans les fichiers*/
        fsetpos(lecture, reading_cursor);
        fgetpos(ecriture, tmp);
        *mean = 0;
        
        /*Récupération chaine de caractère puis conversion en int*/
        int ind=0;
        do{
            if(NULL==fgets(chaine, 20, lecture)){
                end = 1;
            }
            else{
                printf("valeur_capteur: %s", chaine); 
            //Simulation faute en valeurs
            //if(rand()%3000==1){
                //set_mean(*mean);
                //*mean-=150*sliding_window; 
            //}
            //else{       
                    //On verifie que c'est un caractere qui se trouve dans les données capteur, 
                    //si oui, on attribue une moyenne de -10----> simulation faute en valeurs
                    if(atoi(chaine) == 0){
                        set_mean(*mean);
                        *mean=-10;//150*sliding_window;                         
                        //sleep(1);
                        break;
                    }else{
                        *mean+=atoi(chaine); 
                    }                    
                //}
            }
            if(atoi(chaine) == 0){
                break;
            }else
                ind++;
        }while(ind<sliding_window);
        /*calcul moyenne si pas la fin du fichier*/
       if(end!=1){
           count_affichage++;
        *mean/=sliding_window;
        /*Faute temporelle probable détectée*/
        if(*mean <= 0){
            //printf("valeur erreur mean value : %f\n", *mean);
            printf("Possible Fault detected, launching Back up...\n\n");
            
            /*Libération mutex et activation BackUp puis attente retour BackUp*/
            fclose(ecriture);
            fgetpos(lecture, reading_cursor);
            fclose(lecture);
            pthread_cond_signal (&condition_SF); /* On délivre le signal : condition remplie */
            pthread_cond_signal (&condition_cpt); /* On délivre le signal : condition remplie */
            pthread_cond_wait (&condition_SF, &mutex_SF); // On attend que la condition soit remplie 
            
            /*Vérification Valeurs*/
            ecriture = fopen(tableau2, "r");
            fsetpos(ecriture, tmp);
            pthread_exit(NULL);
            //while(fgets(chaine, 20, ecriture)!= NULL);
            // printf("mean : %f, chaine : %s", *mean, chaine);
            // end = (abs((int)(*mean - atof(chaine))));
            // printf("end : %d\n", end);
            /* Si différence trop élevée => failure, on lance le back up et on 
             arrête le primary*/
            /* if(end>1){
                fclose(ecriture);
                printf("Primary Failure, ending Primary and switching to Back up System\n");
                pthread_mutex_unlock (&mutex_SF); /* On déverrouille le mutex 
                pthread_exit(NULL);
            } */
            /*sinon on continue avec le primaire*/
            if(end<=1){
                fgetpos(lecture, reading_cursor);
                fclose(ecriture);
                pthread_mutex_unlock (&mutex_SF); /* On déverrouille le mutex */
                pthread_mutex_lock (&mutex_cpt); /* On verrouille le mutex */
                pthread_cond_signal (&condition_cpt); /* On délivre le signal : condition remplie */
                pthread_mutex_unlock (&mutex_cpt); /* On déverrouille le mutex */
            }
        }
        /*Traitement classique dans le cas d'une moyenne sans injection de faute*/
        else {
            /*On écrit la valeur calculée et on ferme tous les fichiers partagés*/
            fprintf(ecriture, "%f\n", *mean);
            fgetpos(lecture, reading_cursor);
            fclose(ecriture);
            fclose(lecture);
            
            pthread_mutex_unlock (&mutex_SF); /* On déverrouille le mutex */
            printf("+++++++mean value : %f,\t", *mean);
            printf("count affichage primaire : %d\n\n", count_affichage);
            

            /*Onc réveille le Watchdog, avec i am alive*/
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
    /*Récupération noms fichiers partagés*/
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
        /*On attend que la condition du watchdog/ primary en possible 
         *défaillance s'ative*/
        pthread_mutex_lock (&mutex_SF); // On verrouille le mutex 
        pthread_cond_wait (&condition_SF, &mutex_SF); // On attend que la condition soit remplie     
        printf("=====> Lancement Module de BackUp\n");
        
        do{
            lecture = fopen(tableau, "r");
            ecriture = fopen(tableau2, "a");
            fsetpos(lecture, reading_cursor);
            *mean = 0;
            for(int i = 0; i<sliding_window; i++){
                if(NULL==fgets(chaine, 6, lecture)){
                    end = 1;
                }
                else{
                printf("valeur_capteur_bu: %s", chaine);
                *mean+=atoi(chaine) + current_mean; 
                current_mean = 0; //on le remetr à 0 
                }
            }
            if(end!=1){
                *mean/=sliding_window;
                fprintf(ecriture, "%f\n", *mean);
                fgetpos(lecture, reading_cursor);
                pthread_cond_signal (&condition_SF); /* On délivre le signal : condition remplie */
                count_affichage++;
                printf("+++++++BU mean value : %f,\t", *mean);
                printf("count affichage backup:%d\n\n", count_affichage);
           }
            fclose(ecriture);
            fclose(lecture);
            pthread_mutex_unlock (&mutex_SF);
            
            /*On attend 2 sec avant de relancer le back up : condition 
             necessaire car il y a un sleep de 1 sec dans le primaire. Ainsi, 
             on évite les chevauchement temporels */
            ts = timer_creation(2);
            pthread_mutex_lock (&mutex_cpt); // On verrouille le mutex 
            rt = pthread_cond_timedwait (&condition_cpt, &mutex_cpt, &ts); // On attend que la condition soit remplie            }while(end!=1);
            pthread_cond_signal (&condition_cpt); /* On délivre le signal : condition remplie */
            pthread_mutex_unlock (&mutex_cpt); // On déverrouille le mutex 
         /* On recommence si les deux secondes sont passées et pas de retours*/
        }while(rt == ETIMEDOUT && end!=1);
    }while(end!=1);
    printf("Backup ended\n");
    pthread_exit(NULL); // Fin du thread
}
/**
 * fonction de création de timer pour les mutex
 * @param number : temps en seconde voulu pour la pause
 * @return la structure à ajouter dans la pause sur condition du mutex.
 */
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

void set_mean(float mean_val){
    current_mean = mean_val;
}