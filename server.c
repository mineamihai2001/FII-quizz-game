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
  int eta = 10000;
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

  key_t shmkey2;
  int shmid2;
  sem_t sem2;
  int *q;

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

  while (1)
  {
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

    timeout.tv_sec = 10;
    timeout.tv_usec = 0;
    do
    {

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

        client = accept(sd, (struct sockaddr *)&newAddr, &length);
        if (client < 0)
        {
          perror("Eroare la accept\n");
          return errno;
        }
        clientsFD[numberOfClients] = client;
        ++numberOfClients;
        printf("\nClient conectat de la adresa <%s> is port <%d> (numar clienti [%d])\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port), numberOfClients);
      }

      clock_t difference = clock() - before;
      msec = difference * 1000 / CLOCKS_PER_SEC;
    } while (msec < eta);

    printf("TOTAL JUCATORI: %d\n\n", numberOfClients);
    copyNumberOfClients = numberOfClients;

    shmkey = ftok("/dev/null", 5);
    shmid = shmget(shmkey, sizeof(int), 0644 | IPC_CREAT);

    if (shmid < 0)
    {
      perror("shmget\n");
      return errno;
    }

    // initializare semaphore
    p = (int *)shmat(shmid, NULL, 0);
    *p = numberOfClients;
    int k = 1;

    // semaphore 2
    shmkey2 = ftok("/dev/null", 6);
    shmid2 = shmget(shmkey2, sizeof(int), 0644 | IPC_CREAT);
    if (shmid2 < 0)
    {
      perror("shmget2\n");
      return errno;
    }
    q = (int *)shmat(shmid2, NULL, 0);
    *q = 0;
    sem_init(&sem2, 0, numberOfClients);

    sem_init(&sem, 0, numberOfClients);

    for (int j = 0; j < numberOfClients; ++j)
    {
      if ((pid = fork()) == -1)
      {
        close(client);
        sem_unlink("pSem");
        sem_close(&sem);
        sem_close(&sem2);
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
        while (pid = waitpid(-1, NULL, 0))
        {
          if (errno == ECHILD)
            break;
        }
        printf("All children have exited\n");
        // shared memory detach
        shmdt(p);
        shmdt(q);
        shmctl(shmid, IPC_RMID, 0);
        shmctl(shmid2, IPC_RMID, 0);
        sem_unlink("pSem");
        sem_close(&sem);
        sem_close(&sem2);
        break;
        // printf("aici\n");
        // exit(0);
      }
      else
      {
        int semWait = sem_wait(&sem);
        if (semWait < 0)
        {
          perror("Eroare la sem_wait\n");
          return errno;
        }

        printf("Child id in critical section.\n");

        *p -= k; // decrementeaza p cu 1
        currentClient = clientsFD[*p];
        printf("Child new value asigned: %d, %d\n", *p, currentClient);
        sem_post(&sem);

        printf("Clientul curent este: %d\n", currentClient);

        int score = 0;
        int thisLogged = 0;
        int loggedClients = 0;

        // verificare credentiale

        while (1)
        {
          printf("Logare\n");
          if (signin(currentClient))
          {
            printf("Login successful\n");
            int semWait2 = sem_wait(&sem2); // lock the semaphore
            if (semWait2 < 0)
            {
              perror("Eroare la sem_wait2\n");
              return errno;
            }
            *q += 1;
            sem_post(&sem2);
            break;
          }
        }

        printf("Clienti logati: %d, total clienti: %d, id client: %d\n", loggedClients, numberOfClients, currentClient);

        while (1)
        {
          int semWait2 = sem_wait(&sem2);
          if (semWait2 < 0)
          {
            perror("Eroare la sem_wait2\n");
            return errno;
          }
          loggedClients = *q;
          sem_post(&sem2);
          if (loggedClients == numberOfClients)
            break;
        }

        // comunicare cu clientul -----> comunicare cu clientii
        while (1)
        {
          char answer[100] = "";
          char question[200] = "";
          int clientsAnswered = 0;
          int timeForResponse = 0;

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
          timeForResponse = readAnswer(currentClient, answer);

          int isCorrect = addPoints(questionsArray[idQuestion], currentClient, answer, inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port), score);
          if (isCorrect == -1)
          {
            close(currentClient);
            break; // optiunea quit
          }
          else if (isCorrect == 1)
            score += timeForResponse * 100;
          ++idQuestion;
        }
        break;
      }
      break;
    }
    // break;
  }
  return 0;
}