#ifndef MICHART_H
#define MICHART_H

#ifndef IPv6
typedef struct sockaddr_in SI;
#define IPv4
#else
typedef struct sockaddr_in6 SI;
#endif  //IPv6
typedef struct sockaddr SA;


#define BUFSIZE         1024
#define NNBUFFSIZE		(20+1)		//nickname buff size

typedef struct _client
{
    int     sfd;        //套接字描述符
    char    name[NNBUFFSIZE];   //客户端昵称
    char    ip[16];     //点分十进制ip地址
}Client;


typedef struct Message
{
    char from[NNBUFFSIZE];
    char to[NNBUFFSIZE];
    char msg[BUFSIZE];
}Message;

#define PORT            2016    //服务器端口

#define TCP
#ifdef  TCP
#define MAX_LISTENED    20      //tcp 最大监听数
#endif  //TCP


#endif //MICHART_H
