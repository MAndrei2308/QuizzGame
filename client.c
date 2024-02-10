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

#define BUFFER_SIZE 256
int PORT;
int numarVariante = 4;
int numarIntrebari = 0;

struct client
{
    int socket;
    struct sockaddr_in address;
    bool plecat;
} client;

void conectareJucator();
void joacaRunda();
void afisareClasament();
void primesteRaspuns(int timp_alocat);

void signalHandler()
{
    printf("\nEsti sigur ca vrei sa parasesti jocul? Daca da, scrie <exit> sau <quit>\n");
    printf("Raspunsul tau: ");
    fflush(stdout);
}

void alarmHandler(int timp_alocat)
{
    if(timp_alocat==SIGALRM)
    {
        printf("Ai fost scos din joc pentru inactivitate...");
        fflush(stdout);
        char raspuns[BUFFER_SIZE];
        sprintf(raspuns,"0");
        send(client.socket, raspuns, sizeof(raspuns),0);
    }
}

// void alarmHandler_1(int timp_alocat)
// {
//     if(timp_alocat==SIGALRM)
//     {
//         char orice[BUFFER_SIZE];
//         printf("Hei! Mai esti aici? Scrie orice , apoi apasa pe ENTER pentru pentru a nu fi deconectat!!!");
//         scanf("%s",orice);
//         if(strlen(orice)>0) primesteRaspuns(timp_alocat);
//     }
// }

void semnale()
{
    // signal(SIGINT, signalHandler);
    signal(SIGHUP, signalHandler);
    signal(SIGTERM, signalHandler);
    signal(SIGTSTP, signalHandler);
    signal(SIGQUIT, signalHandler);
    signal(SIGKILL, signalHandler);
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Sintaxa: %s <adresa_server> <port>\n", argv[0]);
        fflush(stdout);
        return 1;
    }

    PORT = atoi(argv[2]);
    semnale();

    client.address.sin_family = AF_INET;                 // stabilirea familiei de socket-uri
    client.address.sin_addr.s_addr = inet_addr(argv[1]); // ne conectam la portul serverului
    client.address.sin_port = htons(PORT);               // utilizam un port utilizator

    conectareJucator();
    int numarIntrebare = 0;
    while (1)
    {
        if (client.plecat == true)
        {
            printf("Ai parasit jocul.\n");
            fflush(stdout);
            break;
        }
        else
        {
            if (numarIntrebare < numarIntrebari)
            {
                numarIntrebare++;
                joacaRunda();
            }
            else
            {
                if (numarIntrebare == numarIntrebari)
                {
                    char mesajTerminare[BUFFER_SIZE];
                    if (recv(client.socket, mesajTerminare, sizeof(mesajTerminare), 0) == -1)
                    {
                        perror("[Client]Eroare la primirea mesajului de finalizare joc.\n");
                        exit(EXIT_FAILURE);
                    }

                    printf("%s", mesajTerminare);
                    fflush(stdout);

                    afisareClasament();

                    return 0;
                }
            }
        }
    }
    return 0;
}

