#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>

#define BUF_SIZE 100
#define MAX_CLNT 26
#define ID_SIZE 10
#define ARR_CNT 5

#define DEBUG
typedef struct {
  	char fd;
	char *from;
	char *to;
	char *msg;
	int len;
}MSG_INFO;

typedef struct {
	int index;;
	int fd;
    char ip[20];
	char id[ID_SIZE];
	char pw[ID_SIZE];
}CLIENT_INFO;

void * clnt_connection(void * arg);
void send_msg(MSG_INFO * msg_info, CLIENT_INFO * first_client_info);
void error_handling(char * msg);
void log_file(char * msgstr);

int clnt_cnt=0;
pthread_mutex_t mutx;

int main(int argc, char *argv[])
{
	int serv_sock, clnt_sock;
	struct sockaddr_in serv_adr, clnt_adr;
	int clnt_adr_sz;
	int sock_option  = 1;
	pthread_t t_id[MAX_CLNT] = {0};
	int str_len = 0;
	int i;
	char idpasswd[(ID_SIZE*2)+3];
	char *pToken;
	char *pArray[ARR_CNT]={0};
	char msg[BUF_SIZE];

	CLIENT_INFO client_info[MAX_CLNT] = {{0,0,"","KSHARD","PASSWD"}, \
			 {0,0,"","2","PASSWD"},  {0,0,"","3","PASSWD"}, \
			 {0,0,"","4","PASSWD"},  {0,0,"","5","PASSWD"}, \
			 {0,0,"","6","PASSWD"},  {0,0,"","7","PASSWD"}, \
			 {0,0,"","8","PASSWD"},  {0,0,"","9","PASSWD"}, \
			 {0,0,"","10","PASSWD"},  {0,0,"","11","PASSWD"}, \
			 {0,0,"","12","PASSWD"},  {0,0,"","13","PASSWD"}, \
			 {0,0,"","14","PASSWD"},  {0,0,"","15","PASSWD"}, \
			 {0,0,"","16","PASSWD"},  {0,0,"","17","PASSWD"}, \
			 {0,0,"","18","PASSWD"},  {0,0,"","19","PASSWD"}, \
			 {0,0,"","20","PASSWD"},  {0,0,"","21","PASSWD"}, \
			 {0,0,"","22","PASSWD"},  {0,0,"","23","PASSWD"}, \
			 {0,0,"","24","PASSWD"},  {0,0,"","25","PASSWD"}, \
			 {0,0,"","26","PASSWD"}};

	if(argc != 2) {
		printf("Usage : %s <port>\n",argv[0]);
		exit(1);
	}
	fputs("IoT Server Start!!\n",stdout);

	if(pthread_mutex_init(&mutx, NULL))
		error_handling("mutex init error");

	serv_sock = socket(PF_INET, SOCK_STREAM, 0);

	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family=AF_INET;
	serv_adr.sin_addr.s_addr=htonl(INADDR_ANY);
	serv_adr.sin_port=htons(atoi(argv[1]));

	 setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, (void*)&sock_option, sizeof(sock_option));
	if(bind(serv_sock, (struct sockaddr *)&serv_adr, sizeof(serv_adr))==-1)
		error_handling("bind() error");

	if(listen(serv_sock, 5) == -1)
		error_handling("listen() error");

	while(1) {
		clnt_adr_sz = sizeof(clnt_adr);
		clnt_sock = accept(serv_sock, (struct sockaddr *)&clnt_adr, &clnt_adr_sz);
		if(clnt_cnt == MAX_CLNT)
       	{
       		printf("socket full\n");
           	close(clnt_sock);
           	continue;
		}
		else if(clnt_sock < 0)
		{
       		printf("fail to call accept()\n");
          	continue;
		}

		str_len = read(clnt_sock, idpasswd, sizeof(idpasswd));
		idpasswd[str_len] = '\0';

		if(str_len > 0)
		{
			i=0;
			pToken = strtok(idpasswd,"[:]");
	
			while(pToken != NULL)
			{
				pArray[i] =  pToken;
        		if(i++ >= ARR_CNT)
					break;	
				pToken = strtok(NULL,"[:]");
			}
			for(i=0;i<MAX_CLNT;i++)
			{
				if(!strcmp(client_info[i].id,pArray[0]))
				{
					if(client_info[i].fd != 0)
					{
						sprintf(msg,"[%s] Already logged!\n",pArray[0]);
						write(clnt_sock, msg,strlen(msg));
						log_file(msg);
						close(clnt_sock);

						close(client_info[i].fd);
						pthread_mutex_lock(&mutx);
						client_info[i].fd = 0;
						clnt_cnt--;
						pthread_mutex_unlock(&mutx);

						break;

					}
					if(!strcmp(client_info[i].pw,pArray[1])) 
					{

						pthread_mutex_lock(&mutx);
						client_info[i].index = i; 
						client_info[i].fd = clnt_sock; 
						strcpy(client_info[i].ip,inet_ntoa(clnt_adr.sin_addr));
						pthread_mutex_unlock(&mutx);

						clnt_cnt++;
						sprintf(msg,"[%s] New connected! (ip:%s,fd:%d,sockcnt:%d)\n",pArray[0],inet_ntoa(clnt_adr.sin_addr),clnt_sock,clnt_cnt);
						log_file(msg);
						write(clnt_sock, msg,strlen(msg));

  						pthread_create(t_id+1, NULL, clnt_connection, (void *)(client_info + i));
						break;
					}
				}
			}
			if(i == MAX_CLNT)
			{
				sprintf(msg,"[%s] Authentication Error!\n",pArray[0]);
				write(clnt_sock, msg,strlen(msg));
				log_file(msg);
				close(clnt_sock);
			}
		}
		else 
			close(clnt_sock);

	}
	return 0;
}

