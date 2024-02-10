#ifndef DATABASE_H
#define DATABASE_H

#include <sqlite3.h>

sqlite3* BD;

struct quizz
{
    char date[256];
    int raspunsuri_corecte[256];
    int timp_alocat;
    int numarIntrebare;
    int nrIntrebari;
} Quizz;

int callback_COUNT(void *data, int argc, char **argv, char **col);
int callback_OPT_C(void *data, int argc, char **argv, char **col);
int callback_QUESTION(void *data, int argc, char **argv, char **col);
int callback_OPTIONS(void *data, int argc, char **argv, char **col);
int callback_TIME(void *data, int argc, char **argv, char **col);
void selectareNrIntrebari();
void selectareOPT_C();
void selectareIntrebare(int id);
void selectareVariante(int id);
void selectareTimpAlocat(int id);

#endif