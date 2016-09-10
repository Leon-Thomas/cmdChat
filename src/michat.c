/*
 *文件：michat.c
 *功能：局域网命令行多人聊天工具
 *日期：2016.7.23
 *Auther:Leon
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdbool.h>
#include <signal.h>
#include "apue.h"
#include "michat.h"
#include "List.h"


List clients;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;


void broadcast(int sock_fd,const char *msg, ssize_t sz)
{
    int i,len;
    ssize_t n;
    Client *tmp;

    //pthread_mutex_lock(&mutex);
    len = clients.lt_len;
    //pthread_mutex_unlock(&mutex);

    for(i = 0;i < len;++i)
    {
        tmp = listGet(&clients, i);
        if(tmp && tmp->sfd != sock_fd)
        {
            n =send(tmp->sfd, msg, sz, 0);
            if(n != sz)
            {
                perror("send");
                return ;
            }
        }
    }
}

//退出处理函数
static void do_clean()
{
    const char *exit_msg = "服务器即将退出...\n";
#define EXITMSGSIZE 5
    char timed_msg[EXITMSGSIZE];
    broadcast(-1, exit_msg, strlen(exit_msg)+1);
    int i = 5;
    while(i > 0)
    {
        snprintf(timed_msg, EXITMSGSIZE, "%d\n",i);
        broadcast(-1, timed_msg, strlen(timed_msg)+1);
        sleep(1);
        --i;
    }
    destroyList(&clients);
}
//信号处理函数
static void handler(int arg)
{
    exit(EXIT_SUCCESS);
}

static void *func(void *arg)
{
    Client *c = (Client*)arg;
    ssize_t n;
    char buf[BUFSIZE];
    char msg[BUFSIZE];
    const char *exit_msg = "%s 离开了聊天室!\n";
    const char *prfix    = "来自%s:%s";
    fd_set fd_recv;
    FD_ZERO(&fd_recv);


    //memset(msg, 0, BUFSIZE);
    while(true)
    {
        FD_SET(c->sfd, &fd_recv);
        struct timeval timeout;
        timeout.tv_sec  = 0;
        timeout.tv_usec = 1000;

        int reval = Select(c->sfd+1, &fd_recv, NULL, NULL, &timeout);
        if(reval > 0 && FD_ISSET(c->sfd, &fd_recv))
        {
            pthread_mutex_lock(&mutex);
            if( (n  = recv(c->sfd, buf, BUFSIZE, MSG_DONTWAIT)) > 0)
            {
                //广播
                snprintf(msg, BUFSIZE, prfix, c->ip, buf);
                fprintf(stderr,"%s",msg);
                broadcast(c->sfd,msg,strlen(msg)+1);
            }
            else if(n == 0)
            {
                snprintf(buf, BUFSIZE, exit_msg, c->ip);
                broadcast(c->sfd,buf,strlen(buf)+1);
                fprintf(stderr,"%s",buf);
                close(c->sfd);
                listDelByData(&clients, (Data)arg, sizeof(Client));
                pthread_mutex_unlock(&mutex);
                pthread_exit((void*)0);
            }
            pthread_mutex_unlock(&mutex);

        }
    }
}

int main()
{
    int fd,cfd;
    SI serv_addr,clit_addr;
    char wel_buf[BUFSIZE];
    const char *warning = "对不起，聊天室已满！\n";
    const char *welcome = "%s欢迎进入聊天室！\n";

    //初始化
    signal(SIGINT, handler);
    atexit(do_clean);
    listInit(&clients);     //初始化链表
    createTcpSocket(&fd, PORT, MAX_LISTENED);

    //监听并生成客户端列表
    while(true)
    {
        socklen_t slen = sizeof(SI);
        memset(&clit_addr, 0, sizeof(SI));
        cfd = Accept(fd, (SA *)&clit_addr, &slen);
       if(clients.lt_len == MAX_LISTENED)
       {
           write(cfd, warning, strlen(warning)+1);
           close(cfd);
       }
       else
       {
           Client *clt =(Client *)malloc(sizeof(Client));
           if(clt == NULL)
           {
               perror("malloc");
               exit(EXIT_FAILURE);
           }
           clt->sfd = cfd;
           strncpy(clt->ip, inet_ntoa(clit_addr.sin_addr), sizeof(clt->ip));

           pthread_mutex_lock(&mutex);
           listAppend(&clients, (Data)clt, sizeof(Client));
           snprintf(wel_buf, BUFSIZE, welcome, clt->ip);
           fprintf(stderr,"%s\n",wel_buf);
           //broadcast(clt->sfd,wel_buf,strlen(wel_buf)+1);
           broadcast(-1, wel_buf, strlen(wel_buf)+1);

           pthread_mutex_unlock(&mutex);


           pthread_t pid;
           if(pthread_create(&pid, NULL, func, (void*)clt) < 0)
           {
            perror("pthread_create");
            exit(EXIT_FAILURE);
           }
        }
       }

    exit(EXIT_SUCCESS);
}


