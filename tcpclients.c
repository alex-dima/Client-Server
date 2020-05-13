#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "helpers.h"

typedef struct ClientTCP
{
    int online;
    char id[10];
    int socket;
    int offmsgSize;
    int offmsgASize;
    char **offmsg;
} clientTCP;

int findID (clientTCP* vect , int size , char * id) //ne returneaza indicele clientului cu id-ul id
{
    int i;
    for(i = 0; i < size; i++)
    {
        if( strcmp(vect[i].id, id) == 0)
        {
            return i;
        }
    }
    return -1;
}

clientTCP* reallocateTCP(clientTCP* vect, int* size) //aloca pt vector un spatiu dublu fata de cel anterior
{   
    clientTCP *forrealloc;
    int i;
    (*size) = (*size) * 2;
    forrealloc = (clientTCP*) calloc((*size), sizeof(clientTCP)); 
    for(i = 0; i < (*size)/2; i++)
    {
        strcpy(forrealloc[i].id,vect[i].id);
        forrealloc[i].socket = vect[i].socket;
    }

    free(vect);
    return forrealloc;
}

int findSocket (clientTCP* vect, int size, int sock) //ne returneaza indicele clientului cu socketul sock
{
    int i;
    for(i = 0; i < size; i++)
    {
        if(vect[i].socket == sock)
        return i;
    }
    return -1;
}

char** reallocateOFFMSG (char** vect, int* size) //aloca pt vector un spatiu dublu fata de cel anterior
{
    char **forrealloc;
    int i;
    (*size) = (*size) * 2;
    forrealloc = (char**) calloc((*size), sizeof(char*));
    for(i = 0; i < (*size)/2; i++)
    {
        forrealloc[i] = (char*) calloc(BUFLEN,sizeof(char));
        strcpy(forrealloc[i],vect[i]);
    }
    return forrealloc;
}