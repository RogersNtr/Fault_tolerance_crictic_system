/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main_thread.h
 * Author: michael
 *
 * Created on December 23, 2018, 5:53 PM
 */

#ifndef MAIN_THREAD_H
#define MAIN_THREAD_H

#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>


#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
//#include<ckpt.h>
/*#include "Count.h"
#include "Primary.h"
#include "Watchdog.h"*/


void * watchDog(void * arg);

struct Checkpoint {
   char  filename[50];
   long int   cursor_pointer_input;
   long int cursor_pointer_share_file;
} Checkpoint;

void * mean_calculation_BackUp(int nb_iter, struct Checkpoint* resume_point);

void * mean_calculation(int nb_iter);

void mean_backup(int nb_iter, struct Checkpoint* resume_point);
#endif /* MAIN_THREAD_H */

