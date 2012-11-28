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
#include <wchar.h>

/* 預設緩衝區長度為 1024 bytes */

#define BUFF_LEN 1024

/* 預設伺服器位址 */
#define SERVER "127.0.0.1"

/* 預設開放端口*/

#define PORT 12345

void cls(void);

void split_path_file(char**, char**, char*);


int main(int argc, char *argv[]) {
    
    int client_sockfd;
    int len;
    struct sockaddr_in address;
    char server[UCHAR_MAX];
    int result;
    wchar_t bytebuf[1];
    wchar_t buf[BUFF_LEN]; 
    wchar_t tmp[BUFF_LEN];
    char filename[BUFF_LEN];
    fd_set readfds;
    int i, j;

    DEBUG("除錯模式啟動\n");

    /*  建立客戶端 socket  */

    if ((client_sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        perror("socket() 呼叫失敗");
        exit(EXIT_FAILURE);
    }

    /*  設定客戶端 socket  */
    
    /* 使用者提供伺服器資訊與檔案名稱 */
    
    bzero(&filename, BUFF_LEN);
    
    if (argc > 2) {
        /* 使用參數模式 */
        strcpy(server, argv[1]);
        printf("正在連線到 %s:%d ...\n", server, PORT);
        
        if (argc >= 2) {
            strcpy(filename, argv[2]);
        } else {
            printf("請提供檔案名稱！\n");
            perror("argv[2] 呼叫失敗");
            exit(EXIT_FAILURE);   
        }
        
    } else {
        /* 使用預設伺服器資訊 */
        strcpy(server, SERVER);
        
        if (argc > 1) {
            strcpy(filename, argv[1]);
        } else {
            printf("請提供檔案名稱！\n");
            perror("argv[1] 呼叫失敗");
            exit(EXIT_FAILURE);   
        }
        
    }

    bzero(&address, sizeof (struct sockaddr_in));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(SERVER);
    address.sin_port = htons(PORT);
    len = sizeof (address);

    /* 與伺服器建立連線 */

    if ((result = connect(client_sockfd, (struct sockaddr *) &address, len)) < 0) {
        perror("connect() 呼叫失敗"); 
        close(client_sockfd);
        exit(EXIT_FAILURE);

    } else {
        printf("連線中...\n");
    }
    
    FD_ZERO(&readfds);
    FD_SET(client_sockfd, &readfds);
    
    /* 開始傳送接收資料 */

    /* 開啟使用者指定的檔案 */
    
    FILE *fp;
    int st, nread, cread, per, dc;
    dc = 12;
    char* dot[12] = {"", "<", "<", "<", "<<<<", 
                     "<<<<", "<<<<", "<<<<<",
                     "<<<<<<<<", "<<<<<<<<",
                     "<<<<<<<<", "<<<<<<<<"};
    
    bzero(bytebuf, 1);           
    bzero(buf, BUFF_LEN);
    bzero(tmp, BUFF_LEN);
    
    /* 處理檔案路徑 */
    /* malloc two buffs */
    char* _path = malloc(BUFF_LEN);
    char*_fname = malloc(BUFF_LEN);
    
    split_path_file(&_path, &_fname, filename);
    strcpy(filename, _fname);
    chdir(_path);
    
    DEBUG("File name: %s\n", filename);
    
    fp = fopen(filename, "rb, ccs=UTF-8");

    /* 開始傳送資料 */
    
    /*ioctl(client_sockfd, FIONREAD, &nread);*/
    
    /* 取得檔案大小 */
    
    fseek(fp, 0L, SEEK_END);
    nread = ftell(fp);
    fseek(fp, 0L, SEEK_SET);
    
    DEBUG("File size: %d\n", nread);
    
    /* 傳送檔案名稱與檔案大小給對方 */
    
    if (send(client_sockfd, filename, BUFF_LEN, 0)) {
        DEBUG("Filename 訊息已送出\n");
    }
    
    if (send(client_sockfd, &nread, sizeof (int), 0)) {
        DEBUG("Filesize 訊息已送出\n");
    }

    for (i = 0, cread = 1, per = 0; i < nread; i = i + 1, cread = cread + 1) { 
        
        bzero(bytebuf, sizeof (wchar_t*));
        
        if (!feof(fp)) {
            st = fread(bytebuf, 1, 1, fp);
        }
               
        st = send(client_sockfd, bytebuf, 1, 0);
        
        per = ((float) cread / (float) nread) * 100;
        j = i % dc;
        if (i == nread - 1) {
            j = dc - 1;
        }
        
        cls();
        printf("\r進度：%3d %c %8s 正在上傳 \"%s\" %d / %d bytes\n", per, '%', dot[j], filename, cread, nread);
        usleep(250);    /* 避免 CPU 佔用過高與畫面閃爍 */

    }
    
    fclose(fp);
    
    printf("傳輸完成\n");
    
    chdir("..");
    
    /* 關閉 socket */
    
    close(client_sockfd);
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