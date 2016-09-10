#ifndef APUE_H
#define APUE_H
#include <sys/socket.h>
#include <sys/types.h>

#define sys_err(msg)    \
    do {perror(msg);exit(EXIT_FAILURE);}while(0)

#ifdef __cplusplus
extern "C" {
#endif  //__cplusplus

typedef struct sockaddr SA;

int Socket(int domain, int type, int proto);

void Bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

void Listen(int sockfd, int backlog);

int Accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);

void Connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

void createTcpSocket(int *sock, int port, int maxlisten);

void createUdpSocket(int *sock, int port);

int Select(int nfds, fd_set *readfds, fd_set *writefds,
            fd_set *exceptfds, struct timeval *timeout);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //APUE_H
