/*
 * 文件:client_2.c
 * 功能：局域网聊天工具客户端
 * Auther:Leon
 */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include "apue.h"
#include "michat.h"

char send_buf[BUFSIZE];
char recv_buf[BUFSIZE];

void *rthread(void *arg)
{
    ssize_t rn;
    int sfd =*(int*)arg;
    //读取数据
    while(true)
    {
        memset(recv_buf, 0, BUFSIZE);
        if((rn = recv(sfd, recv_buf, BUFSIZE, 0)) > 0)
        {
            printf("\n%s",recv_buf);
        }
        else if(rn == 0)
        {   //eof 服务器已关闭
            exit(EXIT_FAILURE);
        } 
        puts("输入信息：");
    }

}
int main(int argc, char *argv[])
{
    int sfd;
    SI serv_addr;
    pthread_t tid;

    if(argc != 4)
    {
        printf("Usage:<server IP> <port> <nickname>\n");
        exit(EXIT_FAILURE);
    }

    sfd = Socket(AF_INET, SOCK_STREAM, 0);

    //connect
    memset(&serv_addr, 0, sizeof(SI));
    serv_addr.sin_family      = AF_INET;
    inet_pton(AF_INET, argv[1], &serv_addr.sin_addr.s_addr);
    serv_addr.sin_port        = htons(atoi(argv[2]));
    Connect(sfd, (const SA *)&serv_addr, sizeof(SI));
    //send 
    const char *login_format ="@%s@";
    char login_buf[NNBUFFSIZE+3];
    snprintf(login_buf, sizeof(login_buf), login_format, argv[3]);
    send(sfd, login_buf, strlen(login_buf)+1, 0); 

    if(pthread_create(&tid, NULL, rthread, (void *)&sfd) !=0)
    {
        perror("pthread_create");
        exit(EXIT_FAILURE);
    }
    while(true)
    { 

      ssize_t wn;
      //发送数据
      if(fgets(send_buf, BUFSIZE,stdin) == NULL)
      {   //输入EOF并退出
          exit(EXIT_SUCCESS);
      }
      if((wn = write(sfd, send_buf, strlen(send_buf)+1)) < 0)
      {
          perror("write");
          exit(EXIT_SUCCESS);
      } 
    }

   exit(EXIT_SUCCESS);
}

