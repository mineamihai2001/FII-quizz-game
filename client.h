#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>

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
  char question[100] = "";
  int bytes;
  printf("Receiving...\n");
  if ((bytes = read(sd, question, 99)) < 0)
  {
    perror("Eroare la primire intrebare\n");
    return errno;
  }
  question[bytes] = '\0';
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
  char buffer[50];
  int bytes;
  if ((bytes = read(sd, buffer, 20)) < 0)
  {
    perror("Eroare la primire rezultat\n");
    return errno;
  }
  else
  {
    buffer[bytes] = '\0';
    printf("\n\t\tREZULTAT: %s\n\n", buffer);
  }
  return 1;
}
