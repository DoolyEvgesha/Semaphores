#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/sem.h>


int main(int argc, char* argv[]) {
    pid_t child_pid = 0;
    if (argc >= 2) {
        char *endptr1 = 0;
        char *endptr2 = 0;

        int num = strtol(argv[1], &endptr1, 10);
        int sum = strtol(argv[2], &endptr2, 10);
        
        int id = shmget(IPC_PRIVATE, sizeof(int),IPC_CREAT|0700);
        int *p =shmat(id, NULL,0);
        printf("%d\n",*p);

        int semid = semget(IPC_PRIVATE,1,IPC_CREAT|0700);
        struct sembuf sops;
        sops.sem_num=0;
        sops.sem_op=1;
        sops.sem_flg=0;

        int res = 0;
        if((res=semop(semid, &sops,1))<0)
            perror("semop init\n");

        for(int i = 0; i<num; i++){
            child_pid = fork();
            if (child_pid == 0) {
                for(int j =0; j<sum; j++){
                    sops.sem_num=0;
                    sops.sem_op=-1;
                    sops.sem_flg=0;
                    if(semop(semid, &sops,1)<0)
                        perror("semop -1\n");

                    (*p)++;

                    sops.sem_num=0;
                    sops.sem_op=1;
                    sops.sem_flg=0;
                    if(semop(semid, &sops,1)<0)
                        perror("semop +1\n");
                }
                return 0;
            }
        }
        for(int k = 0; k<num; k++)
            wait(NULL);
        printf("Result: %d,\ntheoretical = %d\n", *p, sum*num);
        semctl(semid,IPC_RMID,0);
        shmctl(id,IPC_RMID,0);
    }

    return 0;

}