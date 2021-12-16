#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include "server.h"

#define PORT 2728

int errno;

int main()
{

  int sd, numberOfQuestions = 15, counter = 0;
  int client;
  int option = 1;
  char buffer[1024];
  int ids[16] = {0};
  struct sockaddr_in serverAddr; // server
  struct sockaddr_in newAddr;    // client

  socklen_t addr_size;
  pid_t pid;

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
    int score = 0;
    client = accept(sd, (struct sockaddr *)&newAddr, &addr_size);
    if (client < 0)
    {
      perror("Eroare la accept\n");
      return errno;
    }
    printf("\nClient conectat de la adresa <%s> is port <%d>\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));

    if ((pid = fork()) == -1)
    {
      close(client);
      // continue;
      perror("Eroare la fork\n");
      return errno;
    }
    if (pid == 0)
    {
      // verificare credentiale
      while (1)
      {
        if (signin(client))
          break;
      }
      // comunicare cu clientul
      while (1)
      {
        char question[200] = "", answer[100] = "";
        int idQuestion = pickQuestion(ids, numberOfQuestions);
        if (counter < numberOfQuestions)
        {
          getQuestion(idQuestion, question);
          counter++;
        }
        if(counter == numberOfQuestions)
        {
          printf("INTREBARI TERMINATE \tSCOR FINAL: %d\n", score);
          writeScore(client, score);
          close(client);
          close(sd);
          return 0;
        }
        printf("Intrebare: %s\t%d\n", question, score);
        writeQuestion(client, question);
        readAnswer(client, answer);

        int isCorrect = addPoints(idQuestion, client, answer, inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port), score);
        if (isCorrect == -1)
        {
          close(client);
          break; // optiunea quit
        }
        else if (isCorrect == 1)
          score += 100;
      }
    }
  }

  return 0;
}