#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<unistd.h>

// Calculate the mean of the collected data from sensor, then write them in the 
//shared file
int main(){
    FILE * lecture = NULL;
    lecture = fopen("data_sensor", "r");
    char  chaine[6] = "";
    if (lecture != NULL){
        int mean, end = 0;
        FILE * ecriture = NULL;
        ecriture = fopen("share_file.txt", "w+");
        do{
           mean = 0;
            for(int i = 0; i<10; i++){
                if(NULL==fgets(chaine, 6, lecture)){
                    end = 1;
                }
                else{
                //printf("chaine : %s ", chaine);
                mean+=atoi(chaine);  
                }
            }
           if(end!=1){
            mean/=10;
            fprintf(ecriture, "%d\n", mean);
           }
        }while(end!=1); 
    }
    else {
        printf("Impossible d'ouvrir le fichier data_sensor");
    }
    fclose(lecture);
    return 0;
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
