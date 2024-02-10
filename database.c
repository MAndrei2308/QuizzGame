#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "database.h"

int callback_COUNT(void *data, int argc, char **argv, char **col)
{
    char count[11];
    strcpy(count, argv[0]);
    Quizz.nrIntrebari = atoi(&count[0]);
    return 0;
}

void selectareNrIntrebari()
{
    char *comanda_sql = "SELECT COUNT(*) FROM QUIZZ";
    if (sqlite3_open("questions.db", &BD) != SQLITE_OK)
        fprintf(stderr, "[BD]Eroare la deschidere\n");
    if (sqlite3_exec(BD, comanda_sql, callback_COUNT, NULL, NULL) != SQLITE_OK)
        fprintf(stderr, "[BD]Eroare la extragerea COUNT\n");
    if (sqlite3_close(BD) != SQLITE_OK)
        fprintf(stderr, "[BD]Eroare la inchidere\n");
}

int callback_OPT_C(void *data, int argc, char **argv, char **col)
{
    Quizz.numarIntrebare++;
    for (int i = 1; i < argc; i++)
    {
        Quizz.raspunsuri_corecte[Quizz.numarIntrebare] = atoi(argv[i]);
    }
    return 0;
}

void selectareOPT_C()
{
    char *comanda_sql = "SELECT ID, OPT_C FROM QUIZZ";
    if (sqlite3_open("questions.db", &BD) != SQLITE_OK)
        fprintf(stderr, "[BD]Eroare la deschidere\n");
    if (sqlite3_exec(BD, comanda_sql, callback_OPT_C, NULL, NULL) != SQLITE_OK)
        fprintf(stderr, "[BD]Eroare la extragerea OPT_C\n");
    if (sqlite3_close(BD) != SQLITE_OK)
        fprintf(stderr, "[BD]Eroare la inchidere\n");
}

int callback_QUESTION(void *data, int argc, char **argv, char **col)
{
    sprintf(Quizz.date, "%s: %s\n", col[0], argv[0]);
    return 0;
}

void selectareIntrebare(int id)
{
    char comanda_sql[256];
    sprintf(comanda_sql, "SELECT QUESTION FROM QUIZZ WHERE ID=%d", id);
    if (sqlite3_open("questions.db", &BD) != SQLITE_OK)
        fprintf(stderr, "[BD]Eroare la deschidere\n");
    if (sqlite3_exec(BD, comanda_sql, callback_QUESTION, NULL, NULL) != SQLITE_OK)
        fprintf(stderr, "[BD]Eroare la extragerea QUESTION\n");
    if (sqlite3_close(BD) != SQLITE_OK)
        fprintf(stderr, "[BD]Eroare la inchidere\n");
}

int callback_OPTIONS(void *data, int argc, char **argv, char **col)
{
    char date[1024];
    strcpy(date, "\0");
    strcpy(Quizz.date, "\0");

    for (int i = 0; i < argc; i++)
    {
        char aux[256];
        sprintf(aux, "%s: %s\n", col[i], argv[i]);
        strcat(date, aux);
    }
    strcat(date, "\0");
    strcat(Quizz.date, date);
    return 0;
}

void selectareVariante(int id)
{
    char comanda_sql[256];
    sprintf(comanda_sql, "SELECT OPT1, OPT2, OPT3, OPT4 FROM QUIZZ WHERE ID=%d", id);
    if (sqlite3_open("questions.db", &BD) != SQLITE_OK)
        fprintf(stderr, "[BD]Eroare la deschidere\n");
    if (sqlite3_exec(BD, comanda_sql, callback_OPTIONS, NULL, NULL) != SQLITE_OK)
        fprintf(stderr, "[BD]Eroare la extragerea OPTIONS\n");
    if (sqlite3_close(BD) != SQLITE_OK)
        fprintf(stderr, "[BD]Eroare la inchidere\n");
}

int callback_TIME(void *data, int argc, char **argv, char **col)
{
    sprintf(Quizz.date, "%s: %s\n", col[0], argv[0]);
    Quizz.timp_alocat = atoi(&Quizz.date[5]);
    return 0;
}

void selectareTimpAlocat(int id)
{
    char comanda_sql[256];
    sprintf(comanda_sql, "SELECT TIME FROM QUIZZ WHERE ID=%d", id);
    if (sqlite3_open("questions.db", &BD) != SQLITE_OK)
        fprintf(stderr, "[BD]Eroare la deschidere\n");
    if (sqlite3_exec(BD, comanda_sql, callback_TIME, NULL, NULL) != SQLITE_OK)
        fprintf(stderr, "[BD]Eroare la extragerea TIME\n");
    if (sqlite3_close(BD) != SQLITE_OK)
        fprintf(stderr, "[BD]Eroare la inchidere\n");
}
