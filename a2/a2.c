#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "a2_helper.h"
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>

sem_t s1, s2, s3;
pthread_mutex_t m1 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t c1 = PTHREAD_COND_INITIALIZER;
pthread_barrier_t b1;
int simultaneous_threads = 0;
pthread_t threadsss[42];

void *threadFunction(void *arg)
{
    int id = *(int *)arg;
    // sem_t* s1 = sem_open("s1", O_CREAT, O_RDWR, 0);
    // sem_t* s2 = sem_open("s2", O_CREAT, O_RDWR, 0);

    if (id == 4)
    {                      
        info(BEGIN, 2, 4); 
        sem_post(&s1);     
        sem_wait(&s2);     
        info(END, 2, 4);   
    }
    else if (id == 3)
    {
        sem_wait(&s1);      
        info(BEGIN, 2, id); 
        info(END, 2, id);   
        sem_post(&s2);      
    }
    else
    {
        info(BEGIN, 2, id); 
        info(END, 2, id);   
    }

    // info(BEGIN, 2, *id);
    /*if (id == 4){
        sem_post(s1);
    }
    */

    // info(END, 2, *id);

    pthread_exit(0);

    // return NULL;
}

void *threads42(void *arg)
{
    int id = *(int *)arg;
    /*
    pthread_barrier_wait(&b1); // synchronize threads at the beginning
    sem_wait(&s3); // limit the number of running threads

    pthread_mutex_lock(&m1);
    simultaneous_threads++; // increase the number of running threads
    if (id == 14 && simultaneous_threads == 6) {
        pthread_cond_wait(&c1, &m1); // wait for the other threads
    }
    pthread_mutex_unlock(&m1);
    */

    sem_wait(&s3);

    info(BEGIN, 4, id);
    info(END, 4, id);
    
    sem_post(&s3);

    // sem_post(&s3); // release the semaphore
    // pthread_join(threadsss[id], NULL); // join the thread

    pthread_exit(0);
}

void *threadFunctionP3(void *arg)
{
    int id = *(int *)arg;



    info(BEGIN, 3, id); 
    info(END, 3, id);   

    pthread_exit(0);

    return NULL;
}





int main()
{
    init(); // tester init //only one time in the main

    info(BEGIN, 1, 0); // tester begin //P1
    // info(BEGIN, process_no, thread_no)

    pid_t p2, p3, p4, p5, p6, p7, p8;

    p2 = fork();
    if (p2 == 0)
    {
        info(BEGIN, 2, 0); // P2
        p8 = fork();       // P8 is child of P2
        if (p8 == 0)
        {
            info(BEGIN, 8, 0); // P8
            info(END, 8, 0);   // P8 end
            exit(0);
        }
        else
        {

            pthread_t threads[5];
            int ids[5] = {1, 2, 3, 4, 5};
            sem_init(&s1, 0, 0);
            sem_init(&s2, 0, 0);
            for (int i = 0; i < 5; i++)
            {
                pthread_create(&threads[i], NULL, threadFunction, &ids[i]);
            }
            for (int i = 0; i < 5; i++)
            {

                pthread_join(threads[i], NULL);
            }
            sem_destroy(&s1);
            sem_destroy(&s2);

            waitpid(p8, NULL, 0); // wait for P8
            info(END, 2, 0);      // P2 end
            exit(0);
        }
    }
    else
    {
        p3 = fork();
        if (p3 == 0)
        {
            info(BEGIN, 3, 0); // P3

            pthread_t threadsP3[5];
            int id3[5] = {1, 2, 3, 4, 5};
            //sem_init(&s1, 0, 0);
            //sem_init(&s2, 0, 0);
            for (int i = 0; i < 5; i++)
            {
                pthread_create(&threadsP3[i], NULL, threadFunctionP3, &id3[i]);
            }
            for (int i = 0; i < 5; i++)
            {
                pthread_join(threadsP3[i], NULL);
            }

            p5 = fork();       // P5 is child of P3
            if (p5 == 0)
            {
                info(BEGIN, 5, 0); // P5
                info(END, 5, 0);
                exit(0);
            }
            else
            {
                p7 = fork(); // P7 is child of P3
                if (p7 == 0)
                {
                    info(BEGIN, 7, 0); // P7
                    info(END, 7, 0);
                    exit(0);
                }
                else
                {
                    waitpid(p5, NULL, 0); // wait for P5
                    waitpid(p7, NULL, 0); // wait for P7
                    info(END, 3, 0);
                    exit(0);
                }
            }
        }
        p4 = fork();
        if (p4 == 0)
        {
            info(BEGIN, 4, 0); // P4

            pthread_t threads_42[42];
            int ids_42[42] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
                              11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
                              21, 22, 23, 24, 25, 26, 27, 28, 29, 30,
                              31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
                              41, 42};
            sem_init(&s3, 0, 6);
            // pthread_barrier_init(&b1, NULL, 42);

            for (int i = 0; i < 42; i++)
            {
                pthread_create(&threads_42[i], NULL, threads42, &ids_42[i]);
            }
            // pthread_mutex_lock(&m1);
            // pthread_mutex_unlock(&m1);
            for (int i = 0; i < 42; i++)
            {

                pthread_join(threads_42[i], NULL);
            }

            sem_destroy(&s3);
            // pthread_barrier_destroy(&b1);
            // pthread_mutex_destroy(&m1);
            // pthread_cond_destroy(&c1);

            p6 = fork(); // P6 is child of P4
            if (p6 == 0)
            {
                info(BEGIN, 6, 0); // P6
                info(END, 6, 0);
                exit(0);
            }
            else
            {
                waitpid(p6, NULL, 0);
                info(END, 4, 0);
                exit(0);
            }
        }
        else
        {
            waitpid(p2, NULL, 0); // wait for P2
            waitpid(p3, NULL, 0); // wait for P3
            waitpid(p4, NULL, 0); // wait for P4
            info(END, 1, 0);      // P1 end
            exit(0);
        }
    }

    return 0;
}