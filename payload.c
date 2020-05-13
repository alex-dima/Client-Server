#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct Payload
{
    char topic[50];
    unsigned char tip_date;
    char continut[1500];
} payload;

char* insertDot(char *string, int dep) //adauga zerouri si/sau pune punctul in locul corespunzator
{
    int i,size;
    if(string[0] == '-')
        size = strlen(string + 1);
    else
        size = strlen(string);
    if(dep < size)
    {
        if(dep == 0)
            return string;

        if(string[0] == '-')
            i = size + 2;
        else
            i = size + 1;

        while(dep >= 0)
        {
            string[i] = string[i-1];
            i--;
            dep--;
        }
        string[i]='.';
        return string;
    }
    else
    {
        char *string2;
        string2 = (char*)calloc(strlen(string) +10, sizeof(char));
        dep = dep - size;  // numarul de zerouri - 1 din fata lui string 
        if(string[0] == '-')
            strcpy(string2,"-0.");
        else
            strcpy(string2,"0.");
        while(dep > 0)
        {
            strcat(string2, "0");
            dep--;
        }
        if(string[0] == '-')
            strcat(string2,string + 1);
        else
            strcat(string2,string);

        return string2;
    }
    
}