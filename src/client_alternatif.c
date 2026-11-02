#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "../includes/segdef.h"

//Code de mon collègue - Son code de tests


#define NB_SEMAPHORES 3
#define NB_REQUESTS 10

static void exit_with_error(const char *context)
{
    perror(context);
    exit(EXIT_FAILURE);
}

static segment *init_client(int *semid)
{
    int shmid = shmget(cle, segsize, 0);
    if (shmid < 0)
        exit_with_error("shmget");

    *semid = semget(cle, NB_SEMAPHORES, 0);
    if (*semid < 0)
        exit_with_error("semget");

    segment *seg = shmat(shmid, NULL, 0);
    if (seg == (void *)-1)
        exit_with_error("shmat");

    init_rand();
    return seg;
}

static long fill_segment(segment *seg, int req)
{
    long sum = 0;

    seg->pid = getpid();
    seg->req = req;
    for (int i = 0; i < maxval; i++)
    {
        seg->tab[i] = getrand();
        sum += seg->tab[i];
    }
    return sum / maxval;
}

int main(void)
{
    int semid;
    segment *seg = init_client(&semid);

    for (int req = 1; req <= NB_REQUESTS; req++)
    {
        acq_sem(semid, seg_dispo);
        long client_average = fill_segment(seg, req);
        acq_sem(semid, seg_init);

        wait_sem(semid, res_ok);
        long server_average = seg->result;
        int result_pid = seg->pid;
        int result_req = seg->req;
        lib_sem(semid, seg_init);

        acq_sem(semid, res_ok);
        lib_sem(semid, res_ok);
        lib_sem(semid, seg_dispo);

        int is_valid = result_pid == getpid() && result_req == req && server_average == client_average;
        printf("Request %d from pid %d: client %ld, server %ld, %s\n", result_req, result_pid, client_average,
               server_average, is_valid ? "OK" : "ERROR");
    }

    if (shmdt(seg) < 0)
        exit_with_error("shmdt");

    return EXIT_SUCCESS;
}
