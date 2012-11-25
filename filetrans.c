/**
 * filetrans.c : 簡易檔案傳輸程式
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
#include <math.h>

/* 預設緩衝區長度為 1024 bytes */

#define BUFF_LEN 4096

/* 預設開放端口*/

#define PORT 12345

void cls(); 


/* 預設監聽接收檔案 (Server mode)，引數指定檔案傳送 (Client mode) */

int main(int argc, char *argv[]) {
    
    int server_sockfd, client_sockfd, on;
    unsigned int server_len, client_len;
    struct sockaddr_in server_address;
    struct sockaddr_in client_address;
    int result;
    fd_set readfds, testfds;
    int fdmax;
    char buf[BUFF_LEN]; 
    char tmp[BUFF_LEN]; 
    char msg[BUFF_LEN]; 
    int i, j, k, l, m;
    char c;
    

    /*  建立伺服器 socket  */

    if ((server_sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket() 呼叫失敗");
        exit(EXIT_FAILURE);
    }
    
    /* "address already in use" 錯誤訊息 */
    
    if (setsockopt(server_sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof (int)) == -1) {
        perror("setsockopt() 呼叫失敗");
        exit(EXIT_FAILURE);
    }

    /*  設定伺服器 socket  */

    memset(&server_address, 0x00, sizeof (struct sockaddr_in));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(PORT);
    server_len = sizeof (server_address);
    server_len = sizeof (server_address);

    /*  綁定 socket fd 與伺服器位址  */

    if (bind(server_sockfd, (struct sockaddr *)&server_address, server_len) == -1) {
        perror("bind() 呼叫失敗");
        exit(EXIT_FAILURE);

    }

    /*  傾聽 socket fd  */

    if (listen(server_sockfd, 10) == -1) {
        perror("listen() 呼叫失敗");
        exit(EXIT_FAILURE);

    }

    FD_ZERO(&readfds);
    FD_ZERO(&testfds);
    FD_ZERO(&buf);
    FD_ZERO(&tmp);
    FD_ZERO(&msg);
    FD_SET(server_sockfd, &readfds);
    
    /* 紀錄目前的 fd 數量 */
    
    fdmax = server_sockfd;

    DEBUG("除錯模式啟動\n");
    printf("接收模式已啟動，正在等候對方傳輸檔案\n");

    for (;;) {
        int fd;
        int nread;

        /* 複製編號 */
        testfds = readfds;
        fd = 0;
        
        /* 使用 select() 模擬多人傳輸 */

        result = select(fdmax + 1, &testfds, (fd_set *)0, (fd_set *)0, (struct timeval *) 0);

        if (result < 1) {
            perror("伺服器發生問題");
            exit(EXIT_FAILURE);
        }

        /* 遍歷 fd_set */
        
        for (fd = 0; fd <= fdmax; fd++) {
            if (FD_ISSET(fd, &testfds)) {

                /* 處理伺服器端 */
                
                if (fd == server_sockfd) {
                    client_len = sizeof(client_address);
                    client_sockfd = accept(server_sockfd, (struct sockaddr *)&client_address, &client_len);
                    FD_SET(client_sockfd, &readfds);
                    
                    /* 紀錄 file descriptor 最大值 */
                    
                    if (client_sockfd > fdmax) { 
                        fdmax = client_sockfd;
                    }
                    DEBUG("%s: 新連線 %s 於 socket#%d\n", argv[0], inet_ntoa(client_address.sin_addr), client_sockfd);
                    
                } else {
                    
                    /* 處理連入的客戶端 */
                    
                    ioctl(fd, FIONREAD, &nread);

                    if (nread == 0) {        /* 客戶端沒資料或是斷線 */
                        close(fd);
                        FD_CLR(fd, &readfds);
                        DEBUG("客戶端#%d離線\n", fd);
                        
                    } else {                /* 處理客戶端資料 */
                    
                        /* 開始接收資料 */
                        
                        /* 顯示進度 */
                        
                        int i, st, cread, per, dc;
                        dc = 12;
                        char* dot[12] = {"", ".", "..", "...", "....", ".....", "......", ".......", "........", ".........", "..........", "............"};
                        
                        for (i = 0, cread = 1, per = 0; i < nread; i++, cread++) { 
                            st = recv(fd, buf, 1, 0);
                            per = ((float)cread / (float)nread) * 100;
                            
                            j = i % dc;
                            if (i == nread - 1) j = 0;
                            
                            printf("進度：\t %d %c %12s 正在下載 %d / %d bytes\n", per, '%', dot[j], cread, nread);
                            usleep(50000);    //slow mode
                            cls();
                        }
                        
                        /* 處理取得的資料 */
                        
                        usleep(100);

                        printf("傳輸完成\n");
                    }
                }
            }
        }
    }
    
    /* 關閉 socket */
    
    close(server_sockfd);
    exit(EXIT_SUCCESS);
    
    return EXIT_SUCCESS;
}


/* work on ANSI terminals, demands POSIX. */

void cls()
{
    const char* CLEAR_SCREE_ANSI = "\e[1;1H\e[2J";
    write(STDOUT_FILENO, CLEAR_SCREE_ANSI, 12);
}