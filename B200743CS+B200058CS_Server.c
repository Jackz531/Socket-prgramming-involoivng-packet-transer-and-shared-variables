#include<arpa/inet.h>
#include<netinet/in.h>
#include<unistd.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<string.h>
#include<stdio.h>
#include<netdb.h>
#include<stdlib.h>
#include<time.h>
#include <pthread.h>
#include<time.h>

#define serverip "127.0.0.1"
#define pno "9998"
#define buflen 1040
#define time1 100
#define time2 150

pthread_t t1,t2,t3,t4;
	
void error(const char * msg)
{
	perror(msg);
	exit(0);
}


typedef struct 
	{
         int type;
         int seq_no;
         char payload[1024];
         int checksum;
	}packet;

packet buffer1[1024],buffer2[1024];

int buffer_size1=0,buffer_size2=0;

pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;

int find_checksum(packet p){
    int xor_arr = 0;
    for (int i = 0; i < 1024; i++) {
        xor_arr = xor_arr ^ p.payload[i];
    }
    return xor_arr;
}
 
 int count1=0;
 int count2=0;
 
 
 
 void *process_type2(void *arg)
  {
  	while(1)
	{
		pthread_mutex_lock(&mutex);
		while(buffer_size2 > 0)
		{
	        printf("TYPE: %d\nSEQUENCE NUMBER: %d\nPAYLOAD: %s\nCHECKSUM: %d\n\n",buffer2[0].type,buffer2[0].seq_no,buffer2[0].payload,buffer2[0].checksum);	
			memmove(&buffer2[0], &buffer2[1], (buffer_size2 - 1) * sizeof(packet));
			buffer_size2--;
			count2++;
			
		}
 		
 		pthread_mutex_unlock(&mutex);
 	}
  }
 
  void *process_type1(void *arg)
  {
  	
	while(1)
	{
		pthread_mutex_lock(&mutex);
		while(buffer_size1 > 0)
		{
			
			
     printf("TYPE: %d\nSEQUENCE NUMBER: %d\nPAYLOAD: %s\nCHECKSUM:%d\n\n",buffer1[0].type,buffer1[0].seq_no,buffer1[0].payload,buffer1[0].checksum);	
			memmove(&buffer1[0], &buffer1[1], (buffer_size1 - 1) * sizeof(packet));
			buffer_size1--;
			count1++;
			
		}
 		
 		pthread_mutex_unlock(&mutex);
 	}
  }
  
  void *print_no(void *arg)
  {
  	while(1)
	{
	pthread_mutex_lock(&mutex);
  	printf("The number of packets of type1 recieved is %d and type2 recieved is %d in the interval of 300ms\n",count1,count2);
  	pthread_mutex_unlock(&mutex);
  	usleep(300000);	
  	}
  }
  

 void *check_error(void *arg)
 {	
 
 	int sockfd,n,length;
 	struct sockaddr_in serv_addr;
	int portno=atoi(pno);
	
	    bzero((char*) &serv_addr,sizeof(serv_addr));
	    serv_addr.sin_family=AF_INET;
    	    serv_addr.sin_addr.s_addr=inet_addr(serverip);;
    	    serv_addr.sin_port=htons(portno);
    	    
    	     sockfd=*(int*)arg;
    	     
    	     if(bind(sockfd,(struct sockaddr*)&serv_addr,sizeof(serv_addr))<0)
		error("error in binding");
		
		int flen=sizeof(serv_addr);
	
		while(1)
		{
		  packet pk;
		  n = recvfrom(sockfd, &pk,sizeof(pk),0,(struct sockaddr *)&serv_addr,&flen);
		  
		  if(pk.checksum!=find_checksum(pk))
		   {
		  	printf("packet of type %d and sequence number : %d has issue\n",pk.type,pk.seq_no);
		   }
		 else
		 	{
		 	 pthread_mutex_lock(&mutex);
		 	 if(pk.type==1)
		 	 	buffer1[buffer_size1++]=pk;
		 	  else
		 		buffer2[buffer_size2++]=pk;
		 		
		 	 pthread_mutex_unlock(&mutex);
		 	}
		  
		  
		
		 if(n<0)
		 error("error in recieving");
		
	       }
		close(sockfd);
 
 
 }


int main()
{
	int sockfd,n,length,portno;
	char buffer[1040];
	
	sockfd=socket(AF_INET,SOCK_DGRAM,0);
	
	if(sockfd<0)
	{
		error("error creating socket");
	}
	pthread_create(&t1,NULL,check_error,(void *) &sockfd);
	pthread_create(&t2,NULL,process_type1,(void *)NULL);
	pthread_create(&t3,NULL,process_type2,(void *)NULL);	  		
	pthread_create(&t4,NULL,print_no,(void *)NULL);
	
	pthread_join(t1,NULL);
	pthread_join(t2,NULL);
	pthread_join(t3,NULL);
	pthread_join(t4,NULL);
	}
