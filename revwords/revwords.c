#include "helpers.h"
#include "errno.h"
#include "unistd.h"
#include "memory.h"
#include "stdbool.h"

#define BUFFER_SIZE 4097

char buf[BUFFER_SIZE];

int main() {
    int prevPos = 0;
    ssize_t readCount;
    while (true) {
        readCount = read_until(STDIN_FILENO, buf + prevPos, BUFFER_SIZE - prevPos, ' ');
        if (readCount == -1) {
            return errno;
        }
        if (readCount == 0) {
            write_(STDOUT_FILENO, buf, prevPos);
            break;
        }
        int last = 0;
        for (int i = 0; i < prevPos + readCount; i++) {
            if (buf[i] == ' ') {
                for (int j = last; j < (last + i) / 2; j++) {
                    char bj = buf[j];
                    buf[j] = buf[i - 1 - j + last];
                    buf[i - 1 - j + last] = bj;
                }
                write_(STDOUT_FILENO, buf + last, i - last + 1);
                last = i + 1;
            }
        }
        memcpy(buf, buf + last, prevPos + readCount - last);
        prevPos = prevPos + readCount - last;
    }
    return 0;
}
