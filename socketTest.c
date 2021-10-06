#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define LOCAL_IP_ADDR       "127.0.0.1"
#define SERVER_LISTEN_PORT  5197
#define NET_MSG_BUF_LEN     128
#define CLINET_SEND_MSG     "Hello Server~"
#define SERVER_SEND_MSG     "Hello Client~"


void* client(void* param){
    int iRes = 0;
    int iConnFd;
    int iNetMsgLen = 0;
    char szNetMsg[NET_MSG_BUF_LEN] = {0};
    struct sockaddr_in stServAddr;

    iConnFd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == iConnFd) {
        printf("Client failed to create socket, err[%s]\n", 
               strerror(errno));
        return NULL;
    }

    // 填充目标地址结构体，指定协议族、目标端口、目标主机 IP 地址
    stServAddr.sin_family = AF_INET;
    stServAddr.sin_port = htons(SERVER_LISTEN_PORT);
    stServAddr.sin_addr.s_addr = inet_addr(LOCAL_IP_ADDR);
    
    while (1) {
        // 1 参传套接字句柄，2 参传准备连接的目标地址结构体指针
        // 3 参传地址结构体大小
        iRes = connect(iConnFd, (struct sockaddr *)&stServAddr, 
                       sizeof(stServAddr));
        if (0 != iRes) {
            printf("Client failed to connect to[%s:%u], err[%s]\n", 
                   LOCAL_IP_ADDR, SERVER_LISTEN_PORT, 
                   strerror(errno));
            sleep(60);
            continue;        
		} else {
            printf("Client succeeded to connect to[%s:%u]\n", 
                   LOCAL_IP_ADDR, SERVER_LISTEN_PORT);
            break;
        }
    }
    
    iNetMsgLen = send(iConnFd, CLINET_SEND_MSG, 
                      strlen(CLINET_SEND_MSG), 0);
    if (iNetMsgLen < 0) {
        printf("Client failed to send msg to server, err[%s]\n", 
               strerror(errno));
        close(iConnFd);
        return NULL;
    }
    
    iNetMsgLen = recv(iConnFd, szNetMsg, sizeof(szNetMsg), 0);
    if (iNetMsgLen < 0) {
        printf("Client failed to read from network, err[%s]\n", 
               strerror(errno));
        close(iConnFd);
        return NULL;
    } else {
        printf("Client recv reply[%s]\n", szNetMsg);
    }
    
    close(iConnFd);
    return NULL;
}

void* server(void* param){
    int iRes = 0;
    int iLsnFd, iConnFd;
    int iNetMsgLen = 0;
    socklen_t iSockAddrLen = 0;
    char szNetMsg[NET_MSG_BUF_LEN] = {0};
    struct sockaddr_in stLsnAddr;
    struct sockaddr_in stCliAddr;

    // 1 参指定协议族，AF_INET 对应 IPv4
    // 2 参指定套接字类型，SOCK_STREAM 对应 面向连接的流式套接字
    // 3 参指定协议类型，0 对应 TCP 协议
    iLsnFd = socket(AF_INET, SOCK_STREAM, 0);                       
    if (-1 == iLsnFd) {
        printf("Server failed to create socket, err[%s]\n", 
               strerror(errno));
        return NULL;
    }

    // 填写监听地址
    // 设置 s_addr = INADDR_ANY 表示监听所有网卡上对应的端口
    stLsnAddr.sin_family = AF_INET;
    stLsnAddr.sin_port = htons(SERVER_LISTEN_PORT);
    stLsnAddr.sin_addr.s_addr = INADDR_ANY;
    // 1 参传入 socket 句柄，2 参传入监听地址，
    // 3 参传入监听地址结构体的大小
    iRes = bind(iLsnFd, (struct sockaddr*)&stLsnAddr, 
                sizeof(stLsnAddr));   
    if (-1 == iRes) {
        printf("Server failed to bind port[%u], err[%s]\n", 
               SERVER_LISTEN_PORT, strerror(errno));
        close(iLsnFd);
        return NULL;
    } else {
        printf("Server succeeded to bind port[%u], start listen.\n",
               SERVER_LISTEN_PORT);
    }

    // 1 参传入监听句柄
    // 2 参设置已完成连接队列（已完成三次握手，未 accept 的连接）的长度
    iRes = listen(iLsnFd, 16);
    if (-1 == iRes) {
        printf("Server failed to listen port[%u], err[%s]\n", 
               SERVER_LISTEN_PORT, strerror(errno));
        close(iLsnFd);
        return NULL;
    }

    iSockAddrLen = sizeof(stCliAddr);
    // 1 参传入监听句柄，2 传入地址结构体指针接收客户端地址，
    // 3 参传入地址结构体大小
    iConnFd = accept(iLsnFd, (struct sockaddr*)&stCliAddr, 
                     &iSockAddrLen);
    if (-1 == iConnFd) {
        printf("Server failed to accept connect request, err[%s]\n", 
               strerror(errno));
        close(iLsnFd);
        return NULL;
    } else {
        printf("Server accept connect request from[%s:%u]\n", 
               inet_ntoa(stCliAddr.sin_addr), 
               ntohs(stCliAddr.sin_port));
    }
    
    // 1 参传已连接套接字描述符，2 参传缓冲区指
    // 3 参传缓冲区大小，4 参指定行为，默认为 0
    iNetMsgLen = recv(iConnFd, szNetMsg, sizeof(szNetMsg), 0);
    if (iNetMsgLen < 0) {
        printf("Server failed to read from network, err[%s]\n", 
               strerror(errno));
        close(iConnFd);
        close(iLsnFd);
        return NULL;
    } else {
        printf("Server recv msg[%s]\n", szNetMsg);
    }

    // 1 参传已连接套接字的描述符，2 参传指向消息数据的指针
    // 3 参传消息长度，4 参指定行为，默认为 0
    iNetMsgLen = send(iConnFd, SERVER_SEND_MSG, 
                      strlen(SERVER_SEND_MSG), 0);
    if (iNetMsgLen < 0) {
        printf("Server failed to reply client, err[%s]\n", 
               strerror(errno));
    }

    close(iConnFd);
    close(iLsnFd);
    return NULL;
}


int main(){
    // 线程 ID，实质是 unsigned long 类型整数
    pthread_t thdServer = 1;
    pthread_t thdClient = 2;
    
    // 1 参传线程 ID，2 参传线程属性，
    // 3 参指定线程入口函数，4 参指定传给入口函数的参数
    pthread_create(&thdServer, NULL, server, NULL);    
    pthread_create(&thdClient, NULL, client, NULL);

    // 1 参传入线程 ID
    // 2 参用于接收线程入口函数的返回值，不需要返回值则置 NULL
    pthread_join(thdServer, NULL);    
    pthread_join(thdClient, NULL);
    return 0;
}
