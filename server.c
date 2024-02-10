#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/syscall.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <signal.h>
#include "database.c"

#define PORT 2308

typedef struct
{
    int id;
    int socket;
    int status; // activ, inactiv
    char nume[256];
    struct sockaddr_in address;
} client;

struct info
{
    char nume[256];
    int socket;
    int status;
    int punctaj;
} infoClient[1000];

struct server
{
    int socket;
    struct sockaddr_in address;
} server;

struct partida
{
    int numar_jucatori;
    int numar_jucatori_activi;
    int numar_jucatori_finalizat;
    char clasament[1024];
} partida;

pthread_mutex_t mlock = PTHREAD_MUTEX_INITIALIZER;

void configurareServer();
void creareSocketServer();
void *handleClient(void *arg);
void conectareClienti();
void incepereJoc(void *arg);
void reincepereJoc();
void creareClasament();
void afisarePozitieInClasament(int pozitie);
void extragereDate(int id, void *arg);
void raspunsJucator(int id, void *arg);
void trimitereDate(void *arg);
void verificareRaspuns(int id, int raspuns, void *arg);

bool if_accept = true;
int timp_accept = 60;

void alarmHandler(int timp_accept)
{
    if (timp_accept == SIGALRM)
    {
        printf("\nPerioada de inscriere la joc s-a terminat.\n");
        if_accept = false;
    }
}

int main()
{
    selectareNrIntrebari();
    selectareOPT_C();

    configurareServer();
    creareSocketServer();

    signal(SIGALRM, alarmHandler);
    alarm(timp_accept);
    while (1)
    {
        if (if_accept = true)
        {
            conectareClienti();
        }
    }

    return 0;
}

void configurareServer()
{
    bzero(&server.address, sizeof(server.address));
    server.address.sin_family = AF_INET;                // stabilirea familiei de socket-uri
    server.address.sin_addr.s_addr = htonl(INADDR_ANY); // acceptam orice adresa
    server.address.sin_port = htons(PORT);              // utilizam un port utilizator
}