void * clnt_connection(void *arg)
{
   	CLIENT_INFO * client_info = (CLIENT_INFO *)arg;
	int str_len = 0;
	int index = client_info->index;
	char msg[BUF_SIZE];
  	char to_msg[MAX_CLNT*ID_SIZE+1];
	int i=0;
	char *pToken;
   	char *pArray[ARR_CNT]={0};
	char strBuff[100]={0};

	MSG_INFO msg_info;
	CLIENT_INFO  * first_client_info;

	first_client_info = (CLIENT_INFO *)((void *)client_info - (void *)( sizeof(CLIENT_INFO) * index ));
	while(1)
	{
		memset(msg,0x0,sizeof(msg));
		str_len = read(client_info->fd, msg, sizeof(msg)-1); 
		if(str_len <= 0)
			break;

		msg[str_len] = '\0';
		pToken = strtok(msg,"[:]");
		i = 0; 
		while(pToken != NULL)
		{
			pArray[i] =  pToken;
			if(i++ >= ARR_CNT)
				break;	
			pToken = strtok(NULL,"[:]");
		}

		msg_info.fd = client_info->fd;
		msg_info.from = client_info->id;
		msg_info.to = pArray[0];
		sprintf(to_msg,"[%s]%s",msg_info.from,pArray[1]);
		msg_info.msg = to_msg;
		msg_info.len = strlen(to_msg);

		sprintf(strBuff,"msg : [%s->%s] %s",msg_info.from,msg_info.to,pArray[1]);
		log_file(strBuff);
		send_msg(&msg_info, first_client_info);
	}

	close(client_info->fd);

	sprintf(strBuff,"Disconnect ID:%s (ip:%s,fd:%d,sockcnt:%d)\n",client_info->id,client_info->ip,client_info->fd,clnt_cnt-1);
	log_file(strBuff);

	pthread_mutex_lock(&mutx);
	clnt_cnt--;
	client_info->fd = 0;
	pthread_mutex_unlock(&mutx);

	return 0;
}

void send_msg(MSG_INFO * msg_info, CLIENT_INFO * first_client_info)
{
	int i=0;

   	//[ID]MSG

	pthread_mutex_lock(&mutx);

	if(!strcmp(msg_info->to,"ALLMSG"))
	{
		for(i=0;i<MAX_CLNT;i++)
			if((first_client_info+i)->fd != 0)	
     			write((first_client_info+i)->fd, msg_info->msg, msg_info->len);
	}
	else if(!strcmp(msg_info->to,"IDLIST"))
	{
		msg_info->msg[strlen(msg_info->msg) - 1] = '\0';

		for(i=0;i<MAX_CLNT;i++)
		{
			if((first_client_info+i)->fd != 0)	
			{
				strcat(msg_info->msg,(first_client_info+i)->id);
				strcat(msg_info->msg," ");
			}
		}
		strcat(msg_info->msg,"\n");
    	write(msg_info->fd, msg_info->msg, strlen(msg_info->msg));
	}
	else
		for(i=0;i<MAX_CLNT;i++)
			if((first_client_info+i)->fd != 0)	
				if(!strcmp(msg_info->to,(first_client_info+i)->id))
     				write((first_client_info+i)->fd, msg_info->msg, msg_info->len);
	pthread_mutex_unlock(&mutx);
}

void error_handling(char *msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}

void log_file(char * msgstr)
{
	char date[11];
	char fname[80];
	char str[100];
	int outfd;
	struct tm *t;
	time_t tt;
	DIR * dirp;
	tt=time(NULL);
	t=localtime(&tt);
#ifdef DEBUG
	fputs(msgstr,stdout);
#endif
	sprintf(date,"%d-%02d-%02d",t->tm_year+1900,t->tm_mon+1,t->tm_mday);
	getcwd(str,sizeof(str));
	strcat(str,"/log.d");
	dirp = opendir(str);
	if(dirp == NULL)
		mkdir(str,0755);
	sprintf(fname,"%s/%s.txt",str,date);

	outfd = open(fname, O_RDWR | O_CREAT,0644); 
	if(outfd > 0 )
	{
		lseek(outfd,(off_t)0,SEEK_END);
		sprintf(str,"%02d:%02d:%02d %s",t->tm_hour,t->tm_min,t->tm_sec,msgstr);
		write(outfd,str,strlen(str));
		close(outfd);
	}
	else
		perror("open:");
}

