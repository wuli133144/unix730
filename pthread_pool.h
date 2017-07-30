

#include <stdio.h>
#include <unistd.h>

#include <signal.h>
#include <stddef.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include  <pthread.h>

#include "list.h"
#include "utils.h"

#define PTHREAD_POOL_MAX      10
typedef void *(*callback_t)(void *)   ;

typedef  struct __pthread_pool_node{

   pthread_t pid;
   int status;
   void *arg;
   void *(*call_back)(void *);
   LIST_ENTRY(__pthread_pool_node)entry;
   pthread_mutex_t mutex;
}pthread_pool_t;


typedef struct _task{
      void *arg;
      pthread_mutex_t mutex;
      TAILQ_ENTRY(_task)entry;
}t_task;


LIST_HEAD(_pthread_pool,__pthread_pool_node)pthread_pool\
=LIST_HEAD_INITIALIZER(_pthread_pool);



TAILQ_HEAD(_task_pool,_task)task_pool\
=TAILQ_HEAD_INITIALIZER(task_pool);

pthread_mutex_t  mutex=PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t   cond=PTHREAD_COND_INITIALIZER;
pthread_mutex_t  mutex_task=PTHREAD_MUTEX_INITIALIZER;



static pthread_pool_t *alloc_pool_node();
static void create_pthread_pool();
static void insert(int );





static void insert_task(void *arg);
static void  remove_task();


static t_task* alloc_task(){
            t_task *item=NULL;
            item=(t_task *)malloc(sizeof(t_task));
            if(item==NULL){
               fprintf(stderr,"%s","malloc pthread_pool_t failed!\n");
                exit(EXIT_FAILURE);
            }
            item->arg=NULL;
            pthread_mutex_init(&(item->mutex),NULL);
            item->arg=NULL;
            return item;
}

static void remove_task(){
         t_task *item=NULL;
         t_task*pre=NULL;
         TAILQ_FOREACH(item,&task_pool,entry){
                pre=item;
                pthread_mutex_trylock(&mutex_task);
                TAILQ_REMOVE(&task_pool,item,entry);
                pthread_mutex_unlock(&mutex_task);
                free(pre);
                pre=NULL;
                break;
         }
}
static t_task *get_task(){
         
          return TAILQ_FIRST(&task_pool);
}

static void insert_task(void *arg){
        t_task *item=NULL;
        item=alloc_task();
        item->arg=arg;
        pthread_mutex_trylock(&mutex_task);
        TAILQ_INSERT_TAIL(&task_pool,item,entry);
        pthread_mutex_unlock(&mutex_task);

        pthread_mutex_lock(&mutex);
        pthread_cond_signal(&cond);//awake all thread
        pthread_mutex_unlock(&mutex);

        }


static void create_pthread_pool(){
       //pthread_mutex_init(&mutex,NULL);
       int i=1;
       FOR_EACH(i,PTHREAD_POOL_MAX,insert);//create pool
}

void show(){
       pthread_pool_t *item=NULL;
       LIST_FOREACH(item,&pthread_pool,entry){
            printf("%d\n",item->pid);
       }
}


static void insert(int i){
        pthread_pool_t *item=NULL;
        item=alloc_pool_node();
        LIST_INSERT_HEAD(&pthread_pool,item,entry);
}

callback_t call(void *arg){
         return NULL;
}




void * function(void *arg){
        //  pthread_mutex_lock(&mutex);      //1
         while(1){
                  pthread_mutex_lock(&mutex);
                  pthread_cond_wait(&cond,&mutex); //如g_cond无信号,则阻塞
                  pthread_mutex_unlock(&mutex);
                  t_task *task=get_task();
                  
                  // append_task(call,task->arg);

                  int connfd=*(int *)(task->arg);
                  char buf[100]={0};
                  
                  read(connfd,buf,100);
                  write(connfd,buf,100);
                  close(connfd);
                  remove_task();
                  pthread_mutex_unlock(&mutex);   

         //to dosomthing
         }

         pthread_detach(pthread_self());//release resource
         return NULL;

}

static pthread_pool_t *alloc_pool_node(){
            pthread_pool_t *item=NULL;
            item=(pthread_pool_t *)malloc(sizeof(pthread_pool_t));
            if(item==NULL){
               fprintf(stderr,"%s","malloc pthread_pool_t failed!\n");
                exit(EXIT_FAILURE);
            }
            item->pid=-1;
            item->status=0;
            item->call_back=function;
            pthread_mutex_init(&(item->mutex),NULL);
            item->arg=NULL;
            return item;
}


int  append_task(callback_t callback,void* arg){
         pthread_pool_t *item=NULL;
         LIST_FOREACH(item,&pthread_pool,entry){
                if(item->status==0){
                    pthread_mutex_lock(&(item->mutex));
                     item->call_back=callback;
                     item->arg=arg;
                     item->status=1;//ok
                    pthread_mutex_unlock(&(item->mutex));
                     break;
                }
         }
         if(item==NULL){
              return (-1);
         }
         item->call_back(item->arg);//run
         pthread_mutex_lock(&(item->mutex));
         item->status=0;
         pthread_mutex_unlock(&(item->mutex));
         return 0;
}


void destroy_pool(){
        pthread_pool_t *item=NULL;
          pthread_pool_t *pre=NULL;
         LIST_FOREACH(item,&pthread_pool,entry){
                pre=item;
                pthread_mutex_destroy(&(item->mutex));
                LIST_REMOVE(item,entry);
                info();
                free(pre);
                pre=NULL;
         }
         LIST_FIRST(&pthread_pool)=NULL;

}



void init_pthreadpool(){

       pthread_t pid;
       pthread_pool_t *item=NULL;
       create_pthread_pool();
       LIST_FOREACH(item,&pthread_pool,entry){
             pthread_create(&(item->pid),NULL,item->call_back,item->arg);//not modify attrbute pthread
       }

}   