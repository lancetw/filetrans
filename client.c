/**
 * client.c : 簡易多人CLI聊天室客戶端
 *
 * Copyright (c) 2012 鄭新霖 aka Hsin-lin Cheng <lancetw@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */       

#include "global.h"

#include <stdio.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
#include <limits.h>
#include <netdb.h>
#include <errno.h>

/* 預設緩衝區長度為 1024 bytes */

#define BUFF_LEN 1024

/* 預設伺服器位址 */
#define SERVER "127.0.0.1"

/* 預設開放端口*/

#define PORT 12345

int stdin_ready(void);

int main(int argc, char *argv[]) {
    
    int client_sockfd, nread;
    int len;
    struct sockaddr_in address;
    char server[UCHAR_MAX];
    int result;
    char buf[BUFF_LEN]; 
    char tmp[BUFF_LEN];
    char msg[BUFF_LEN];
    char nick[UCHAR_MAX];
    fd_set readfds;


    DEBUG("除錯模式啟動\n");

    /* 要求使用者輸入暱稱 */
    
    do {
        printf("請輸入暱稱：");
        scanf("%s", nick);
    } while ((char*)NULL == nick);

    /*  建立客戶端 socket  */

    if ((client_sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket() 呼叫失敗");
        exit(EXIT_FAILURE);
    }

    /*  設定客戶端 socket  */
    
    /* 使用者提供伺服器資訊 */
    if (argc > 1) {
        /* 使用參數模式 */
        strcpy(server, argv[1]);
        printf("正在連線到 %s:%d ...\n", server, PORT);
    } else {
        /* 使用預設伺服器資訊 */
        strcpy(server, SERVER);
    }

    memset(&address, 0x00, sizeof (struct sockaddr_in));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(SERVER);
    address.sin_port = htons(PORT);
    len = sizeof (address);

    /* 與伺服器建立連線 */

    if ((result = connect(client_sockfd, (struct sockaddr *)&address, len)) < 0) {
        perror("connect() 呼叫失敗"); 
        close(client_sockfd);
        exit(EXIT_FAILURE);

    } else {
        printf("連線中...\n");
    }
    
    FD_ZERO(&readfds);
    FD_ZERO(&buf);
    FD_ZERO(&tmp);
    FD_ZERO(&msg);
    FD_SET(client_sockfd, &readfds);
    
    /* 開始傳送接收資料 */

    printf("%s 歡迎加入聊天室\n", nick);

    for(;;) {
        /* 開始接收資料 */
        
        ioctl(client_sockfd, FIONREAD, &nread);
        
        if (stdin_ready()) {
            fscanf(stdin, "\n%[^\n]", buf);
            sprintf(msg, "%s: %s", nick, buf);
            if (send(client_sockfd, msg, sizeof (msg), 0)) {
                DEBUG("訊息已送出\n");
            }
        }

        usleep(100);

        if (!nread == 0) { /* 處理客戶端資料 */
            
            /* 開始接收資料 */
            if (FD_ISSET(client_sockfd, &readfds)) {
                if (recv(client_sockfd, buf, sizeof (buf), 0)) {
                     DEBUG("訊息已接收\n");
                }

                usleep(100);

                printf("%s\n", buf);
            }
        }
    }

    /* 關閉 socket */
    
    close(client_sockfd);
    exit(EXIT_SUCCESS);
    
    return EXIT_SUCCESS;
}


/* 以 select() 實作 non-blocking 的輸入 */
int stdin_ready() {
    fd_set fdset;
    struct timeval timeout;
    int fd;
    
    fd = fileno(stdin);
    FD_ZERO(&fdset);
    FD_SET(fd, &fdset);
    timeout.tv_sec = 0;
    timeout.tv_usec = 1;
    
    usleep(5000);    /* avoid high CPU loading */
    
    return select(fd + 1, &fdset, NULL, NULL, &timeout) == 1 ? 1 : 0;
}
