#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>

void sensor_value(int nber_range){    
    /**
     * nber_length : the number of number to generate
     * nber_range: the function generates  numbers between[0, nber_range]
     * return : print the resulting generate value in a file.
    */
   FILE* data_file=fopen("data_sensor", "w+");
   int rand_nber=0;
   for(int i=0; i<=nber_range;i++){
       rand_nber = rand()%nber_range + 1;
       fprintf(data_file, "%d\n", rand_nber);
   }
}

//Unit Test
/*
int main(){
    int nber_range = 10000;
    sensor_value(nber_range);
}
 */