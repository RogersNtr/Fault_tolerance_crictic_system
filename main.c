#include "Count.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <math.h>

int main(int argc, char*argv[]){
    
    while(1){
        struct timespec ts;
	    ts.tv_sec = 0;
	    ts.tv_nsec = pow(5,9);
	    nanosleep(&ts, NULL);
        IncCount();
        printf("%d\n", GetCount());
        if(GetCount()>= 5){
            printf("Outch!");
            fflush(stdout);
            ClearCount();
        }
    }
}
