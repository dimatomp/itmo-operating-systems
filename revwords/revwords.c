#include "helpers.h"
#include "errno.h"
#include "unistd.h"
#include "memory.h"
#include "stdbool.h"

#define BUFFER_SIZE 4097

char buf[BUFFER_SIZE];

void reverse(char *buf, int len) {
    for (int j = 0; j < len / 2; j++) {
        char bj = buf[j];
        buf[j] = buf[len - j - 1];
        buf[len - j - 1] = bj;
    }
    write_(STDOUT_FILENO, buf, len);
}

const char DELIMITER = ' ';

int main() {
    int prevPos = 0;
    ssize_t readCount;
    while (true) {
        readCount = read_until(STDIN_FILENO, buf + prevPos, BUFFER_SIZE - prevPos, DELIMITER);
        if (readCount == -1) {
            return errno;
        }
        if (readCount == 0) {
            reverse(buf, prevPos);
            break;
        }
        int last = 0;
        for (int i = 0; i < prevPos + readCount; i++) {
            if (buf[i] == ' ') {
                reverse(buf + last, i - last);
                write_(STDOUT_FILENO, &DELIMITER, 1);
                last = i + 1;
            }
        }
        memcpy(buf, buf + last, prevPos + readCount - last);
        prevPos = prevPos + readCount - last;
    }
    return 0;
}
