#include  <stdio.h>
#include  <stdlib.h>
#include  <sys/types.h>
#include  <sys/ipc.h>
#include  <sys/shm.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

int  main()
{
     int    ShmID;
     int    *ShmPTR;
     pid_t  pid;
     int    status;

     //create shared memory for BankAccount and Turn
     ShmID = shmget(IPC_PRIVATE, 2*sizeof(int), IPC_CREAT | 0666);
     if (ShmID < 0) {
          printf("*** shmget error ***\n");
          exit(1);
     }
     printf("Process has received a shared memory of two integers...\n");
     //attach shared memory
     ShmPTR = (int *) shmat(ShmID, NULL, 0);
     if (ShmPTR ==(void *)-1) {
          printf("*** shmat error ***\n");
          exit(1);
     }
     printf("Process has attached the shared memory...\n");
     //SharedMem[0] = BankAccount
     //SharedMem[1] = Turn
     ShmPTR[0] = 0;
     ShmPTR[1] = 0;

     printf("Orig Bank Account = %d\n", ShmPTR[0]);
     //Create child process
     pid = fork();

     if (pid < 0) {
          printf("*** fork error ***\n");
          exit(1);
     }
     //Poor student - child process
     else if (pid == 0) {
          int account;
          int balance;
          int i;

          srand(getpid());
          for (i=0; i < 25; i++){
            //sleep for a random amount of time from 0-5 seconds
            sleep(rand() % 6);
            //wait for poor student's turn
            while (ShmPTR[1] != 1);
            //read BankAccount from shared memory
            account = ShmPTR[0];
            //student needs a random amount from $0-$50
            balance = rand() % 51;
            printf("Poor Student needs $%d\n", balance);
            if (balance <= account){
              account = account - balance;
              printf("Poor Student: Withdraws $%d / Balance = $%d\n", balance, account);
            }
            else{
              printf("Poor Student: Not Enough Cash ($%d)\n", account);
            }
            //update shared BankAccount
            ShmPTR[0] = account;
            //Give turn back to Dear Old Dad
            ShmPTR[1] = 0;
          }
          //child detaches from shared memory
          shmdt((void *)ShmPTR);
          exit(0);
     }
     //Dear Old Dad - Parent Process
     else{
      int account;
      int balance;
      int i;
      srand(getpid());
      for (i=0; i < 25; i++){
        //sleep for a random amount of time from 0-5 seconds
        sleep(rand() % 6);
        //Wait for Dear Old Dad's turn 
        while (ShmPTR[1] != 0);
        //read BankAccount from shared memory
        account = ShmPTR[0];
        if (account <= 100){
          //dad generates a random amount from $0-$100
          balance = rand() % 101;
          //deposits only if amount is even
          if (balance % 2 == 0){
            account = account + balance;
            printf("Dear old Dad: Deposits $%d / Balance = $%d\n", balance, account);
          }
          else{
            printf("Dear old Dad: Doesn't have any money to give\n");
          }
        }
        else{
          printf("Dear old Dad: Thinks Student has enough Cash ($%d)\n", account);
        }
        //update shared BankAccount
        ShmPTR[0] = account;
        //Give turn to Poor Student
        ShmPTR[1] = 1;
      }
      //wait for poor student to finish
      wait(&status);
      printf("Process has detected the completion of its child...\n");
      //Detach shared memory
      shmdt((void *) ShmPTR);
      printf("Process has detached its shared memory...\n");
      //remove shared memory
      shmctl(ShmID, IPC_RMID, NULL);
      printf("Process has removed its shared memory...\n");
      printf("Process exits...\n");
  }
  return 0;
}