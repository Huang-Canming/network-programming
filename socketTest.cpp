#include <stdio.h>
#include <iostream>
#include <errno.h>
#include <string.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define CLINET_SEND_MSG     "Hello Server~"
#define SERVER_SEND_MSG     "Hello Client~"
#define NET_MSG_BUF_LEN     128
#define SERVER_LISTEN_PORT  5197

// 编译命令 g++ -o socketExpirement -g socketTest.cpp -lpthread

using namespace std;

void* socketClient(void *){
    int iRes = 0;
    int iConnFd;
    int iNetMsgLen = 0;
    char szNetMsg[NET_MSG_BUF_LEN] = {0};
    struct sockaddr_in stServAddr;

    iConnFd = socket(AF_INET, SOCK_STREAM, 0);      // 面向网络的，流格式套接字，接收任何的 IP 数据包
    if (-1 == iConnFd) {
        printf("Client failed to create socket, err[%s]\n", strerror(errno));

        return NULL;
    }

    stServAddr.sin_family = AF_INET;                // 不直接对 sockaddr 操作，而是使用等价的 sockaddr_in
    stServAddr.sin_port = htons(SERVER_LISTEN_PORT);
    stServAddr.sin_addr.s_addr = inet_addr("127.0.0.1");        // https://blog.csdn.net/qianshen88/article/details/11934461
    iRes = connect(iConnFd, (struct sockaddr *)&stServAddr, sizeof(stServAddr));
    if (0 != iRes) {
        printf("Server failed to bind port[%d], err[%s]\n", SERVER_LISTEN_PORT, strerror(errno));
        close(iConnFd);
        return NULL;
    }
    else {
        printf("Client succeeded to connect to[%s:%u]\n", "127.0.0.1", SERVER_LISTEN_PORT);
    }

    iNetMsgLen = write(iConnFd, CLINET_SEND_MSG, strlen(CLINET_SEND_MSG));             // https://www.cnblogs.com/junneyang/p/6126635.html
    if (iNetMsgLen < 0) {
        printf("Client failed to send msg to server, err[%s]\n", strerror(errno));
        close(iConnFd);
        return NULL;
    }

    do
    {
        iNetMsgLen = read(iConnFd, szNetMsg, sizeof(szNetMsg));             // 通用的读文件设备的接口，默认阻塞
        if (iNetMsgLen < 0) {
            printf("Client failed to read from network, err[%s]\n", strerror(errno));
            close(iConnFd);
            return NULL;

        }
        else if (0 == iNetMsgLen) {
            printf("Clinet recv reply[%s] from server\n", szNetMsg);
        }
    }
    while(0 != iNetMsgLen);
    
    close(iConnFd);
    return NULL;
}

void* socketServer(void *){
    int iRes = 0;
    int iLsnFd, iConnFd;
    int iNetMsgLen = 0;
    socklen_t iSockAddrLen = 0;
    char szNetMsg[NET_MSG_BUF_LEN] = {0};
    struct sockaddr_in stLsnAddr;
    struct sockaddr_in stCliAddr;

    iLsnFd = socket(AF_INET, SOCK_STREAM, 0);      // 面向网络的，流格式套接字，接收任何的 IP 数据包
    if (-1 == iLsnFd) {
        printf("Server failed to create socket, err[%s]\n", strerror(errno));
        return NULL;
    }

    stLsnAddr.sin_family = AF_INET;                // 不直接对 sockaddr 操作，而是使用等价的 sockaddr_in
    stLsnAddr.sin_port = htons(SERVER_LISTEN_PORT);
    stLsnAddr.sin_addr.s_addr = INADDR_ANY;        // https://blog.csdn.net/qianshen88/article/details/11934461
    iRes = bind(iLsnFd, (struct sockaddr*)&stLsnAddr, sizeof(stLsnAddr));

    if (-1 == iRes) {
        printf("Server failed to bind port[%u], err[%s]\n", SERVER_LISTEN_PORT, strerror(errno));
        close(iLsnFd);
        return NULL;
    }

    do {
        iRes = listen(iLsnFd, 16);
    }while(0 == iRes);

    iSockAddrLen = sizeof(sockaddr_in);
    iConnFd = accept(iLsnFd, (struct sockaddr*)&stCliAddr, &iSockAddrLen);
    if (-1 == iConnFd) {
        printf("Server failed to accept connect request, err[%s]\n", strerror(errno));
        close(iLsnFd);
        return NULL;
    }
    else {
        printf("Server accept connect request\n");
    }

    do
    {
        iNetMsgLen = read(iConnFd, szNetMsg, sizeof(szNetMsg));             // 通用的读文件设备的接口，默认阻塞
        if (iNetMsgLen < 0) {
            printf("Server failed to read from network, err[%s]\n", strerror(errno));
            close(iConnFd);
            close(iLsnFd);
            return NULL;
        }
        else if (0 == iNetMsgLen) {
            printf("Server read msg finished\n");
        }
        else {
            printf("Server recv msg[%s]\n", szNetMsg);
        }
    }
    while(0 != iNetMsgLen);

    iNetMsgLen = write(iConnFd, SERVER_SEND_MSG, strlen(SERVER_SEND_MSG));             // https://www.cnblogs.com/junneyang/p/6126635.html
    if (iNetMsgLen < 0) {
        printf("Server failed to reply client, err[%s]\n", strerror(errno));
    }

    close(iConnFd);
    close(iLsnFd);
    return NULL;
}

int main(){
    pthread_t thdServer = 1;
    pthread_t thdClient = 2;
    
    pthread_create(&thdServer, NULL, socketServer, NULL);
    pthread_create(&thdClient, NULL, socketClient, NULL);

    pthread_join(thdServer, NULL);                  //https://blog.csdn.net/heybeaman/article/details/90896663
    pthread_join(thdClient, NULL);                  // 不添加，进程过早退出，会导致线程也退出
    return 0;
}