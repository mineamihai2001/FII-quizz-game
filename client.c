#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include "client.h"

#define PORT 2728

int errno;
int adminConnected = 0;

int main()
{
  int sd, counter = 1, numberOfQuestions = 4;
  struct sockaddr_in serverAddr;
  char username[100], password[100];
  char numberOfPlayers[10] = "";
  int firstQuestion = 0;

  clock_t before = clock();
  int msec = 0;

  system("clear");

  sd = socket(AF_INET, SOCK_STREAM, 0);
  if (sd < 0)
  {
    perror("Eroare la socket\n");
    return errno;
  }

  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(PORT);
  serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

  if ((connect(sd, (struct sockaddr *)&serverAddr, sizeof(serverAddr))) < 0)
  {
    perror("Eroare la connect()\n");
    return errno;
  }
  printf("Asteptare clienti... (nu apasati nicio tasta)\n\n");
  sleep(10);
  // verificare credentiale
  while (1)
  {
    inputCredentials(sd, username, password);
    sendCredentials(sd, username, password);
    if (isLogged(sd, username))
      break;
  }


  // comunicare cu serverul
  while (1)
  {

    if (counter == numberOfQuestions)
    {
      getScore(sd);
      break;
    }
    readQuestion(sd);
    if (writeAnswer(sd) == 0)
      break;
    getFeedback(sd);
    counter++;
  }

  return 0;
}