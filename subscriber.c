#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "helpers.h"

void usage(char *file)
{
	fprintf(stderr, "Usage: %s id_client server_address server_port\n", file);
	exit(0);
}

int main(int argc, char *argv[])
{
	int sockfd, n, ret;
	struct sockaddr_in serv_addr;
	char buffer[BUFLEN];
	char subs[12],topic[51];
	int sf;


	if (argc < 4) {
		usage(argv[0]);
	}

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	DIE(sockfd < 0, "socket");

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(atoi(argv[3]));
	ret = inet_aton(argv[2], &serv_addr.sin_addr);
	DIE(ret == 0, "inet_aton");

	ret = connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr));
	DIE(ret < 0, "connect");

	memset(buffer, 0, BUFLEN);
	strcpy (buffer,argv[1]);
	n = send(sockfd, buffer, strlen(buffer), 0);
	DIE(n < 0, "send id");

	fd_set read_fds;
	fd_set tmp_fds;
	int fdmax, i;

	FD_ZERO(&read_fds);
	FD_ZERO(&tmp_fds);

	FD_SET(sockfd, &read_fds);
	FD_SET(0, &read_fds);
	fdmax = sockfd;

	int quit = 0;

	while (1) 
	{
		tmp_fds = read_fds;

		ret = select(fdmax + 1, &tmp_fds, NULL, NULL, NULL);
		DIE(ret < 0, "select");

		for (i = 0; i <= fdmax; i++) 
		{
			if (FD_ISSET(i, &tmp_fds)) 
			{
				if (i == 0) 
				{
					// se citeste de la tastatura
					memset(buffer, 0, BUFLEN);
					fgets(buffer, BUFLEN - 1, stdin);

					if (strncmp(buffer, "exit", 4) == 0) 
					{
						quit = 1;
						break;
					}

					if(sscanf(buffer,"%s%s%d",subs,topic,&sf) == 3)
					{
						if((strcmp(subs,"subscribe") == 0) && (sf == 0 || sf == 1))
						{
							printf("%sd %s\n",subs,topic);
							// se trimite mesaj la server
							n = send(sockfd, buffer, strlen(buffer), 0);
							DIE(n < 0, "send");
						}
					}
					else if(sscanf(buffer,"%s%s",subs,topic) == 2)
					{
						if(strcmp(subs,"unsubscribe") == 0)
						{
							printf("%sd %s\n",subs,topic);
							// se trimite mesaj la server
							n = send(sockfd, buffer, strlen(buffer), 0);
							DIE(n < 0, "send");
						}
					}

				} 
				else 
				{
					memset(buffer, 0, BUFLEN);
					n = recv(i, buffer, sizeof(buffer), 0);
					DIE(n < 0, "recv");

					if (n == 0) {

						printf("Serverul a inchis conexiunea!\n");
						quit = 1;
						break;
					} 
					else 
					{
						printf("%s", buffer);
					}
				}
			}
		}

		if (quit) 
		{
			break;
		}
	}

	close(sockfd);

	return 0;
}
