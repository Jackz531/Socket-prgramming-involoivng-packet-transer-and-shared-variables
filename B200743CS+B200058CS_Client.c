#include<arpa/inet.h>
#include<netdb.h>
#include<netinet/in.h>
#include<stdio.h>
#include<string.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<unistd.h>
#include<stdlib.h>
#include<time.h>
#include <pthread.h>

#define serverip "127.0.0.1"
#define pno "9998"
#define buflen 1040
#define time1 100
#define time2 150

typedef struct {
        int type;
        int seq_no;
        char payload[1024];
        unsigned int checksum;
	}packet;
	
	
	
void error(const char *msg)
	{
	  perror(msg);
	  exit(0);
	}


int find_checksum(packet p){
    int xor_arr = 0;
    for (int i = 0; i < 1024; i++) {
        xor_arr = xor_arr ^ p.payload[i];
    }
    return xor_arr;
}

void *send_packet2(void *arg) 
{
    struct sockaddr_in serv_addr;

    int portno=atoi(pno); 
    
    bzero((char*) &serv_addr,sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=INADDR_ANY;
	serv_addr.sin_port=htons(portno); 
	
    int sockfd,n,length;
    packet pk;
    char buffer[buflen];
    sockfd=*(int*)arg;
    length=sizeof(struct sockaddr_in);	
    struct hostent *server;

   
	if(sockfd<0)
	 error("error opening socket");
		 

	server=gethostbyname(serverip);
	
    	if (server == NULL)
    	{
        fprintf(stderr, "error,no such host");
    	}
    	
	int i=0;
	  while(1)
	    {
	 	i++;
		bzero(buffer,buflen);
		pk.type=2;
		pk.seq_no=i;
		//memset(pk.payload,rand()%1024,1024);
		
		 for(int i = 0;i < 1024;i++)
	           {
		 	pk.payload[i] = (rand()%2) + 48;
		   }
		pk.checksum=find_checksum(pk);
		
	   	usleep(time2*1000);
	     	n = sendto(sockfd, &pk,sizeof(pk),0,(struct sockaddr *)&serv_addr,length);
        	if (n < 0)
            	error("error in sending");
            	  
		    printf("TYPE: %d\nSEQUENCE NUMBER: %d\nPAYLOAD: %s\nCHECKSUM: %d \n\n",pk.type,pk.seq_no,pk.payload,pk.checksum);
		}
	    
	    close(sockfd);
  
    return NULL;
}


void *send_packet1(void *arg) 
{
    struct sockaddr_in serv_addr;

  bzero((char*) &serv_addr,sizeof(serv_addr));

    	int portno=atoi(pno); 
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=INADDR_ANY;
	serv_addr.sin_port=htons(portno); 
	
    int sockfd,n,length;
    packet pk;
    char buffer[buflen];
    sockfd=*(int*)arg;
    length=sizeof(struct sockaddr_in);	
    struct hostent *server;

   
	if(sockfd<0)
	 error("error opening socket");
		 

	server=gethostbyname(serverip);
	
    	if (server == NULL)
    	{
        fprintf(stderr, "error,no such host");
    	}
    	
	int i=0;
	  while(1)
	    {
	 	i++;
		pk.type=1;
		pk.seq_no=i;
		for(int i = 0;i < 1024;i++)
		 {
			pk.payload[i] =(rand()%2) + 48;;
		 }
		pk.checksum=find_checksum(pk);
	   	
		usleep(time1*1000);
	     	n = sendto(sockfd, &pk,sizeof(pk),0,(struct sockaddr *)&serv_addr,length);
	     	
        	if (n < 0)
            	error("error in sending");
            	
            	printf("TYPE: %d\nSEQUENCE NUMBER: %d\nPAYLOAD: %s\nCHECKSUM: %d \n\n",pk.type,pk.seq_no,pk.payload,pk.checksum);
		}
	    
	    close(sockfd);
  
    return NULL;
}

int main()
{
	int sockfd,portno,n,length;
	pthread_t thread1, thread2;
	
	 sockfd=socket(AF_INET,SOCK_DGRAM,0);
	
    	pthread_create(&thread1,NULL,send_packet1,(void *) &sockfd); 
	pthread_create(&thread2,NULL,send_packet2,(void *) &sockfd);
    
	pthread_join(thread1,NULL);
	pthread_join(thread2,NULL);
	return 0;
 }
