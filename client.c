#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 2728

int errno;

void inputCredentials(int sd, char username[], char password[])
{

  printf("Introduceti username-ul: ");
  fgets(username, 100, stdin);
  username[strlen(username) - 1] = '\0';
  printf("Introduceti parola: ");
  fgets(password, 100, stdin);
  password[strlen(password) - 1] = '\0';
}

int sendCredentials(int sd, char username[], char password[])
{
  if ((write(sd, username, 100)) < 0)
  {
    perror("Eroare la trimitere username\n");
    return errno;
  }
  if ((write(sd, password, 100)) < 0)
  {
    perror("Eroare la trimitere parola\n");
    return errno;
  }
  return 1;
}

int readQuestion(int sd)
{
  char question[1000] = "";
  if ((read(sd, question, 1000)) < 0)
  {
    perror("Eroare la primire intrebare\n");
    return errno;
  }
  else
    printf("Intrebare: %s\n", question);
}

int writeAnswer(int sd)
{
  char answer[100];

  printf("Introduceti raspunsul: ");
  fgets(answer, 100, stdin);
  answer[strlen(answer) - 1] = '\0';
  if ((write(sd, answer, 100)) == -1)
  {
    perror("Eroare la trimitere raspuns\n");
    return errno;
  }
  if (strcmp(answer, "\\quit") == 0)
  {
    close(sd);
    return 0;
  }
  return 1;
}

int isLogged(int sd, char username[])
{
  char response[100];
  if ((read(sd, response, 100)) < 0)
  {
    perror("Eroare la primire feedback\n");
    return errno;
  }
  if (response[0] == '1')
  {
    printf("\n\t\tLogare cu success cu username: %s\n\n", username);
    return 1;
  }
  else
    printf("\n\t\tUsername/Parola incorecte\n\n");
  return 0;
}

int getFeedback(int sd)
{
  char buffer[1024];
  if ((read(sd, buffer, sizeof(buffer))) < 0)
  {
    perror("Eroare la primire rezultat\n");
    return errno;
  }
  else
    printf("\n\t\tREZULTAT: %s\n\n", buffer);
  return 1;
}

int main()
{

  int sd, ret;
  struct sockaddr_in serverAddr;
  char buffer[1024];
  char username[100], password[100];

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
    printf("Eroare la connect()\n");
    exit(1);
  }
  printf("Conexiune cu success la server\n\n");

  //loop pentru trimitere credentiale
  while (1)
  {
    inputCredentials(sd, username, password);
    sendCredentials(sd, username, password);
    if (isLogged(sd, username))
      break;
  }

  while (1)
  {
    readQuestion(sd);
    if (writeAnswer(sd) == 0) // optiunea quit
      break;
    getFeedback(sd);
  }

  return 0;
}