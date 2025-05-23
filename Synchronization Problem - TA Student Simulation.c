#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>

#define MAX_CHAIRS 5

// Synchronization variables
sem_t ta_wakeup;
sem_t student_waiting;
pthread_t mutex;

// Status variables
int waiting_students = 0;
int ta_sleep = 1;
int total_helped = 0;
int program = 1;

// TA function
void *ta_thread(void *arg) {
    while (program) {
        // TA waits for students
        sem_wait(&ta_wakeup);

        pthread_mutex_lock(&mutex);
        ta_sleep = 0;
         printf("TA woke up and is ready to help students.\n");
        pthread_mutex_unlock(&mutex);

        while (program) {
            pthread_mutex_lock(&mutex);
            if (waiting_students == 0) {
                ta_sleep = 1;
                printf("TA is sleeping because there are no students to help.\n");
                pthread_mutex_unlock(&mutex);
                break;
            }

            waiting_students--;
            printf("TA is helping a student. Waiting students: %d\n", waiting_students);
            sem_post(&student_waiting); // Allows one student to enter
            pthread_mutex_unlock(&mutex);

            // Simulating help time
            sleep(rand() % 3 + 1);

            pthread_mutex_lock(&mutex);
            total_helped++;
            printf("TA finished helping student. Total students helped so far: %d\n", total_helped);
            pthread_mutex_unlock(&mutex);
        }
    }
    return NULL;
}

// student function
void *student_thread(void *arg) {
    int student_id = *(int *)arg;

    pthread_mutex_lock(&mutex);
    printf("Student %d entered the office.\n", student_id);

    if (waiting_students < MAX_CHAIRS) {
        waiting_students++;
        printf("Student %d sat on a chair. Waiting students: %d\n", student_id, waiting_students);

        if (ta_sleep) {
            printf("Student %d woke up the TA.\n", student_id);
            sem_post(&ta_wakeup);
        }

        pthread_mutex_unlock(&mutex);
        sem_wait(&student_waiting); // Waits for TA's help

        printf("Student %d is getting help from TA.\n", student_id);
    } else {
        printf("Student %d left because no chairs were available.\n", student_id);
        pthread_mutex_unlock(&mutex);
    }

    return NULL;
}

int main() {
    srand(time(NULL));
    int num;

    printf("Please enter the number of students: ");
    scanf("%d", &num);

    if (num <= 0) {
        printf("Number of students must be greater than zero.\n");
        return 1;
    }

    // Initialize semaphores and mutex
    sem_init(&ta_wakeup, 0, 0);
    sem_init(&student_waiting, 0, 0);
    pthread_mutex_init(&mutex, NULL);


    pthread_t ta;
    pthread_create(&ta, NULL, ta_thread, NULL);


    pthread_t students[num];
    int student_ids[num];

    for (int i = 0; i < num; i++) {
        student_ids[i] = i + 1;
        pthread_create(&students[i], NULL, student_thread, &student_ids[i]);

        // Random delay between student arrivals
        sleep(rand() % 3);
    }


    for (int i = 0; i < num; i++) {
        pthread_join(students[i], NULL);
    }

    // End program
    pthread_mutex_lock(&mutex);
    program = 0;
    pthread_mutex_unlock(&mutex);

    // Wake up TA to terminate
    sem_post(&ta_wakeup);
    pthread_join(ta, NULL);


    sem_destroy(&ta_wakeup);
    sem_destroy(&student_waiting);
    pthread_mutex_destroy(&mutex);

    printf("\nSimulation ended. Helped %d out of %d students.\n",total_helped, num);

    return 0;
}
