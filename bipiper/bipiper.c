#include <bufio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <errno.h>
#include <netdb.h>

#define BUFSIZE 4096
#define FAIL(str) {perror(str); return errno;}

int transfer(int from, int to) {
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
    return 0;
}

int main(int argc, char *argv[]) {
    int sock1 = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP),
        sock2 = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock1 == -1 || sock2 == -1)
        FAIL("socket");

    struct addrinfo hint;
    hint.ai_flags = 0;
    hint.ai_family = AF_INET;
    hint.ai_socktype = SOCK_STREAM;
    hint.ai_protocol = IPPROTO_TCP;
    struct addrinfo *result;
    getaddrinfo("localhost", argv[1], &hint, &result);
    if (bind(sock1, result->ai_addr, result->ai_addrlen) == -1)
        FAIL("bind");
    freeaddrinfo(result);
    getaddrinfo("localhost", argv[2], &hint, &result);
    if (bind(sock2, result->ai_addr, result->ai_addrlen) == -1) {
        close(sock1);
        FAIL("bind");
    }
    freeaddrinfo(result);

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
                return transfer(client1, client2);
            } else {
                return transfer(client2, client1);
            }
        } else {
            close(client1);
            close(client2);
        }
    }
    perror("accept");
    return 0;
}
