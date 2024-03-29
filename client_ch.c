#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

int whoami = -1;
void *recvmg(void *sock)
{
	int their_sock = *((int *)sock);
	char msg[500];
	int len;
	while((len = recv(their_sock,msg,500,0)) > 0) {
		msg[len] = '\0';
		if(strncmp(msg,"@@@",3) == 0) 
		{
			whoami = msg[3] + '0';	
			printf("you log in as guest%d\n",whoami -'0' - '0');
		}
		else if(strncmp(msg,"!!@@!!",6) == 0)
		{
			whoami = -1;
			printf("you have log out\n");
		}
		else if(strncmp(msg,"&&&&&",5) == 0)
		{
			int opp = msg[5] - '0';
			printf("guest%d invite you to play\n",opp);
		}
		else if(strncmp(msg,"_JOIN",5) == 0)
		{
			printf("joing the game\n");
			char command[50] = "./game_client 127.0.0.1 8080";
			system(command);
		}
		else
			fputs(msg,stdout);
		memset(msg,'\0',sizeof(msg));
	}
}
int main(int argc, char *argv[])
{
	struct sockaddr_in their_addr;
	int my_sock;
	int their_sock;
	int their_addr_size;
	int portno;
	pthread_t sendt,recvt;
	char msg[500];
	char username[100];
	char res[600];
	char ip[INET_ADDRSTRLEN];
	int len;

	if(argc > 3) {
		printf("too many arguments");
		exit(1);
	}
	portno = atoi(argv[2]);
	strcpy(username,argv[1]);
	my_sock = socket(AF_INET,SOCK_STREAM,0);
	memset(their_addr.sin_zero,'\0',sizeof(their_addr.sin_zero));
	their_addr.sin_family = AF_INET;
	their_addr.sin_port = htons(portno);
	their_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	if(connect(my_sock,(struct sockaddr *)&their_addr,sizeof(their_addr)) < 0) {
		perror("connection not esatablished");
		exit(1);
	}
	inet_ntop(AF_INET, (struct sockaddr *)&their_addr, ip, INET_ADDRSTRLEN);
	printf("connected to %s, start chatting\n",ip);
	pthread_create(&recvt,NULL,recvmg,&my_sock);
	while(fgets(msg,500,stdin) > 0) {
		if(whoami == -1)
			strcat(res,":");
		else
		{
			char word = whoami - '0';
			res[0]= word;

		}		
		strcat(res,msg);
		len = write(my_sock,res,strlen(res));
		if(len < 0) {
			perror("message not sent");
			exit(1);
		}
		memset(msg,'\0',sizeof(msg));
		memset(res,'\0',sizeof(res));
	}
	pthread_join(recvt,NULL);
	close(my_sock);

}
