#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/select.h>
#include "server.h"

#define PORT 2728

int errno;

int main()
{
  int sd, numberOfQuestions = 4, counter = 0;
  int client;
  int option = 1;
  char buffer[1024];
  int ids[16] = {0};
  int numberOfClients = 0;
  int questionsArray[15] = {0};
  int idQuestion;
  int eta = 5000; /* 10s */
  int currentClient;

  clock_t before = clock();
  int msec = 0;

  struct sockaddr_in serverAddr; // server
  struct sockaddr_in newAddr;    // client
  fd_set actfds;                 // descriptori activi
  fd_set readfds;
  int fd; // desriptor folosit pt parcurgerea listelor de descriptori
  int clientsFD[20] = {0};
  int copyNumberOfClients = 0;

  key_t shmkey;                                 /*      shared memory key       */
  int shmid;                                    /*      shared memory id        */
  sem_t sem; /*      synch semaphore         */ /*shared */
  int *p; /*      shared variable         */    /*shared */
  int n;                                        /*      fork count              */
  int value;

  socklen_t addr_size;
  pid_t pid;
  pid_t timer;

  sd = socket(AF_INET, SOCK_STREAM, 0);
  if (sd < 0)
  {
    perror("Eroare la socket\n");
    return errno;
  }
  setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(PORT);
  serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

  if ((bind(sd, (struct sockaddr *)&serverAddr, sizeof(serverAddr))) < 0)
  {
    perror("Eroare la bind\n");
    return errno;
  }
  int lis = listen(sd, 10);
  if (lis == 0)
  {
    printf("Asteptare clienti....\n");
  }
  else if (lis < 0)
  {
    perror("Eroare la lilsten\n");
    return errno;
  }

  for (int i = 0; i < 3; ++i)
  {
    idQuestion = pickQuestion(ids, 29);
    questionsArray[i] = idQuestion;
    printf("%d, ", questionsArray[i]);
  }
  printf("\n");
  idQuestion = 0;

  fd_set set;
  struct timeval timeout;
  int rv;
  FD_ZERO(&set);
  FD_SET(sd, &set);

  FD_ZERO(&actfds);
  FD_SET(sd, &actfds);

  timeout.tv_sec = 5;
  timeout.tv_usec = 0;
  do
  {
    bcopy((char *)&actfds, (char *)&readfds, sizeof(readfds));

    int length = sizeof(newAddr);
    rv = select(sd + 1, &set, NULL, NULL, &timeout);

    if (rv == -1)
    {
      perror("Eroare la select\n");
      return errno;
    }
    else if (rv == 0)
    {
      printf("Timeout: %ld sec\n", timeout.tv_sec);
      break;
    }
    else
    {
      if (FD_ISSET(sd, &readfds))
      {
        client = accept(sd, (struct sockaddr *)&newAddr, &length);
        if (client < 0)
        {
          perror("Eroare la accept\n");
          return errno;
        }
        clientsFD[numberOfClients] = client;
        ++numberOfClients;
        printf("\nClient conectat de la adresa <%s> is port <%d> (numar clienti [%d])\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port), numberOfClients);
        FD_SET(client, &actfds);
      }
    }

    clock_t difference = clock() - before;
    msec = difference * 1000 / CLOCKS_PER_SEC;
  } while (msec < eta);

  printf("TOTAL JUCATORI: %d\n", numberOfClients);
  copyNumberOfClients = numberOfClients;
  for (int j = 0; j < copyNumberOfClients; ++j)
    printf("Client: %d\n", clientsFD[j]);

  shmkey = ftok("/dev/null", 5);
  printf("shmkey for p = %d\n", shmkey);
  shmid = shmget(shmkey, sizeof(int), 0644 | IPC_CREAT);
  if (shmid < 0)
  { /* shared memory error check */
    perror("shmget\n");
    exit(1);
  }

  // initializare semaphore
  p = (int *)shmat(shmid, NULL, 0); /* attach p to shared memory */
  *p = numberOfClients;
  int k = 1;

  // sem = sem_open("pSem", O_CREAT | O_EXCL, 0644, value);
  sem_init(&sem, 0, numberOfClients);

  for (int j = 0; j < numberOfClients; ++j)
  {
    if ((pid = fork()) == -1)
    {
      close(client);
      sem_unlink("pSem");
      sem_close(&sem);
      perror("Eroare la fork\n");
      return errno;
    }
    else if (pid == 0)
      break;
  }
  while (1)
  {
    if (pid != 0)
    {
      // wait for all the children to exit
      printf("PARINTE\n");
      while (pid = waitpid(-1, NULL, 0))
      {
        if (errno == ECHILD)
          break;
      }
      printf("All children have exited\n");
      // shared memory detach
      shmdt(p);
      shmctl(shmid, IPC_RMID, 0);
      sem_unlink("pSem");
      sem_close(&sem);
      // printf("aici\n");
      exit(0);
    }
    else
    {

      printf("COPIL\n");
      int semWait = sem_wait(&sem);
      if (semWait < 0)
      {
        perror("Eroare la sem_wait\n");
        return errno;
      }
      printf("Child id in critical section.\n");

      *p -= k; // incrementa p cu 1
      currentClient = clientsFD[*p];
      printf("Child new value asigned: %d, %d\n", *p, currentClient);

      // sleep(1);

      // for (int j = 0; j < copyNumberOfClients; ++j)
      //   printf("Client: %d\n", clientsFD[j]);
      printf("Clientul curent este: %d\n", currentClient);
      int score = 0;
      // verificare credentiale
      while (1)
      {
        printf("Logare\n");
        if (signin(currentClient))
        {
          printf("Login successful\n");
          break;
        }
      }
      // comunicare cu clientul -----> comunicare cu clientii
      while (1)
      {
        char answer[100] = "";
        char question[200] = "";
        msec = 0;

        if (counter < numberOfQuestions)
        {
          getQuestion(questionsArray[idQuestion], question);
          counter++;
        }
        if (counter == numberOfQuestions)
        {
          printf("INTREBARI TERMINATE \tSCOR FINAL: %d\n", score);
          writeScore(currentClient, score);
          close(currentClient);
          close(sd);
          return 0;
        }

        printf("%d) Intrebare: %s\t%d\n", questionsArray[idQuestion], question, score);
        writeQuestion(currentClient, question);
        readAnswer(currentClient, answer);

        int isCorrect = addPoints(questionsArray[idQuestion], currentClient, answer, inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port), score);
        if (isCorrect == -1)
        {
          close(currentClient);
          break; // optiunea quit
        }
        else if (isCorrect == 1)
          score += 100;
        ++idQuestion;

        // do
        // {
        //   clock_t difference = clock() - before;
        //   msec = difference * 1000 / CLOCKS_PER_SEC;
        // } while (msec < eta);

        sem_post(&sem);
      }
      exit(0);
    }
  }

  return 0;
}