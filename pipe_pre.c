#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#define BUF_SIZE 100

void end(int sig)
{
		if(sig == SIGCHLD)
				puts("END~~~~~~~~~~~~~~~~~~~");
}

int main(int argc, char *argv[])
{
		int fds1[2],fds2[2];
		char str[30],str2[30],c[30]="c",temp[30]="c";
		int i,status,str_len;
		char buf[BUF_SIZE];
		pid_t pid;
		struct sigaction act;
		act.sa_handler = end;
		sigemptyset(&act.sa_mask);
		act.sa_flags=0;
		sigaction(SIGCHLD, &act, 0);

		pipe(fds1), pipe(fds2);
		pid = fork();

		if(pid == 0)
		{
				while(1)
				{
						str_len=read(fds1[0], buf, BUF_SIZE);
						buf[str_len] = 0;
						printf("child : %s\n", buf);
						strcpy(str,buf);
						strcat(c,str);
						write(fds2[1], c, BUF_SIZE);
						strcpy(c,temp);

						if(!strcmp(buf,"10"))
							exit(13);
			//			sleep(1);
				}
				if(str_len == 0)
						exit(1);
		}
		else
		{
				for(i=1;i<=10;i++)
				{
						snprintf(str2, BUF_SIZE, "%d", i);
						write(fds1[1], str2, BUF_SIZE);
					//	sleep(1);
						read(fds2[0], buf, BUF_SIZE);
						printf("parent : %s\n", buf);
				}
				wait(&status);
		}
		return 0;
}


