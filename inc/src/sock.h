

#ifndef      __SOCKET__H

#define      __SOCKET__H

#include<sys/socket.h>
#include<stdarg.h>
#include<stddef.h>
#include<signal.h>
#include<sys/types.h>
#include<netdb.h>
#include<arpa/inet.h>
#include<fcntl.h>
#include <sys/stat.h>
#include <time.h>

#include <sys/uio.h>
#include <sys/un.h>
#include "utils.h" 



#include"unix_error.h"

 
#define LISTEN_NUM  100


//设置file status
int Setnoblock(int fd,int status){
    int rval=fcntl(fd,F_GETFL,0);
    if(rval<0){
        unix_error("fcntl error");
    }
    rval|=status;
    rval=fcntl(fd,F_SETFL,rval);
     if(rval<0){
        unix_error("fcntl error");
    }
    return 0;
    
}  

/*@read_s start@*/
 size_t Read_s(int fd,void *buf,ssize_t sz){
     size_t nleft;
     ssize_t nread;
     nleft=sz;
     
     while(nleft>0){
     
         if((nread=read(fd,buf,nleft))<0){
             if(nleft==0){
                 return -1;
             }else{
                 break;
             }
         }else if(nread==0){
             break;//eof
         }else{
               nleft-=nread;
               buf+=nread;
         }
     }
      return sz-nleft;
 }

/*@read end@*/


/*@write start@*/

   size_t Write_s(int fd,void *buf,ssize_t sz){
            size_t nwrite;
            ssize_t nleft;
            nleft=sz;
            while(nleft>0){

                  if((nwrite=write(fd,buf,nleft))<0){
                       if(nleft==0){
                           return -1;
                       }else
                       break;
                  }else if(nwrite ==0){
                          break;//eof
                  }else{
                        nleft-=nwrite;
                        buf+=nwrite;
                  }
            }
       return sz-nleft;
   }


/*@write end@*/



/*@socket start@*/
int Socket(int domain,int type,int protocol){
    int rval;
    if((rval=socket(AF_INET,SOCK_STREAM,0))<0){
        unix_error("socket");
    }
    return rval;
}

/*@socket end@*/


/*@connect start@*/

int Connnect(int clientfd, struct sockaddr *addr,int len){
    int rval;
    if((rval=connect(clientfd,addr,len))<0){
        unix_error("connect");
    }
    return rval;
}
/*@connect end@*/


/*@ bind start@*/
int Bind(int sockfd,const struct sockaddr*addr,socklen_t len){
     int rval;
     if((rval=bind(sockfd,addr,len))<0){
         unix_error("bind");
     }
     return rval;
}
/*@bind end@*/

/*@listen start@*/
int Listen(int listenfd,int num){
    int rval;
    if((rval=listen(listenfd,num))<0){
        unix_error("listen");
    }
    return rval;
}
/*@listen end@*/

/*accept start*/
int Accept(int listenfd,struct sockaddr *addr,socklen_t *len){
    int rval;
    if((rval=accept(listenfd,addr,len))<0){
          unix_error("accept");
    }
   
    return rval;
}
/*accept end*/

/*@getaddrinfo handler@*/
int Getaddrinfo(const char *hostname,\
const char *service,                 \
const struct addrinfo *hints,        \
struct addrinfo**info)
{      
       int rval;
       if((rval=getaddrinfo(hostname,service,hints,info))<0){
           unix_error("getaddrinfo");
       }
     return rval;
}
/*@getaddrinfo handler@*/


/*@getnameinfo @*/
int Getnameinfo(const struct sockaddr *restrict sa, socklen_t salen, \
       char *restrict node, socklen_t nodelen, char *restrict service, \
       socklen_t servicelen, int flags){
            int rval;
            if((rval=getnameinfo(sa,salen,node,nodelen,service,servicelen,flags))<0){
                unix_error("getnameinfo");
            }
            return rval;
       }
/*@getnameinfo end@*/

/*@freeaddrinfo@*/
void Freeaddrinfo(struct addrinfo *p){
      freeaddrinfo(p);
}
/*@freeaddrinfo@*/

/*Setsockopt start*/
int Setsockopt(int sock, int level, int optname, const void *optval, socklen_t optlen){
    int rval;
    if((rval=setsockopt(sock,level,optname,optval,optlen))<0){
        unix_error("setsockopt");
    }
    return rval;
}
/*Setsockopt*/

/*@open_clientfd start@*/
int open_clientfd(const char *hostname,char* port){
   
      int clientfd;
      struct addrinfo hints,*listp,*p;
      memset(&hints,0,sizeof(struct addrinfo));
      //hints.ai_family=AF_INET;
      hints.ai_socktype=SOCK_STREAM;
      hints.ai_flags=AI_NUMERICSERV;
      hints.ai_flags|=AI_ADDRCONFIG;
      Getaddrinfo(hostname,port,&hints,&listp);

      for(p=listp;p;p=p->ai_next){

          if((clientfd=socket(p->ai_family,p->ai_socktype,p->ai_protocol))<0){
              continue;
          }
          if((connect(clientfd,p->ai_addr,p->ai_addrlen))!=-1){
                    break;//success
          }
          close(clientfd);//failed
      }
      Freeaddrinfo(listp);
      if(p==NULL){
          return -1;//failed
      }
     return clientfd;

}
/*open_clientfd end*/

