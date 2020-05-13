#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <inttypes.h>
#include "helpers.h"
#include "tcpclients.c"
#include "topics.c"
#include "payload.c"

void usage(char *file)
{
	fprintf(stderr, "Usage: %s server_port\n", file);
	exit(0);
}

int main(int argc, char *argv[])
{
	int sockfdTCP, sockfdUDP, newsockfd, portno;
	char buffer[BUFLEN];
	struct sockaddr_in serv_addr, cli_addr;
	int n, i, ret, k, var;
	int quit = 0;
	socklen_t clilen;

	int tcpASize = 0, tcpSize = 30; // tcpSize este marimea maxima , iar tcpASize este marimea reala a vectorului tcp
	int topicASize = 0, topicSize = 10; // topicSize este marimea maxima , iar topicASize este marimea reala a vectorului topics 
	clientTCP *tcp;
	strTopics *topics;
	payload msg;
	uint32_t case0, case3a;
	uint16_t case1;
	uint8_t case3b;
	char sign; //folosit pt retinerea semnului in cazul clientilor udp
	unsigned char tip_date; // folosit pt retinerea tipului de date in cazul clientilor udp (deoarece msg.tip_date )
	char number[30];	//folosit pt retinerea numarului din case-uri
	char subs[12],topic[51];
	int sf;


	fd_set read_fds;	// multimea de citire folosita in select()
	fd_set tmp_fds;		// multime folosita temporar
	int fdmax;			// valoare maxima fd din multimea read_fds

	if (argc < 2) {
		usage(argv[0]);
	}

    tcp = (clientTCP *) calloc(tcpSize, sizeof(clientTCP)); //initializare vecotr id-uri, unde i = id si v[i] = socket 
	topics = (strTopics *) calloc (topicSize, sizeof(strTopics));
	
	// se goleste multimea de citire (read_fds) si multimea temporara (tmp_fds)
	FD_ZERO(&read_fds);
	FD_ZERO(&tmp_fds);

	/* deschidere socket TCP */
	sockfdTCP = socket(AF_INET, SOCK_STREAM, 0);
	DIE(sockfdTCP < 0, "socketTCP");

	/* deschidere socket UDP*/
	sockfdUDP = socket(AF_INET, SOCK_DGRAM, 0);
	DIE(sockfdUDP < 0, "socketUDP");

	portno = atoi(argv[1]);
	DIE(portno == 0, "portno");

	memset((char *) &serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(portno);
	serv_addr.sin_addr.s_addr = INADDR_ANY;

	/* legare proprietăți de socket TCP */
	ret = bind(sockfdTCP, (struct sockaddr *) &serv_addr, sizeof(struct sockaddr));
	DIE(ret < 0, "bindTCP");

	/* legare proprietăți de socket UDP*/
	ret = bind(sockfdUDP, (struct sockaddr *) &serv_addr, sizeof(struct sockaddr));
	DIE(ret < 0, "bindUPD");

	ret = listen(sockfdTCP, MAX_CLIENTS);
	DIE(ret < 0, "listenTCP");

	// se adauga noul file deor (socketul pe care se asculta conexiuni) in multimea read_fds
	FD_SET(sockfdTCP, &read_fds);
	FD_SET(sockfdUDP, &read_fds);
	FD_SET(0, &read_fds);
	fdmax = sockfdTCP > sockfdUDP ? sockfdTCP : sockfdUDP;

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
					
				}
				else if (i == sockfdTCP) 
				{
					// a venit o cerere de conexiune pe socketul inactiv (cel cu listen),
					// pe care serverul o accepta
					clilen = sizeof(cli_addr);
					newsockfd = accept(sockfdTCP, (struct sockaddr *) &cli_addr, &clilen);
					DIE(newsockfd < 0, "accept");

					// se adauga noul socket intors de accept() la multimea deorilor de citire
					FD_SET(newsockfd, &read_fds);
					if (newsockfd > fdmax) 
					{
						fdmax = newsockfd;
					}
										
					//se primeste in buffer id-ul clientului
					memset(buffer, 0, BUFLEN);
					n = recv(newsockfd, buffer, sizeof(buffer), 0);
					DIE(n < 0, "recv");

					n = findID(tcp,tcpASize,buffer);
					if(n == -1)
					{
						if( tcpASize == tcpSize - 1)
							tcp = reallocateTCP(tcp, &tcpSize);

						strcpy(tcp[tcpASize].id,buffer);
						tcp[tcpASize].socket = newsockfd;
						tcp[tcpASize].online = 1;
						tcp[tcpASize].offmsgSize = 15;
						tcp[tcpASize].offmsgASize = 0;
						tcp[tcpASize].offmsg = (char**) calloc(tcp[tcpASize].offmsgSize,sizeof(char*)); 
						for(k = 0; k < tcp[tcpASize].offmsgSize; k++)
							tcp[tcpASize].offmsg[k] = (char*) calloc(BUFLEN,sizeof(char));

						printf("New client %s connected from %s:%d\n",
							tcp[tcpASize++].id,inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));
					}
					else
					{
						tcp[n].socket = newsockfd;
						tcp[n].online = 1;	
						for(k = 0; k < tcp[n].offmsgASize; k++)  //trimiterea mesajelor primite "while offline"
						{
							send(tcp[n].socket,tcp[n].offmsg[k], BUFLEN*sizeof(char), 0);
							memset(tcp[n].offmsg[k],0,BUFLEN);
						}
							
						tcp[n].offmsgASize=0;

						printf("Client %s connected from %s:%d\n",
							tcp[n].id,inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));
					}
					
					
					
				} 
				
				else if(i == sockfdUDP)
				{
					memset(buffer, 0, BUFLEN);
					n = recvfrom(i, buffer, sizeof(buffer), 0,  (struct sockaddr *) &cli_addr, &clilen);
					DIE (n < 0, "recvUDP");
					memset(&msg,0,sizeof(msg));
					memcpy(&msg,buffer,sizeof(buffer));
					msg.tip_date+=48;

					switch(msg.tip_date)
					{
						case('0'):
							memset(number,0,sizeof(number));
							sign = msg.continut[0] + 48;
							memcpy(&case0, msg.continut + 1, sizeof(uint32_t));
							
							if(sign == '0')
								sprintf(number,"%" PRIu32, htonl(case0));
							else
								sprintf(number,"-%" PRIu32, htonl(case0));
							
							memset(msg.continut,0,sizeof(msg.continut));
							strcpy(msg.continut,number);
							break;

						case('1'): 
							memset(number,0,sizeof(number));
							memcpy(&case1, msg.continut, sizeof(uint16_t));
							sprintf(number,"%" PRIu16, htons(case1));
							strcpy(number, insertDot(number,2));

							memset(msg.continut,0,sizeof(msg.continut));
							strcpy(msg.continut,number);
							break;

						case('2'):
							memset(number,0,sizeof(number));
							sign = msg.continut[0] + 48;
							memcpy(&case3a, msg.continut + 1, sizeof(uint32_t));
							memcpy(&case3b, msg.continut + 1 + sizeof(uint32_t), sizeof(uint8_t));

							if(sign == '0')
								sprintf(number,"%" PRIu32,htonl(case3a));
							else
								sprintf(number,"-%" PRIu32,htonl(case3a));
							strcpy(number, insertDot(number,case3b));	
							
							memset(msg.continut,0,sizeof(msg.continut));
							strcpy(msg.continut,number);
							break;
					}
					tip_date = msg.tip_date;
					msg.topic[50]='\0'; // face ca msg.tip_date sa fie \0, dar este necesar pt topicurile cu fix 50 de caractere
					memset(buffer, 0, BUFLEN);
					if((n = findTopic(topics,topicASize,msg.topic)) != -1) //daca topicul a fost creat
					{
						sprintf(buffer,"%s:%d - %s - ", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port), msg.topic);
						switch(tip_date)
						{
							case('0'):
								strcat(buffer,"INT - ");
								break;
							case('1'):
								strcat(buffer,"SHORT_REAL - ");
								break;
							case('2'):
								strcat(buffer,"FLOAT - ");
								break;
							case('3'):
								strcat(buffer,"STRING - ");
								break;
						}
						strcat(buffer,msg.continut);
						strcat(buffer,"\n");

						for(k = 0; k < topics[n].aclsize; k++)
						{	
							var = findID(tcp,tcpASize,topics[n].clients[k].id);
							if(tcp[var].online == 1)
								send(tcp[var].socket, buffer, sizeof(buffer), 0);
								
							else
							{
								if(topics[n].clients[k].sf == 1)
								{
									if(tcp[var].offmsgASize == tcp[var].offmsgSize - 1)
										tcp[var].offmsg = reallocateOFFMSG(tcp[var].offmsg , &tcp[var].offmsgSize);
									strcpy(tcp[var].offmsg[tcp[var].offmsgASize],buffer);
									tcp[var].offmsgASize++;
								}
							}
							
						}
					}	
					memset(&msg,0,sizeof(msg));
				}
				else 
				{
					// s-au primit date pe unul din socketii de client,
					// asa ca serverul trebuie sa le receptioneze
					memset(buffer, 0, BUFLEN);
					n = recv(i, buffer, sizeof(buffer), 0);
					DIE(n < 0, "recv");

					if (n == 0)
					{
						// conexiunea s-a inchis
						n = findSocket (tcp,tcpSize,i);
						tcp[n].socket = 0;
						tcp[n].online = 0;
						printf("Client %s disconnected \n", tcp[n].id);
						close(i);

						// se scoate din multimea de citire socketul inchis
						FD_CLR(i, &read_fds);
					} 
					
					else 
					{
						sscanf(buffer,"%s%s%d",subs,topic,&sf);

						if(strcmp(subs,"subscribe") == 0)
						{
							if((n = findTopic(topics,topicASize,topic)) != -1)		//topicul exista
							{
								if((var = clientExist(topics[n].clients,topics[n].aclsize,tcp[findSocket(tcp,tcpASize,i)].id)) != -1)
									topics[n].clients[var].sf = sf;
								
								else
								{
									if(topics[n].aclsize == topics[n].clsize - 1)
									topics[n].clients = reallocateCLIENTS(topics[n].clients, &topics[n].clsize);
								
									strcpy(topics[n].clients[topics[n].aclsize].id, tcp[findSocket(tcp,tcpASize,i)].id);
									topics[n].clients[topics[n].aclsize++].sf = sf;
								}
								
								
							}
							else	//topicul nu exista inca
							{
								if(topicASize == topicSize - 1)
									topics = reallocateTOPIC(topics, &topicSize);

								strcpy(topics[topicASize].topic,topic);
								topics[topicASize].aclsize = 1;
								topics[topicASize].clsize = tcpSize/2;
								topics[topicASize].clients = (strClients*) calloc(topics[topicASize].clsize,sizeof(strClients));
								strcpy(topics[topicASize].clients[0].id, tcp[findSocket(tcp,tcpASize,i)].id);
								topics[topicASize].clients[0].sf = sf;
								topicASize++;	
							}					
							
						}
						else	 //strcmp(subs,"unsubscribe") == 0
						{
							if((n = findTopic(topics,topicASize,topic)) != -1)
							{
								var = findSocket(tcp,tcpASize,i);
								if((k = findClient(topics[n],tcp[var].id)) != -1) // pozitia din vectorul de clienti la care se afla clientul
								{
									strcpy(topics[n].clients[k].id,topics[n].clients[topics[n].aclsize - 1].id);
									topics[n].clients[k].sf = topics[n].clients[topics[n].aclsize - 1].sf;

									strcpy(topics[n].clients[topics[n].aclsize - 1].id, "\0");
									topics[n].clients[topics[n].aclsize - 1].sf = 0;
									
									topics[n].aclsize--;
								}
							}
						}
					}
				}
			}
		}
		if (quit) 
		{
			break;
		}
	}

	close(sockfdTCP);
	close(sockfdUDP);

	return 0;
}