#include <bufio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <netdb.h>

#define BUFSIZE 4096
#define FAIL(str) {perror(str); return errno;}

int main(int argc, char *argv[]) {
    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == -1)
        FAIL("socket");

    struct addrinfo hint;
    hint.ai_flags = 0;
    hint.ai_family = AF_INET;
    hint.ai_socktype = SOCK_STREAM;
    hint.ai_protocol = IPPROTO_TCP;
    struct addrinfo *addr;
    getaddrinfo("localhost", argv[1], &hint, &addr);
    if (bind(sock, addr->ai_addr, addr->ai_addrlen) == -1)
        FAIL("bind");
    freeaddrinfo(addr);

    if (listen(sock, 10) == -1) {
        close(sock);
        FAIL("listen");
    }
    int clientFd;
    while ((clientFd = accept(sock, NULL, NULL)) != -1) {
        int pid = fork();
        if (pid == -1) {
            close(sock);
            FAIL("fork");
        } else if (pid == 0) {
            close(sock);
            int nFile = open(argv[2], O_RDONLY);
            if (nFile == -1)
                FAIL("open");
            struct buf_t *buffer = buf_new(BUFSIZE);
            ssize_t fillSize;
            while ((fillSize = buf_fill(nFile, buffer, 1)) != 0) {
                if (fillSize == -1)
                    FAIL("buf_fill");
                if (buf_flush(clientFd, buffer, 1) == -1)
                    FAIL("buf_flush");
            }
            close(clientFd);
            return 0;
        } else {
            close(clientFd);
        }
    }
    return 0;
}
