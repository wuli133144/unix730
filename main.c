


#include "pthread_pool.h"
#include "sock.h"


void * hand(void *arg){
     printf("hello world\n");
     return NULL;
}


int main(int argc,char **argv){
      
      init_pthreadpool();
      //append_task(hand,(void *)&a);
      int clientfd;
      struct sockaddr sock;
      socklen_t len=sizeof(struct sockaddr);
      int listenfd=open_listenfd("127.0.0.1","8080");
       
       while(1){

              clientfd=Accept(listenfd,&sock,&len);
              insert_task(&clientfd);
              close(clientfd);
       }


       close(listenfd);
       destroy_pool();
       return 0;
}