/*open_listenfd start*/
int open_listenfd(const char *hostname,char *port){
      
  struct addrinfo hints,*listp,*p;
  int listenfd,optval;
  memset(&hints,0,sizeof(struct addrinfo));
   
   hints.ai_socktype=SOCK_STREAM;
   hints.ai_flags=AI_NUMERICSERV;
   hints.ai_flags=AI_PASSIVE|AI_ADDRCONFIG;
   
   Getaddrinfo(hostname,port,&hints,&listp);

   for(p=listp;p;p=p->ai_next){
          if((listenfd=(socket(p->ai_family,p->ai_socktype,p->ai_protocol)))<0){
              continue;
          }
          Setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,(const void *)&optval,sizeof(int));
         
          if((Bind(listenfd,p->ai_addr,p->ai_addrlen))==0){
              break;
          }
          close(listenfd);
   }

    Freeaddrinfo(listp);
    if(p==NULL){return -1;}

    if(Listen(listenfd,LISTEN_NUM)<0){
           close(listenfd);
           return -1;
    }
   return listenfd;

}
/*open_listenfd end*/

typedef void (*sighandler_t)(int);
/*@signal handler@*/
sighandler_t Signal(int signum,sighandler_t hander){
    
    struct sigaction newaction,oldaction;

    newaction.sa_handler=hander;
    sigemptyset(&newaction.sa_mask);
    newaction.sa_flags=SA_RESTART;

    if(sigaction(signum,&newaction,&oldaction)<0){
        unix_error("sigaction");
    }
    return (oldaction.sa_handler);
}
/*@signla hander end@*/


/*sigprocmask*/
int Sigprocmask(int how,const sigset_t *set,sigset_t *oset){
      int rval;
      if((rval=sigprocmask(how,set,oset))<0){
          unix_error("sigprocmask");
      }
      return rval;
}
/*@sigprocmsk@*/

/*@memory operation@*/

/*unix_domain*/

int S_pipe(int fd[2]){
    
     return socketpair(AF_UNIX,SOCK_STREAM,0,fd); 
}

ssize_t
Sock_fd_write(int sock, void *buf, ssize_t buflen, int fd)
{
  ssize_t     size;
  struct msghdr   msg;
  struct iovec    iov;
  union {
    struct cmsghdr  cmsghdr;
    char        control[CMSG_SPACE(sizeof (int))];
  } cmsgu;
  struct cmsghdr  *cmsg;

  iov.iov_base = buf;
  iov.iov_len = buflen;

  msg.msg_name = NULL;
  msg.msg_namelen = 0;
  msg.msg_iov = &iov;
  msg.msg_iovlen = 1;

  if (fd != -1) {
    msg.msg_control = cmsgu.control;
    msg.msg_controllen = sizeof(cmsgu.control);

    cmsg = CMSG_FIRSTHDR(&msg);
    cmsg->cmsg_len = CMSG_LEN(sizeof (int));
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type = SCM_RIGHTS;

    printf ("passing fd %d\n", fd);
    *((int *) CMSG_DATA(cmsg)) = fd;
  } else {
    msg.msg_control = NULL;
    msg.msg_controllen = 0;
    printf ("not passing fd\n");
  }

  size = sendmsg(sock, &msg, 0);

  if (size < 0)
    perror ("sendmsg");
  return size;
}

ssize_t
Sock_fd_read(int sock, void *buf, ssize_t bufsize, int *fd)
{
  ssize_t     size;

  if (fd) {
    struct msghdr   msg;
    struct iovec    iov;
    union {
      struct cmsghdr  cmsghdr;
      char        control[CMSG_SPACE(sizeof (int))];
    } cmsgu;
    struct cmsghdr  *cmsg;

    iov.iov_base = buf;
    iov.iov_len = bufsize;

    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_control = cmsgu.control;
    msg.msg_controllen = sizeof(cmsgu.control);
    size = recvmsg (sock, &msg, 0);
    if (size < 0) {
      perror ("recvmsg");
      exit(1);
    }
    cmsg = CMSG_FIRSTHDR(&msg);
    if (cmsg && cmsg->cmsg_len == CMSG_LEN(sizeof(int))) {
      if (cmsg->cmsg_level != SOL_SOCKET) {
        fprintf (stderr, "invalid cmsg_level %d\n",
                 cmsg->cmsg_level);
        exit(1);
      }
      if (cmsg->cmsg_type != SCM_RIGHTS) {
        fprintf (stderr, "invalid cmsg_type %d\n",
                 cmsg->cmsg_type);
        exit(1);
      }

      *fd = *((int *) CMSG_DATA(cmsg));
      printf ("received fd %d\n", *fd);
    } else
      *fd = -1;
  } else {
    size = read (sock, buf, bufsize);
    if (size < 0) {
      perror("read");
      exit(1);
    }
  }
  return size;
}




