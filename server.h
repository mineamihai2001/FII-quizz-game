#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <stdint.h>
#include "database.h"

int errno;


int inputNumberOfPlayers(int client)
{
  char buffer[10];
  if((read(client, buffer, 10)) < 0)
  {
    perror("Eroare la primire numar clienti\n");
    return errno;
  }
  int numberOfPlayers = atoi(buffer);
  printf("Number of players: %d\n", numberOfPlayers);
  return numberOfPlayers;
}

int waitForPlayers(int eta, int sd, int client, struct sockaddr_in newAddr, socklen_t addr_size)
{

  clock_t before = clock();
  int msec = 0;
  int numberOfClients = 0;
  int k = 0;
  do
  {
    client = accept(sd, (struct sockaddr *)&newAddr, &addr_size);
    if (client < 0)
    {
      perror("Eroare la accept\n");
      return errno;
    }
    ++numberOfClients;
    printf("\nClient conectat de la adresa <%s> is port <%d> (numar clienti [%d])\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port), numberOfClients);

    clock_t difference = clock() - before;
    msec = difference * 1000 / CLOCKS_PER_SEC;
    ++k;
    printf("%d\n", k);
  } while (msec < eta);

  printf("Time taken %d seconds %d milliseconds\n", msec / 1000, msec % 1000);
  return numberOfClients;
}

int getCredentials(int client, char username[], char password[])
{
  printf("Receveing credentials...\n");
  if ((read(client, username, 100)) < 0)
  {
    perror("Eroare la primire credentiale\n");
    return errno;
  }
  else printf("Credentiale primite\n");
  // if ((read(client, password, 100)) < 0)
  // {
  //   perror("Eroare la primire credentiale\n");
  //   return errno;
  // }
}

int signin(int client)
{
  char username[100], password[100];
  getCredentials(client, username, password);
  printf("USERNAME:\t%s\nPASSWORD:\t%s\n", username, password);
  if (checkCredentials(username, password) == 0)
  {
    if ((write(client, "0", 30)) < 0)
    {
      perror("Eroare la trimitere mesaj catre client\n");
      return errno;
    }
    return 0;
  }
  else
  {
    char response[100];
    strcpy(response, "1");
    if ((write(client, response, sizeof(response))) < 0)
    {
      perror("Eroare la trimitere mesaj catre client\n");
      return errno;
    }
    return 1;
  }
}

int pickQuestion(int ids[], int numberOfQuestions)
{
  int idQuestion = rand() % numberOfQuestions + 1;
  while (ids[idQuestion])
    idQuestion = rand() % numberOfQuestions + 1;
  ids[idQuestion] = 1;
  return idQuestion;
}

int writeQuestion(int client, char question[])
{
  // trimitere intrebare
  question[strlen(question)] = '\0';
  if ((write(client, question, strlen(question))) < 0)
  {
    perror("Eroare la trimitere intrebare\n");
    return errno;
  }
  else
    printf("Intrebare trimisa cu succes:%s\n", question);
  return 1;
}

int readAnswer(int client, char answer[])
{
  // primire raspuns
  if ((read(client, answer, 1024)) < 0)
  {
    perror("Eroare la primire raspuns\n");
    return errno;
  }
  printf("Raspunsul primit este: %s\n", answer);
}

int addPoints(int id, int client, char answer[], char *address, int port, int score)
{
  char response[100];
  if (strcmp(answer, "\\quit") == 0) // optiunea \quit
  {
    printf("Deconectat de la %s:%d\n", address, port);
    close(client);
    return -1;
  }
  if (checkAnswer(id, answer) == 1)
  {
    strcpy(response, "Raspuns corect!");
    response[strlen(response)] = '\0';
    if ((write(client, response, strlen(response))) < 0)
    {
      perror("Eroare la trimitere intrebare\n");
      return errno;
    }
    return 1;
  }
  else
  {
    strcpy(response, "Raspuns gresit!");
    response[strlen(response)] = '\0';
    if ((write(client, response, strlen(response))) < 0)
    {
      perror("Eroare la trimitere intrebare\n");
      return errno;
    }
    return 0;
  }
}

int writeScore(int client, int score)
{
  char s[10];
  char *buff;
  sprintf(s, "%d", score);
  printf("Scorul este: %s\n", s);
  buff = s;

  if ((write(client, buff, sizeof(int *))) < 0)
  {
    perror("Eroare la trimitere punctaj\n");
    return errno;
  }
  else
    printf("Punctaj trimis cu succes:%s\n", buff);
  return 1;
}
