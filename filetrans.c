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

#include "filetrans.h"
#include "server.h"
#include "client.h"


/* 預設監聽接收檔案 (Server mode)，引數指定檔案傳送 (Client mode) */

int main(int argc, char *argv[]) {
                    
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
        
        client_mode(server, filename);
        
    } else {
        /* 使用預設伺服器資訊 */
        strcpy(server, SERVER);
        
        if (argc > 1) {
            strcpy(filename, argv[1]);
            
            client_mode(server, filename);
            
        } else {
            printf("以伺服器模式執行...\n");

            server_mode();
        }
        
    }
    
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