void conectareJucator()
{
    if ((client.socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("[Client]Eroare la crearea socket-ului.\n");
        exit(EXIT_FAILURE);
    }

    if (connect(client.socket, (struct sockaddr *)&client.address, sizeof(client.address)) == -1)
    {
        perror("[Client]Eroare la conectare.\n");
        exit(EXIT_FAILURE);
    }

    printf("Bine ai venit la acest quiz! Introdu-ti numele de jucator: ");
    fflush(stdout);
    client.plecat = false;
    char numeJucator[BUFFER_SIZE];
    scanf("%s", numeJucator);

    if (send(client.socket, numeJucator, sizeof(numeJucator), 0) == -1)
    {
        perror("[Client]Eroare la trimiterea numelui");
        exit(EXIT_FAILURE);
    }

    printf("\nAsteptam sa se conecteze si alti jucatori.\n");
    fflush(stdout);
    sleep(1);
    printf("Jocul va incepe in curand...\n\n");
    fflush(stdout);

    if (recv(client.socket, &numarIntrebari, sizeof(numarIntrebari), 0) == -1)
    {
        perror("[Client]Eroare la primirea numarului de variante");
        exit(EXIT_FAILURE);
    }

}

void joacaRunda()
{
    signal(SIGALRM,alarmHandler);
    if (client.plecat == true)
        return;

    char intrebare[BUFFER_SIZE];
    char variante[BUFFER_SIZE];
    char timp[BUFFER_SIZE];

    if (recv(client.socket, intrebare, sizeof(intrebare), 0) == -1)
    {
        perror("[Client]Eroare la primirea intrebarii.\n");
        exit(EXIT_FAILURE);
    }

    if (recv(client.socket, variante, sizeof(variante), 0) == -1)
    {
        perror("[Client]Eroare la primirea variantelor.\n");
        exit(EXIT_FAILURE);
    }

    if (recv(client.socket, timp, sizeof(timp), 0) == -1)
    {
        perror("[Client]Eroare la primirea timpului alocat.\n");
        exit(EXIT_FAILURE);
    }

    timp[strlen(timp)-1]='\0';
    printf("Intrebare: %s\n" ,&intrebare[10]);
    printf("%s\n", variante);
    printf("Timp alocat acestei intrebari: %s secunde.\n", &timp[6]);
    fflush(stdout);
    int timp_alocat = atoi(&timp[6]);

    alarm(timp_alocat);
    primesteRaspuns(timp_alocat);
    alarm(0);

}

void afisareClasament()
{
    char clasament[1024];

    if (recv(client.socket, clasament, sizeof(clasament), 0) == -1)
    {
        perror("[Client]Eroare la primirea clasamentului.\n");
        exit(EXIT_FAILURE);
    }

    printf("\n%s\n", clasament);
    fflush(stdout);

    close(client.socket);
}

void primesteRaspuns(int timp_alocat)
{
    char varianta_aleasa[BUFFER_SIZE];
    int varianta_aleasa_int;
    int mesajParasireJoc = -1;
    int mesajExpirareTimp = 0;

    bool raspuns_valid = false;
    while (!raspuns_valid)
    {
        printf("Raspunsul tau: ");
        fflush(stdout);
        scanf("%s", varianta_aleasa);

        if (strcmp(varianta_aleasa, "exit") == 0 || strcmp(varianta_aleasa, "quit") == 0)
        {
            if (send(client.socket, &mesajParasireJoc, sizeof(mesajParasireJoc), 0) == -1)
            {
                perror("[Client]Eroare la trimiterea mesajului de parasire joc.\n");
                exit(EXIT_FAILURE);
            }

            if (recv(client.socket, &mesajExpirareTimp, sizeof(mesajExpirareTimp), 0) == -1)
            {
                perror("[Client]Eroare la primirea mesajului de expirare timp alocat.\n");
                exit(EXIT_FAILURE);
            }

            client.plecat = true;
            raspuns_valid = true;
            alarm(0);
        }
        else
        {
            if (strlen(varianta_aleasa) > 1)
            {
                printf("Raspuns invalid...\n");
                fflush(stdout);
            }
            else
            {
                varianta_aleasa_int = atoi(&varianta_aleasa[0]);

                if (varianta_aleasa_int > 0 && varianta_aleasa_int <= numarVariante)
                {
                    if (send(client.socket, &varianta_aleasa_int, sizeof(varianta_aleasa_int), 0) == -1)
                    {
                        perror("[Client]Eroare la trimiterea variantei de raspuns.\n");
                        exit(EXIT_FAILURE);
                    }

                    if (recv(client.socket, &mesajExpirareTimp, sizeof(mesajExpirareTimp), 0) == -1)
                    {
                        perror("[Client]Eroare la primirea mesajului de expirare timp alocat.\n");
                        exit(EXIT_FAILURE);
                    }

                    if (mesajExpirareTimp == 1)
                    {
                        printf("Timpul  a expirat! Raspunsul tau nu se va lua in considerare.\n");
                        fflush(stdout);
                    }

                    raspuns_valid = true;
                    alarm(0);
                }
                else
                {
                    printf("Raspuns invalid...\n");
                    fflush(stdout);
                }
            }
        }
    }
}