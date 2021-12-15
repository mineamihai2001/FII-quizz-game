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

void getCredentials(int client, char username[], char password[])
{
  if ((read(client, username, 100)) < 0)
  {
    perror("Eroare la primire credentiale\n");
    exit(0);
  }
  if ((read(client, password, 100)) < 0)
  {
    perror("Eroare la primire credentiale\n");
    exit(0);
  }
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

int pickQuestion(int ids[])
{
  int idQuestion = rand() % 7 + 1;
  while (ids[idQuestion])
    idQuestion = rand() % 8;
  ids[idQuestion] = 1;
  return idQuestion;
}

int writeQuestion(int client, char question[])
{
  //trimitere intrebare
  if ((write(client, question, strlen(question))) < 0)
  {
    perror("Eroare la trimitere intrebare\n");
    return errno;
  }
  printf("Intrebare trimisa cu succes\n");
  return 1;
}

int readAnswer(int client, char answer[])
{
  //primire raspuns
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
    return 0;
  }
  if (checkAnswer(id, answer) == 1)
  {
    if ((write(client, "Raspuns corect", 15)) < 0)
    {
      perror("Eroare la trimitere intrebare\n");
      return errno;
    }
  }
  else
  {
    if ((write(client, "Raspuns gresit", 15)) < 0)
    {
      perror("Eroare la trimitere intrebare\n");
      return errno;
    }
  }
  return 1;
}
