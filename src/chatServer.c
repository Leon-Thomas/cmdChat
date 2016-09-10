/*
**文件：chatServer.c
**时间：2016.09.08 20:22:46
**作者：Leon
**功能：终端聊天程序

*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <signal.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "apue.h"
#include "List.h"
#include "michat.h"

List client_list;
pthread_mutex_t mutex;
pthread_attr_t pattr;
const char *warning = "对不起，聊天室已满！\n";
const char *welcome = "欢迎 %s 进入聊天室！\n";



//初始化工作
static void init();
//中断信号处理函数
static void sig_handler(int arg);
//退出处理函数
static void ext_clean();
//客户端线程
static void *client(void *arg);
//用于广播信息
static void broadcast(int from_fd, const char *msg, ssize_t sz);

static Client *loginVerify(int serverSock);
static char *parseNickname(const char *text, size_t *len);

static int findSocket(const char *nickname);
static bool sendMsg(const Message *msg);
static void sendWelcome(const Client *cp);


int main(void)
{
    int serv_sock,new_sock;
    pthread_t tid;

    init();
    createTcpSocket(&serv_sock, PORT, MAX_LISTENED+1);
    int optval = 1;
    if(setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, &optval,sizeof(int)) < 0)
        sys_err("setsockopt");
    
    while(true)
    {
        //验证登陆信息并返回Client信息
        Client *cp = loginVerify(serv_sock);
        if(cp == NULL) continue;
        pthread_mutex_lock(&mutex);
        listAppend(&client_list, (Data)cp, sizeof(Client));
        pthread_mutex_unlock(&mutex);
     
        //发送欢迎信息（广播形式）
        sendWelcome(cp);

        //为客户创建聊天线程
        if(pthread_create(&tid, &pattr, client, (void *)cp) < 0)
            sys_err("pthread_creat");
    }

    return 0;
}



void init()
{
    listInit(&client_list);
    pthread_mutex_init(&mutex, NULL);
    pthread_attr_init(&pattr);
    pthread_attr_setdetachstate(&pattr, PTHREAD_CREATE_DETACHED);
    signal(SIGINT, sig_handler);
    atexit(ext_clean);
}

void sig_handler(int arg)
{
    //只是为了在收到SIGINT信号时能正常退出
    exit(EXIT_SUCCESS);
}

void ext_clean()
{
    const char end_msg[128] = "服务器将在5秒钟后退出!!\n";
    broadcast(-1, end_msg, strlen(end_msg));
    for(int i = 0;i < 5;++i)
    {
        sleep(1);
    }
    destroyList(&client_list);
}

void *client(void *arg)
{
    Client *c = (Client*)arg;
    ssize_t n;
    char buf[BUFSIZE];
    char msg_buff[BUFSIZE];
    const char *exit_msg = "%s 离开了聊天室!\n";
    const char *prfix    = "来自%s:%s";
    Message msg;
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
            
            if( (n  = recv(c->sfd, buf, BUFSIZE, 0)) > 0)
            {
                //send message
                ssize_t len = n;
                pthread_mutex_lock(&mutex);

                const char *nickname = parseNickname(buf, &len);               
                if(nickname == NULL)
                    msg.to[0] = '\0';
                else
                    strncpy(msg.to, nickname, len+1);

                pthread_mutex_unlock(&mutex);                              
                strncpy(msg.from, c->name, NNBUFFSIZE);
                ssize_t offset = len ? len+2 : 0;
                strncpy(msg.msg, (buf+offset),BUFSIZE);   //@len@
                if(!sendMsg(&msg))
                {
                    fprintf(stderr,"sendMsg error\n");
                }

            }
            else if(n == 0)
            {
                strncpy(msg.from, "Server", NNBUFFSIZE);
                snprintf(msg.msg, BUFSIZE, exit_msg, c->name);
                fprintf(stderr,"%s",msg.msg);
                msg.to[0] = '\0';
                if(!sendMsg(&msg))
                {
                    fprintf(stderr,"sendMsg error\n");
                }               
                close(c->sfd);
                pthread_mutex_lock(&mutex);
                listDelByData(&client_list, (Data)arg, sizeof(Client));
                pthread_mutex_unlock(&mutex);
                pthread_exit((void*)0);
            }

        }
    }
    pthread_exit((void*)0);
}


void broadcast(int from_fd,const char *msg, ssize_t sz)
{
    int i,len;
    ssize_t n;
    Client *tmp;

    pthread_mutex_lock(&mutex);

    len = client_list.lt_len;
    //fprintf(stderr, "list:%d\n", len);   
    for(i = 0;i < len;++i)
    {
        tmp = listGet(&client_list, i);
        if(tmp && tmp->sfd != from_fd)
        {
            n =send(tmp->sfd, msg, sz, 0);
            if(n != sz)
            {
                perror("send");
                return ;
            }
        }
    }

    pthread_mutex_unlock(&mutex);
}

Client *loginVerify(int serverSock)
{
    struct sockaddr_in clit_addr;
    int sock;
    socklen_t slen;
#define handle_err() do{close(sock);return NULL;}while(0)

    slen = sizeof(struct sockaddr_in);
    sock = Accept(serverSock, (SA *)&clit_addr, &slen);
    if(listSize(&client_list) == MAX_LISTENED)
    {
        ssize_t n = send(sock, warning, strlen(warning)+1, 0);
        if(n < 0) sys_err("send");
        //close(sock);
        shutdown(sock, SHUT_RDWR);  //
        return NULL;
    }
    else
    {
        char logMsg[NNBUFFSIZE+2];
        const char *name = NULL;
        ssize_t n;

        n = recv(sock,logMsg, NNBUFFSIZE+2, 0);
        if(n < 0) sys_err("recv");
        else if(n == 0) handle_err(); //disconnected with remote  
        pthread_mutex_lock(&mutex);  
        name = parseNickname(logMsg, &n);  //not thread-Safty
        if(name == NULL) handle_err();

        Client *client =(Client *)malloc(sizeof(Client));
        if(client == NULL) {pthread_mutex_unlock(&mutex);handle_err();}
        strncpy(client->name, name, n+1);
        pthread_mutex_unlock(&mutex);
        client->sfd = sock;
        strncpy(client->ip, inet_ntoa(clit_addr.sin_addr), sizeof(client->ip));
        return client;
    }
#undef handle_err
}

char *parseNickname(const char *text, size_t *len)
{
    static char buf[NNBUFFSIZE+1];
    int i = 0,j = 0,sz = 0;

    sz = *len,*len = 0;
    if(text[0] != '@') return NULL;
    for(j = 0,i = 1;i < sz && text[i] != '@';++i)
    {
        buf[j++] = text[i];
    }
    if(j == 0) return NULL;
    buf[j] = '\0';
    *len = j;
    return buf;
}

int findSocket(const char *nickname)
{
    pthread_mutex_lock(&mutex);

    Client *cp = NULL;
    size_t n = listSize(&client_list);
    size_t i;
    for(i = 0;i < n;++i)//;   <--!!!
    {
        cp =(Client *)listGet(&client_list, i);
        //fprintf(stderr,"%s-%s\n",cp->name,nickname);
        if(cp && strncmp(cp->name, nickname, strlen(cp->name))==0)
            {pthread_mutex_unlock(&mutex);return cp->sfd;}
    }
    pthread_mutex_unlock(&mutex);
    return -1;
}

bool sendMsg(const Message *msg)
{
    if(msg == NULL) return false;
    if(strlen(msg->to) == 0)
    {   //broadcast
        const char *bc_format = "From %s : %s";
        char bc_buff[BUFSIZE];
        snprintf(bc_buff, BUFSIZE, bc_format, msg->from,msg->msg);
        fprintf(stderr,"%s",bc_buff);
        broadcast(-1, bc_buff, strlen(bc_buff)+1);
    }
    else
    {   //one to one
        const char *o2o_format = "%s to You: %s";
        int tofd = -1;

        tofd = findSocket(msg->to);
        if(tofd < 0) return false;

        /*char o2o_buff[BUFSIZE];
        snprintf(o2o_buff, BUFSIZE, o2o_format, msg->from,msg->msg);*/
        ssize_t n = dprintf(tofd, o2o_format,msg->from, msg->msg);
        if(n < 0) return false;
        
    }

    return true;
}

void sendWelcome(const Client *cp)
{
    Message msg;

    strncpy(msg.from, "Server", NNBUFFSIZE+1);
    sprintf(msg.msg, welcome, cp->name);
    msg.to[0] = '\0';
    if(!sendMsg(&msg)) fprintf(stderr,"sendWelcome error\n");
}