void creareSocketServer()
{
    if ((server.socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("[server]Eroare la crearea socket-ului.\n");
        exit(EXIT_FAILURE);
    }

    int enable = 1;
    if (setsockopt(server.socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) == -1)
    {
        perror("[server]Eroare la reutilizarea adresei.\n");
        exit(EXIT_FAILURE);
    }

    if (bind(server.socket, (struct sockaddr *)&server.address, sizeof(server.address)) == -1)
    {
        perror("[server]Eroare la bind.\n");
        close(server.socket);
        exit(EXIT_FAILURE);
    }
    if (listen(server.socket, 1) == -1)
    {
        perror("[server]Eroare la listen.\n");
        close(server.socket);
        exit(EXIT_FAILURE);
    }
    printf("Serverul asculta la portul %d...\n\n", PORT);
    fflush(stdout);
    printf("\nJocul va incepe in 60 de secunde.\n");
    fflush(stdout);
    sleep(30);
    printf("\nJocul va incepe in 30 de secunde.\n");
    fflush(stdout);
    sleep(15);
    printf("\nJocul va incepe in 15 de secunde.\n");
    fflush(stdout);
    sleep(10);
    printf("\nJocul va incepe in:\n     5     \n");
    fflush(stdout);
    sleep(1);
    printf("     4     \n");
    fflush(stdout);
    sleep(1);
    printf("     3     \n");
    fflush(stdout);
    sleep(1);
    printf("     2     \n");
    fflush(stdout);
    sleep(1);
    printf("     1     \n");
    fflush(stdout);
    sleep(1);
    printf("\nJocul a inceput!\n");
    fflush(stdout);
}

void reincepereJoc()
{
    if (listen(server.socket, 1) == -1)
    {
        perror("[server]Eroare la listen.\n");
        close(server.socket);
        exit(EXIT_FAILURE);
    }
    printf("Serverul asculta la portul %d...\n\n", PORT);
    fflush(stdout);
    printf("\nJocul va incepe in 60 de secunde.\n");
    fflush(stdout);
    sleep(30);
    printf("\nJocul va incepe in 30 de secunde.\n");
    fflush(stdout);
    sleep(15);
    printf("\nJocul va incepe in 15 de secunde.\n");
    fflush(stdout);
    printf("     4     \n");
    fflush(stdout);
    sleep(1);
    printf("     3     \n");
    fflush(stdout);
    sleep(1);
    printf("     2     \n");
    fflush(stdout);
    sleep(1);
    printf("     1     \n");
    fflush(stdout);
    sleep(1);
    printf("\nJocul a inceput!\n");
    fflush(stdout);
}

void conectareClienti()
{
    client *client_th;
    socklen_t clientLength = sizeof(client_th->address);
    if ((client_th = ((client *)malloc(sizeof *client_th))) == NULL)
    {
        perror("[thread]Eroare la alocarea memoriei.\n");
        exit(EXIT_FAILURE);
    }

    int clientSocket;
    pthread_t threadId;

    if ((clientSocket = accept(server.socket, (struct sockaddr *)&client_th->address, &clientLength)) == -1)
    {
        perror("[thread]Eroare la accept.\n");
    }
    client_th->socket = clientSocket;
    client_th->id = partida.numar_jucatori++;
    partida.numar_jucatori_activi++;
    client_th->status = 1;
    infoClient[client_th->id].socket = clientSocket;

    if (pthread_create(&threadId, NULL, handleClient, (void *)client_th) == -1)
    {
        perror("[thread]Eroare la creare.\n");
    }
}

void *handleClient(void *arg)
{
    client *client_th = (client *)arg;
    int clientSocket = client_th->socket;
    struct sockaddr_in socketAddr = client_th->address;

    char numeJucator[256];
    memset(numeJucator, 0, sizeof(numeJucator));

    if (recv(clientSocket, numeJucator, sizeof(numeJucator), 0) == -1)
    {
        perror("[Server]Eroare la primire numelui.\n");
        exit(EXIT_FAILURE);
    }

    if (send(clientSocket, &Quizz.nrIntrebari, sizeof(Quizz.nrIntrebari), 0) == -1)
    {
        perror("[Server]Eroare la trimiterea numarului de intrebari.\n");
        exit(EXIT_FAILURE);
    }

    strcpy(client_th->nume, numeJucator);
    strcpy(infoClient[client_th->id].nume, numeJucator);
    infoClient[client_th->id].status = 1;

    printf("Jucatorul %s s-a conectat.\n", numeJucator);
    fflush(stdout);

    incepereJoc(arg);

    close((intptr_t)arg);

    if (partida.numar_jucatori_activi == partida.numar_jucatori_finalizat)
    {
        printf("\n       Meciul s-a terminat!       \n\n   Multumim pentru participare!   \n");
        fflush(stdout);

        creareClasament();

        for (int i = 0; i < partida.numar_jucatori; i++)
        {
            afisarePozitieInClasament(i);
            infoClient[i].punctaj = 0;
        }

        partida.numar_jucatori = 0;
        partida.numar_jucatori_activi = 0;
        partida.numar_jucatori_finalizat = 0;
        partida.clasament[0] = '\0';

        printf("Un nou joc va incepe in curand...\n\n");
        fflush(stdout);
        sleep(10);
        reincepereJoc();
    }

    // printf("Serverul asculta la portul %d...\n\nJocul va incepe in 60 de secunde.\n", PORT);
    // fflush(stdout);
    // sleep(30);
    // printf("\nJocul va incepe in 30 de secunde.\n");
    // fflush(stdout);
    // sleep(15);
    // printf("\nJocul va incepe in 15 de secunde.\n");
    // fflush(stdout);
    // sleep(10);
    // printf("\nJocul va incepe in:\n     5     \n");
    // fflush(stdout);
    // sleep(1);
    // printf("     4     \n");
    // fflush(stdout);
    // sleep(1);
    // printf("     3     \n");
    // fflush(stdout);
    // sleep(1);
    // printf("     2     \n");
    // fflush(stdout);
    // sleep(1);
    // printf("     1     \n");
    // fflush(stdout);
    // sleep(1);
    // printf("\nJocul a inceput!\n");
    // fflush(stdout);
    return (NULL);
}

void incepereJoc(void *arg)
{
    client *client_th = (client *)arg;

    for (int i = 1; i <= Quizz.nrIntrebari; i++)
    {
        if (client_th->status != 1)
            break;

        pthread_mutex_lock(&mlock);
        extragereDate(i, arg);
        pthread_mutex_unlock(&mlock);
        raspunsJucator(i, arg);
    }
    if (client_th->status != 1)
    {
        return;
    }
    else
    {
        if (client_th->status == 1)
        {
            printf("Jucatorul %s a incheiat jocul cu %d puncte.\n", client_th->nume, infoClient[client_th->id].punctaj);
            fflush(stdout);
            char mesajFinalizareJoc[256];
            sprintf(mesajFinalizareJoc, "\nFelicitari! Ai terminat jocul cu %d puncte.\n\n", infoClient[client_th->id].punctaj);
            if (send(client_th->socket, mesajFinalizareJoc, sizeof(mesajFinalizareJoc), 0) == -1)
            {
                perror("[Server]Eroare la trimiterea mesajului de final de joc.\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

void creareClasament()
{
    char clasament[1024];
    int ok;
    do
    {
        ok = 0;
        for (int i = 0; i < partida.numar_jucatori - 1; i++)
        {
            if (infoClient[i].punctaj < infoClient[i + 1].punctaj)
            {
                struct info aux = infoClient[i];
                infoClient[i] = infoClient[i + 1];
                infoClient[i + 1] = aux;
                ok = 1;
            }
        }
    } while (ok == 1);

    for (int i = 0; i < partida.numar_jucatori; i++)
    {
        if (infoClient[i].status != -1)
        {
            sprintf(clasament, "[Locul %d]: %s (%d puncte)\n", i + 1, infoClient[i].nume, infoClient[i].punctaj);
        }
        else
        {
            sprintf(clasament, "[Descalificat]: %s\n", infoClient[i].nume);
        }
        strcat(partida.clasament, clasament);
    }
}

void afisarePozitieInClasament(int pozitie)
{
    if (infoClient[pozitie].status == -1)
    {
        return;
    }
    if (send(infoClient[pozitie].socket, partida.clasament, sizeof(partida.clasament), 0) == -1)
    {
        perror("[Clasament]Eroare la primire.");
        exit(EXIT_FAILURE);
    }
    close(infoClient[pozitie].socket);
}

void extragereDate(int id, void *arg)
{
    selectareIntrebare(id);
    trimitereDate(arg);
    selectareVariante(id);
    trimitereDate(arg);
    selectareTimpAlocat(id);
    trimitereDate(arg);
}

void trimitereDate(void *arg)
{
    client *client_th = (client *)arg;
    if (send(client_th->socket, Quizz.date, sizeof(Quizz.date), 0) == -1)
    {
        perror("[Server]Eroare la trimiterea datelor.\n");
        exit(EXIT_FAILURE);
    }
}

void raspunsJucator(int id, void *arg)
{
    client *client_th = (client *)arg;
    struct timeval timp;
    int nfds; /* numarul maxim de descriptori */

    /* completam multimea de descriptori de citire */
    fd_set actfds;
    FD_ZERO(&actfds);                                  /* initial, multimea este vida */
    FD_SET(infoClient[client_th->id].socket, &actfds); /* includem in multime socketul clientului */

    timp.tv_sec = Quizz.timp_alocat;
    // timp.tv_usec = 0;

    /* valoarea maxima a descriptorilor folositi */
    nfds = infoClient[client_th->id].socket;

    int selectReturn = select(nfds + 1, &actfds, NULL, NULL, &timp);
    int raspuns;
    int mesajExpirareTimp = 0;

    if (selectReturn == -1)
    {
        perror("[Select]Eroare la selectarea raspunsului.\n");
        exit(EXIT_FAILURE);
    }

    if (selectReturn == 0)
    {
        printf("Jucatorul %s nu a raspuns la intrebarea %d.\n", client_th->nume, id);
        fflush(stdout);

        mesajExpirareTimp = 1;
        if (send(client_th->socket, &mesajExpirareTimp, sizeof(mesajExpirareTimp), 0) == -1)
        {
            perror("[Server]Eroare la trimiterea mesajului (timpul a expirat 1!).\n");
            exit(EXIT_FAILURE);
        }

        if (recv(client_th->socket, &raspuns, sizeof(raspuns), 0) == -1)
        {
            perror("[Server]Eroare la primirea raspunsului.\n");
            exit(EXIT_FAILURE);
        }
        raspuns = -1;
        client_th->status = -1;
        infoClient[client_th->id].status = -1;
        infoClient[client_th->id].punctaj = -1;
        partida.numar_jucatori_activi--;

        printf("%d", infoClient[client_th->id].status);
        fflush(stdout);
        close(infoClient[client_th->id].socket);
    }
    else
    {
        if (recv(client_th->socket, &raspuns, sizeof(raspuns), 0) == -1)
        {
            perror("[Server]Eroare la primirea raspunsului.\n");
            exit(EXIT_FAILURE);
        }

        mesajExpirareTimp = 0;
        if (send(client_th->socket, &mesajExpirareTimp, sizeof(mesajExpirareTimp), 0) == -1)
        {
            perror("[Server]Eroare la trimiterea mesajului (timpul a expirat 0!).\n");
            exit(EXIT_FAILURE);
        }

        if (raspuns == -1)
        {
            client_th->status = -1;
            infoClient[client_th->id].status = -1;
            infoClient[client_th->id].punctaj = -1;
            partida.numar_jucatori_activi--;

            printf("Jucatorul %s a parasit jocul.\n", client_th->nume);
            fflush(stdout);
            close(infoClient[client_th->id].socket);
            return;
        }
        else
        {
            printf("Jucatorul %s a raspuns la intrebarea %d cu varianta %d.\n", client_th->nume, id, raspuns);
            fflush(stdout);
        }
    }
    verificareRaspuns(id, raspuns, arg);
}

void verificareRaspuns(int id, int raspuns, void *arg)
{
    client *client_th = (client *)arg;
    if (raspuns == Quizz.raspunsuri_corecte[id])
        infoClient[client_th->id].punctaj += 100 / Quizz.nrIntrebari;

    if (id == Quizz.nrIntrebari)
    {
        infoClient[client_th->id].status = 0;
        partida.numar_jucatori_finalizat++;
    }
}
