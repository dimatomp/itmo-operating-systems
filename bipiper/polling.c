#include <bufio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <errno.h>
#include <netdb.h>
#include <stdbool.h>
#include <poll.h>
#include <fcntl.h>

#define MAXNFD 256
#define BUFSIZE 4096
#define FAIL(str) {perror(str); return errno;}

struct pollfd pollClients[MAXNFD];
struct buf_t *buffers[MAXNFD];
bool occupied[MAXNFD];
int lClients = 0, rClients = 0;

void handleServer(int num, int plus, int o_plus) {
    if (pollClients[num].fd >= 0 && (pollClients[num].revents & POLLIN)) {
        int pos = 0, fnOccupied = -1;
        while (pos < MAXNFD / 2 - 1) {
            if (fnOccupied == -1 && !occupied[pos + plus])
                fnOccupied = pos;
            if (!occupied[pos + plus] && occupied[pos + o_plus])
                break;
            pos++;
        }
        if (pos < MAXNFD / 2 - 1) {
            pollClients[pos + plus].fd = accept(pollClients[num].fd, NULL, 0);
            int flags = fcntl(pollClients[pos + plus].fd, F_GETFD);
            fcntl(pollClients[pos + plus].fd, F_SETFD, flags | O_NONBLOCK);
            pollClients[pos + o_plus].fd = -pollClients[pos + o_plus].fd;
            buffers[pos + plus] = buf_new(BUFSIZE);
            occupied[pos + plus] = true;
        } else {
            pollClients[fnOccupied + plus].fd = -accept(pollClients[num].fd, NULL, 0);
            int flags = fcntl(-pollClients[pos + plus].fd, F_GETFD);
            fcntl(-pollClients[pos + plus].fd, F_SETFD, flags | O_NONBLOCK);
            buffers[fnOccupied + plus] = buf_new(BUFSIZE);
            occupied[fnOccupied + plus] = true;
        }
        if (++lClients >= MAXNFD / 2 - 1) {
            pollClients[num].fd = -pollClients[num].fd;
        }
    }
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
    pollClients[0].fd = sock1;
    pollClients[1].fd = sock2;
    pollClients[0].events = pollClients[1].events = POLLIN;
    for (int i = 2; i < MAXNFD; i++) {
        pollClients[i].fd = -1;
        pollClients[i].events = POLLIN | POLLOUT;
    }
    while (poll(pollClients, MAXNFD, -1) != -1) {
        for (int i = 0; i < MAXNFD / 2 - 1; i++) 
            if (pollClients[i + 2].fd >= 0 && pollClients[i + MAXNFD / 2 + 1].fd >= 0){
                if ((pollClients[i + 2].revents | pollClients[i + MAXNFD / 2 + 1].revents) & (POLLERR | POLLHUP)) {
                    close(pollClients[i + 2].fd);
                    close(pollClients[i + MAXNFD / 2 + 1].fd);
                    buf_free(buffers[i + 2]);
                    buf_free(buffers[i + MAXNFD / 2 + 1]);
                    occupied[i + 2] = false;
                    occupied[i + MAXNFD / 2 + 1] = false;
                    if (lClients-- == MAXNFD / 2 - 1)
                        pollClients[0].fd = -pollClients[0].fd;
                    if (rClients-- == MAXNFD / 2 - 1)
                        pollClients[1].fd = -pollClients[1].fd;
                    pollClients[i + 2].fd = -1;
                    pollClients[i + MAXNFD / 2 + 1].fd = -1;
                } else {
                    if (pollClients[i + 2].revents & POLLIN) {
                        buf_fill(pollClients[i + 2].fd, buffers[i + 2], 1);
                    }
                    if ((pollClients[i + 2].revents & POLLOUT) && buf_size(buffers[i + 2]) > 0) {
                        buf_flush(pollClients[i + MAXNFD / 2 + 1].fd, buffers[i + 2], 1);
                    }
                    if (pollClients[i + MAXNFD / 2 + 1].revents & POLLIN) {
                        buf_fill(pollClients[i + MAXNFD / 2 + 1].fd, buffers[i + MAXNFD / 2 + 1], 1);
                    }
                    if ((pollClients[i + MAXNFD / 2 + 1].revents & POLLOUT) && buf_size(buffers[i + MAXNFD / 2 + 1]) > 0) {
                        buf_flush(pollClients[i + 2].fd, buffers[i + MAXNFD / 2 + 1], 1);
                    }
                }
            }
        if ((pollClients[0].revents | pollClients[1].revents) & (POLLERR | POLLHUP))
            FAIL("poll server");
        handleServer(0, 2, MAXNFD / 2 + 1);
        handleServer(1, MAXNFD / 2 + 1, 2);
    }
    if (errno == EINTR) {
        close(sock1);
        close(sock2);
    } else
        FAIL("poll");
    return 0;
}
