#include <stdio.h>
#include <sqlite3.h>
#include <stdlib.h>
#include <string.h>

char info[1000];

void openDB()
{
    sqlite3 *db;
    char *zErrMsg = 0;
    int rc;

    rc = sqlite3_open("test.db", &db);

    if (rc)
    {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        exit(0);
    }
    else
    {
        fprintf(stderr, "Opened database successfully\n\n");
    }
    sqlite3_close(db);
}

//data = data int the 4th argument of sqlite3_exec
//argc = the number of columns in row
//argv = an array of strings representing fields in the row
//colName = an array of strings representing column names
int question(void *data, int argc, char **argv, char **colName)
{
    int i;
    fprintf(stderr, "%s\n", (const char *)data);

    for (i = 0; i < argc; i++)
    {
        if (strcmp(colName[i], "intrebare") == 0)
        {
            strcpy(info, argv[i]);
        }
    }

    printf("\n");
    return 0;
}

int answer(void *data, int argc, char **argv, char **colName)
{
    int i;
    fprintf(stderr, "%s\n", (const char *)data);

    for (i = 0; i < argc; i++)
    {
        if (strcmp(colName[i], "raspuns") == 0)
        {
            printf("Raspunsul corect este: %s\n", argv[i]);
            if (strcmp(info, argv[i]) == 0)
            {
                printf("Raspuns corect!\n");
                strcpy(info, "1");
            }
            else
            {
                printf("Raspuns gresit! %s\n", info);
                strcpy(info, argv[i]);
            }
        }
    }

    printf("\n");
    return 0;
}

int login(void *data, int argc, char **argv, char **colName)
{
    int i;
    fprintf(stderr, "%s\n", (const char *)data);

    for (i = 0; i < argc; i++)
    {
        if (strcmp(colName[i], "username") == 0)
        {
            printf("username-ul gasit: %s\n", argv[i]);
            if (strcmp(info, argv[i]) == 0)
            {
                printf("Username gasit in BD  %s!\n", info);
                strcpy(info, "1");
            }
            else
            {
                printf("Username gresit! %s\n", info);
                strcpy(info, argv[i]);
            }
        }
    }

    printf("\n");
    return 0;
}

void getQuestion(int id, char text[])
{
    sqlite3 *db;
    char *zErrMsg = 0;
    char *sql = 0;
    int rc;
    char query[100] = "";
    const char *data = "Callback function called";

    rc = sqlite3_open("quizz.db", &db);

    if (rc)
    {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        exit(0);
    }
    else
    {
        fprintf(stderr, "Opened database successfully\n\n");
    }
    char charid[4];
    strcat(query, "SELECT * FROM intrebari WHERE id=");
    sprintf(charid, "%d", id);
    strcat(query, charid);
    printf("Query: %s\n", query);
    sql = query;

    rc = sqlite3_exec(db, sql, question, (void *)data, &zErrMsg);

    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    else
    {
        // printf("Operation done successfully\n");
        strcpy(text, info);
    }
    sql = 0;
    sqlite3_close(db);
}

int checkAnswer(int id, char response[])
{
    sqlite3 *db;
    char *zErrMsg = 0;
    char *sql = 0;
    int rc;
    char query[100] = "";
    const char *data = "Callback function for ANSWER called";

    rc = sqlite3_open("quizz.db", &db);

    if (rc)
    {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        exit(0);
    }
    else
    {
        fprintf(stderr, "Opened database successfully\n\n");
    }
    char charid[4];
    strcat(query, "SELECT * FROM intrebari WHERE id=");
    sprintf(charid, "%d", id);
    strcat(query, charid);
    printf("Query: %s\n", query);
    sql = query;

    strcpy(info, response);
    rc = sqlite3_exec(db, sql, answer, (void *)data, &zErrMsg);

    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    else
    {
        info[1] = '\0';
        printf("Operation done successfully = %s\n", info);
        if (strcmp(info, "1") == 0) //raspuns corect
        {
            printf("Corect\n");
            return 1;
        }
        else
            return 0; //raspuns gresit
    }
    sql = 0;
    sqlite3_close(db);
}

int checkCredentials(char username[], char password[])
{
    sqlite3 *db;
    char *zErrMsg = 0;
    char *sql = 0;
    int rc;
    char query[100] = "";
    const char *data = "Callback function for USERNAME called";

    rc = sqlite3_open("quizz.db", &db);

    if (rc)
    {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        exit(0);
    }
    else
    {
        fprintf(stderr, "Opened database successfully\n\n");
    }

    strcpy(query, "SELECT * FROM users WHERE username=\"");
    strcat(query, username);
    strcat(query, "\"");
    sql = query;
    printf("QUERY: %s\n", sql);

    strcpy(info, username);
    rc = sqlite3_exec(db, sql, login, (void *)data, &zErrMsg);


    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    else
    {
        // info[1] = '\0';
        printf("cred Operation done successfully = %s\n", info);
        if (info[0] == '1') //username corect
        {
            printf("Username corect\n");
            return 1;
        }
        else
        {
            printf("Username gresit :%s\n", info);
            return 0;
        } //username gresit
    }

    sql = 0;
    sqlite3_close(db);
}