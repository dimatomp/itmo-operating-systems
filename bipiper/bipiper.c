#include <string.h>
#include <bufio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#define BUFSIZE 4096
#define FAIL(str) {perror(str); return errno;}

void transfer(int from, int to) {
    struct buf_t *buffer = buf_new(BUFSIZE);
    ssize_t fillSize;
    while ((fillSize = buf_fill(from, buffer, 1)) != 0) {
        if (fillSize == -1)
            FAIL("buf_fill");
        if (buf_flush(to, buffer, 1) == -1)
            FAIL("buf_flush");
    }
    close(from);
    close(to);
}

int main(int argc, char *argv[]) {
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    inet_pton(AF_INET, "0.0.0.0", &addr.sin_addr.s_addr);
    int sock1 = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP),
        sock2 = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock1 == -1 || sock2 == -1)
        FAIL("socket");
    addr.sin_port = htons(atoi(argv[1]));
    if (bind(sock1, &addr, sizeof(addr)) == -1)
        FAIL("bind");
    addr.sin_port = htons(atoi(argv[2]));
    if (bind(sock2, &addr, sizeof(addr)) == -1) {
        close(sock1);
        FAIL("bind");
    }
    if (listen(sock1, 10) == -1 || listen(sock2, 10) == -1) {
        close(sock1);
        close(sock2);
        FAIL("listen");
    }
    int client1, client2;
    while ((client1 = accept(sock1, NULL, NULL)) != -1 && (client2 = accept(sock2, NULL, NULL)) != -1) {
        int pid = fork();
        if (pid == -1) {
            close(sock1);
            close(sock2);
            FAIL("fork");
        } else if (pid == 0) {
            close(sock1);
            close(sock2);
            pid = fork();
            if (pid == -1) {
                FAIL("fork");
            } else if (pid == 0) {
                transfer(client1, client2);
            } else {
                transfer(client2, client1);
            }
            return 0;
        } else {
            close(client1);
            close(client2);
        }
    }
    perror("accept");
    return 0;
}
