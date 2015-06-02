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

int main(int argc, char *argv[]) {
    int portNum = atoi(argv[1]);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(portNum);
    inet_pton(AF_INET, "0.0.0.0", &addr.sin_addr.s_addr);
    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == -1)
        FAIL("socket");
    if (bind(sock, &addr, sizeof(addr)) == -1)
        FAIL("bind");
    if (listen(sock, 5) == -1) {
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
