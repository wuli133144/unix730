
/*unix_domain*/

#include <stdarg.h>
#include <stddef.h>

#ifndef STDIO__STDOUT

    #include <stdio.h>

#else 
       #define STDIO__STDOUT
#endif

  #define info()           do{\
    fprintf(stderr,"%s:: %d:: %s:: %s\n",__TIME__,__LINE__,__func__,strerror(errno)); \
  }while(0)

      

static void  vspf(const char *fmt,...){
    do{                                  
        va_list argptr;                                       
        char buffer[80];                                      
        int cnt;                                              
        va_start(argptr, fmt);                                   
        cnt = vsnprintf(buffer,sizeof(buffer) ,fmt, argptr);  
        buffer[sizeof(buffer)-1]='\n';
        fputs(buffer,stdout);                       
        va_end(argptr);                                       
     }while(0);
}

#define  debug(fmt,...) do{\
  vspf(fmt,__VA_ARGS__);   \
}while(0)      