int serv_listen( char *filename){
   int fd,len,rval,err;
   struct sockaddr_un un;
   
   if(strlen(filename)>sizeof(un.sun_path)){
       
       errno=ENAMETOOLONG;
       return (-1);
   }
   if((fd=socket(AF_UNIX,SOCK_STREAM,0))<0){
       
       return (-2);
   }
   unlink(filename);
   memset(&un,0,sizeof(struct sockaddr_un));
   strcpy(un.sun_path,filename);
   len=offsetof(struct sockaddr_un,sun_path)+strlen(filename);
   
   if(bind(fd,(struct sockaddr *)&un,len)==-1){
       info();
       debug("%d %s %d ",un.sun_family,un.sun_path,fd);
       rval=-3;
       goto errcode;
   }

   if(listen(fd,LISTEN_NUM)<0){
        rval=-4;
        goto errcode;
   }

   return fd;

   errcode:
      info();
      err=errno;
      close(fd);
      errno=err;
      return rval;

}

#define	STALE	30	

int serv_accept(int listenfd, uid_t *uidptr)
{
	int					clifd, err, rval;
	socklen_t			len;
	time_t				staletime;
	struct sockaddr_un	un;
	struct stat			statbuf;
	char				*name;

	/* allocate enough space for longest name plus terminating null */
	if ((name = malloc(sizeof(un.sun_path + 1))) == NULL)
		return(-1);
	len = sizeof(un);
	if ((clifd = accept(listenfd, (struct sockaddr *)&un, &len)) < 0) {
		free(name);
		return(-2);		/* often errno=EINTR, if signal caught */
	}

	/* obtain the client's uid from its calling address */
	len -= offsetof(struct sockaddr_un, sun_path); /* len of pathname */
	memcpy(name, un.sun_path, len);
	name[len] = 0;			/* null terminate */
	if (stat(name, &statbuf) < 0) {
		rval = -3;
		goto errout;
	}

#ifdef	S_ISSOCK	/* not defined for SVR4 */
	if (S_ISSOCK(statbuf.st_mode) == 0) {
		rval = -4;		/* not a socket */
		goto errout;
	}
#endif

	if ((statbuf.st_mode & (S_IRWXG | S_IRWXO)) ||
		(statbuf.st_mode & S_IRWXU) != S_IRWXU) {
		  rval = -5;	/* is not rwx------ */
		  goto errout;
	}

	staletime = time(NULL) - STALE;
	if (statbuf.st_atime < staletime ||
		statbuf.st_ctime < staletime ||
		statbuf.st_mtime < staletime) {
		  rval = -6;	/* i-node is too old */
		  goto errout;
	}

	if (uidptr != NULL)
		*uidptr = statbuf.st_uid;	/* return uid of caller */
	unlink(name);		/* we're done with pathname now */
	free(name);
	return(clifd);

errout:
	err = errno;
	close(clifd);
	free(name);
	errno = err;
	return(rval);
}



#define	CLI_PATH	"/tmp/info"
#define	CLI_PERM	S_IRWXU			/* rwx for user only */

/*
 * Create a client endpoint and connect to a server.
 * Returns fd if all OK, <0 on error.
 */
int cli_conn(const char *name)
{
	int					fd, len, err, rval;
	struct sockaddr_un	un, sun;
	int					do_unlink = 0;

	if (strlen(name) >= sizeof(un.sun_path)) {
		errno = ENAMETOOLONG;
		return(-1);
	}

	/* create a UNIX domain stream socket */
	if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
		return(-1);

	/* fill socket address structure with our address */
	memset(&un, 0, sizeof(un));
	un.sun_family = AF_UNIX;
	sprintf(un.sun_path, "%s%05ld", CLI_PATH, (long)getpid());
    printf("file is %s\n", un.sun_path);
	len = offsetof(struct sockaddr_un, sun_path) + strlen(un.sun_path);

	unlink(un.sun_path);		/* in case it already exists */
	if (bind(fd, (struct sockaddr *)&un, len) < 0) {
		rval = -2;
		goto errout;
	}
	if (chmod(un.sun_path, CLI_PERM) < 0) {
		rval = -3;
		do_unlink = 1;
		goto errout;
	}

	/* fill socket address structure with server's address */
	memset(&sun, 0, sizeof(sun));
	sun.sun_family = AF_UNIX;
	strcpy(sun.sun_path, name);
	len = offsetof(struct sockaddr_un, sun_path) + strlen(name);
	if (connect(fd, (struct sockaddr *)&sun, len) < 0) {
		rval = -4;
		do_unlink = 1;
		goto errout;
	}
	return(fd);

errout:
	err = errno;
	close(fd);
	if (do_unlink)
		unlink(un.sun_path);
	errno = err;
	return(rval);
}


#endif 
