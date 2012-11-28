/**
 * filetrans.h : 簡易檔案傳輸工具程式
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

#ifndef FILETRANS_H
#define FILETRANS_H

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

/* 預設伺服器位址 */

#define SERVER "127.0.0.1"

/* 預設開放端口*/

#define PORT 54321

#define DOT_LEN 12


int server_sockfd, client_sockfd, on;
unsigned int server_len, client_len;
struct sockaddr_in server_address;
struct sockaddr_in client_address;
int result, len;
fd_set readfds, testfds;
int fdmax;
wchar_t bytebuf[1];
wchar_t buf[BUFF_LEN]; 
wchar_t tmp[BUFF_LEN]; 
wchar_t msg[BUFF_LEN];
char server[BUFF_LEN];
char filename[BUFF_LEN];
int i, j;
int fd;
FILE *fp;
int st, nread, cread, per;

static char* const dot[] = {"", "<", "<", "<", "<<<<", 
                 "<<<<", "<<<<", "<<<<<",
                 "<<<<<<<<", "<<<<<<<<",
                 "<<<<<<<<", "<<<<<<<<"}; 

void cls(); 

void split_path_file(char**, char**, char*);

void timestamp(char*);



#endif /* FILETRANS_H */
