/**
 * filetrans.c : 簡易檔案傳輸工具程式
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
#include <math.h>
#include <wchar.h>

/* 預設緩衝區長度為 1024 bytes */

#define BUFF_LEN 1024

/* 預設開放端口*/

#define PORT 12345

void cls(); 

void split_path_file(char**, char**, char*);


/* 預設監聽接收檔案 (Server mode)，引數指定檔案傳送 (Client mode) */

int main(int argc, char *argv[]) {
    
    int server_sockfd, client_sockfd, on;
    unsigned int server_len, client_len;
    struct sockaddr_in server_address;
    struct sockaddr_in client_address;
    int result;
    fd_set readfds, testfds;
    int fdmax;
    wchar_t bytebuf[1];
    wchar_t buf[BUFF_LEN]; 
    wchar_t tmp[BUFF_LEN]; 
    wchar_t msg[BUFF_LEN];
    char filename[BUFF_LEN];
    int i, j;
    int fd;
    FILE *fp;
    int st, nread, cread, per, dc;
    dc = 12;
    char* dot[12] = {"", "<", "<", "<", "<<<<", 
                     "<<<<", "<<<<", "<<<<<",
                     "<<<<<<<<", "<<<<<<<<",
                     "<<<<<<<<", "<<<<<<<<"};
                    

    /*  建立伺服器 socket  */

    if ((server_sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        perror("socket() 呼叫失敗");
        exit(EXIT_FAILURE);
    }
    
    /* "address already in use" 錯誤訊息 */
    
    if (setsockopt(server_sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof (int)) == -1) {
        perror("setsockopt() 呼叫失敗");
        exit(EXIT_FAILURE);
    }

    /*  設定伺服器 socket  */

    bzero(&server_address, sizeof (struct sockaddr_in));
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
    FD_SET(server_sockfd, &readfds);
    
    /* 紀錄目前的 fd 數量 */
    
    fdmax = server_sockfd;

    DEBUG("除錯模式啟動\n");
    printf("接收模式已啟動，正在等候對方傳輸檔案\n");

    for (;;) {
               
        /* 複製編號 */
        
        testfds = readfds;
        fd = 0;
        
        /* 使用 select() 模擬多人傳輸 */
        
        result = select(fdmax + 1, &testfds, (fd_set *) 0, (fd_set *) 0, (struct timeval *) 0);

        if (result < 1) {
            perror("伺服器發生問題");
            exit(EXIT_FAILURE);
        }
        
        /* 遍歷 fd_set */
        
        for (fd = 0; fd <= fdmax; fd++) {
            if (FD_ISSET(fd, &testfds)) {
  
                bzero(bytebuf, 1);
                bzero(buf, BUFF_LEN);
                bzero(tmp, BUFF_LEN);
                bzero(msg, BUFF_LEN);
                bzero(filename, BUFF_LEN);
                bzero(&nread, sizeof (int));
                
                /* 處理伺服器端 */
                
                if (fd == server_sockfd) {
                    client_len = sizeof(client_address);
                    client_sockfd = accept(server_sockfd, (struct sockaddr *) &client_address, &client_len);
                    FD_SET(client_sockfd, &readfds);
                    
                    /* 紀錄 file descriptor 最大值 */
                    
                    if (client_sockfd > fdmax) { 
                        fdmax = client_sockfd;
                    }
                    DEBUG("%s: 新連線 %s 於 socket#%d\n", argv[0], inet_ntoa(client_address.sin_addr), client_sockfd);
                    
                } else {
          
                    /* 開始接收資料 */
                    
                    ioctl(fd, FIONREAD, &nread);

                    if (nread == 0) {        /* 客戶端沒資料或是斷線 */
                        close(fd);
                        FD_CLR(fd, &readfds);
                        
                        DEBUG("客戶端#%d離線\n", fd);
                        
                    } else {                /* 處理客戶端資料 */
                        
                        /* 接收檔案名稱與大小 */

                        st = recv(fd, filename, BUFF_LEN, 0);
                        if (!st) break;
                        
                        DEBUG("File name: %s\n", filename);
                        
                        st = recv(fd, &nread, sizeof (int), 0);
                        if (!st) break;
                        
                        DEBUG("File size: %d\n", nread);
                        
                        /* 存成檔案 */
                        
                        /* 處理檔案路徑 */
                        /* malloc two buffs */
                        char* _path = malloc(BUFF_LEN);
                        char*_fname = malloc(BUFF_LEN);
                        
                        split_path_file(&_path, &_fname, filename);
                        strcpy(filename, _fname);
                        
                        chdir("./downloads");
                        
                        fp = fopen(filename, "wb, ccs=UTF-8");
                        
                        /* 顯示進度 */

                        for (i = 0, cread = 1, per = 0; i < nread; i = i + 1, cread = cread + 1) {
                            
                            bzero(bytebuf, sizeof (wchar_t*));
                            
                            st = recv(fd, bytebuf, 1, 0);
                            if (st <= 0) break;
                            
                            per = ((float) cread / (float) nread) * 100;
                            j = i % dc;
                            if (i == nread - 1) {
                                j = dc - 1;
                            }
                            
                            /* 處理取得的資料 */
                            
                            fwrite(bytebuf, 1, 1, fp);
                            
                            cls(); 
                            printf("\r進度：%3d %c %8s 正在下載 \"%s\" %d / %d bytes\n", per, '%', dot[j], filename, cread, nread);
                            usleep(250);    /* 避免 CPU 佔用過高與畫面閃爍 */
   
                        }

                        /* 關閉相關資源 */
                        
                        fclose(fp);
                        chdir("../");

                        close(fd);
                        FD_CLR(fd, &readfds);

                        if (cread == nread) {
                            printf("傳輸完成\n");
                        } else {
                            printf("傳輸失敗，客戶端關閉連線。\n");
                        }
                        
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


void split_path_file(char** p, char** f, char* pf) {
    char *slash = pf, *next;
    while ((next = strpbrk(slash + 1, "\\/"))) slash = next;
    if (pf != slash) slash++;
    *p = strndup(pf, slash - pf);
    *f = strdup(slash);
}