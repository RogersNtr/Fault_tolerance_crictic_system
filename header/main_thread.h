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
/*#include "Count.h"
#include "Primary.h"
#include "Watchdog.h"*/


void * mean_calculation(int nb_iter);
void * watchDog(void * arg);

#endif /* MAIN_THREAD_H */

