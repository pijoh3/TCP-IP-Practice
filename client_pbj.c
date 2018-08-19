#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>

#define BUF_SIZE 100

void error_handling(char * message);

int main(int argc, char *argv[])
{
		int sock;
		char message[BUF_SIZE];
		int str_len,file_read_len=0,fd;
		struct sockaddr_in serv_adr;

		if(argc !=4)
		{
				printf("Usage : %s <IP> <port> <file>\n", argv[0]);
				exit(1);
		}

		sock=socket(PF_INET, SOCK_STREAM, 0);
		if(sock == -1)
				error_handling("socket() error");

		memset(&serv_adr, 0, sizeof(serv_adr));
		serv_adr.sin_family=AF_INET;
		serv_adr.sin_addr.s_addr=inet_addr(argv[1]);
		serv_adr.sin_port=htons(atoi(argv[2]));

		if(connect(sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr)) == -1)
				error_handling("connect() error!");
		fd=open(argv[3], O_RDWR);	
		if(fd == -1)
		{
				error_handling("<file> file open error");
				perror("open");
		}

	    while((file_read_len=read(fd, message, BUF_SIZE)) != 0)
		{
				printf("read %d\n",file_read_len);
				if((file_read_len ==-1))
						error_handling("read error");
				write(sock, message, file_read_len);
		}
		close(sock);
		return 0;
}

void error_handling(char *message)
{
fputs(message, stderr);
		fputc('\n', stderr);
		exit(1);
}


