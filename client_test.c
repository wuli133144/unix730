
#include <unistd.h>

#include<stdio.h>

#if 1

int main(){ 
   // init_main_proc();
   #if 1
      
      printf("pid=%d pgrd id=%d\n",getpid(),getpgrp());
       
      setpgid(getpid(),0);
      printf("modify=%d\n",getpgrp());

      printf("session id=%d\n",getsid(getpid()));

    return 1;
   #endif
   #if 0

         create_msg_queue();   
        //   if ((qid = msgget(0x223, IPC_CREAT |IPC_CREAT| 0666)) == -1) {
        //   check("msgget");
        //     }
        //     printf("%d",qid);
          sleep(5);
         destroy();
          
    
   #endif
}
#endif
#if 0

int main(int argc, char *argv[])    
{    
  
    
    int msgid = open_queue();  
     
     struct _msg msg;
     msg.mtype=1;
     int len=sizeof(struct _msg)-sizeof(long);

    // struct msgbuf *buf;    
    // buf = (struct msgbuf *)malloc(len + sizeof(msgbuf::mtype));    
    // buf->mtype = type;    
    if (msgsnd(msgid, &msg, len, IPC_NOWAIT) == -1)  //非阻塞   
        printf("23q4w5");  
      // send_queue(msgid,&msg);
}    

#endif
