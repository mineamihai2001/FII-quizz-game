#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <time.h>

int errno;

int writeNumberOfPlayers(int sd, char numberOfPlayers[])
{
    if ((write(sd, numberOfPlayers, 10)) < 0)
    {
        perror("Eroare la scriere numarul de clienti\n");
        return errno;
    }
    return 0;
}

void getPlayers(int sd, char numberOfPlayers[])
{
    printf("[admin] Introduceti numarul de jucatori\n");
    fgets(numberOfPlayers, 10, stdin);

    writeNumberOfPlayers(sd, numberOfPlayers);

    printf("Lobby creat\n");
}

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
    if ((bytes = read(sd, buffer, 15)) < 0)
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

int getScore(int sd)
{
    char score[5];
    int bytes;
    if ((bytes = read(sd, score, 3)) < 0)
    {
        perror("Eroare la primire raspuns\n");
        return errno;
    }
    score[bytes] = '\0';
    system("clear");
    printf("\n\n\n\t\t\t\t\tScor final: %s\n\n\n\n", score);
    return 0;
}

void animation(int ms)
{
    for (int i = 0; i < 1; ++i)
    {
        system("clear");
        printf("Asteptare intrebare.\n");
        sleep(ms);
        system("clear");
        printf("Asteptare intrebare..\n");
        sleep(ms);
    }
}