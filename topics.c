#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct StrClients
{
    char id[10];
    int sf;
}strClients;

typedef struct StrTopics
{
    char topic[51];
    int clsize;
    int aclsize;
    strClients *clients;
} strTopics;

int findTopic (strTopics* topics , int size, char *ttfind) // gaseste topicul x in vector
{
    int i;
    for(i = 0; i < size; i++)
    {
        if(strcmp(topics[i].topic, ttfind) == 0)
            return i;
    }
    return -1;
}

int findClient(strTopics topics , char* id) //returneaza 1 daca clientut este abonat la topicul x
{
    int i;
    for(i = 0; i < topics.aclsize; i++)
    {
        if(strcmp(id, topics.clients[i].id) == 0)
            return i;
    }
    return -1;
}

strTopics* reallocateTOPIC(strTopics* vect, int* size) //aloca pt vector un spatiu dublu fata de cel anterior
{   
    strTopics *forrealloc;
    int i,j;
    (*size) = (*size) * 2;
    forrealloc = (strTopics*) calloc((*size), sizeof(strTopics)); 
    for(i = 0; i < (*size)/2; i++)
    {
        strcpy(forrealloc[i].topic,vect[i].topic);
        forrealloc[i].clsize = vect[i].clsize;
        forrealloc[i].aclsize = vect[i].aclsize;
        forrealloc[i].clients = (strClients*) calloc(forrealloc[i].clsize, sizeof(strClients));
        for(j = 0; j < forrealloc[i].aclsize; j++)
        {
            strcpy(forrealloc[i].clients[j].id,vect[i].clients[j].id);
            forrealloc[i].clients[j].sf = vect[i].clients[j].sf;
        }
    }

    free(vect);
    return forrealloc;
}

strClients* reallocateCLIENTS (strClients* vect, int* size) //aloca pt vector un spatiu dublu fata de cel anterior
{
    strClients *forrealloc;
    int i;
    (*size) = (*size) * 2;
    forrealloc = (strClients*) calloc((*size), sizeof(strClients));
    for(i = 0; i < (*size)/2; i++)
    {
        strcpy(forrealloc[i].id,vect[i].id);
        forrealloc[i].sf = vect[i].sf;
    }


    return forrealloc;
}

int clientExist (strClients *vect, int size, char* id) //returneaza pozitia pe care gaseste clientul cu id-ul id
{
    int i;
    for(i = 0; i < size; i++)
    {
        if(strcmp(vect[i].id,id) == 0)
            return i;
    }
    return -1;
}