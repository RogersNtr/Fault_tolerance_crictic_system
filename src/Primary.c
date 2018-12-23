#include "../header/Primary.h"
// Calculate the mean of the collected data from sensor, then write them in the 
//shared file

void * mean_calculation(int nb_iter){
    FILE * lecture = NULL;
    char tableau[100] = "";
    char tableau2[100] = "";
    getcwd(tableau, 100);
    getcwd(tableau2, 100);
    printf("fct getcwd : %s\n", tableau );
    strcat(tableau, "/data_sensor");
    printf("tab2 : %s\n",tableau2);
    lecture = fopen(tableau, "r");
    float * mean = (float*)malloc(sizeof(float));
    char  chaine[6] = "";
    if (lecture != NULL){
        int end = 0;
        FILE * ecriture = NULL;
        strcat(tableau2, "/exec/share_file.txt");
        printf("tab2 : %s\n",tableau2);
        ecriture = fopen(tableau2, "w+");
        do{
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
            printf("mean value : %f\n", *mean);
           }
           sleep(1);
        }while(end!=1);
        fclose(ecriture);
    }
    else {
        printf("Impossible d'ouvrir le fichier data_sensor");
    }
    fclose(lecture);
    return mean;
}
   /*  while(1){
        printf("thread fonction");
        sleep(10);
    }  */
/*     int* nb_iteration = (int*) malloc(sizeof(int));
    * nb_iteration = rand()%6;
    for(int i=0;i<*nb_iteration;i++){
        printf("thread fonction %lu\n", (long)pthread_self());
        sleep(1);
  }
  return nb_iteration; */
