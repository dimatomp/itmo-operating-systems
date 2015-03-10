#include "helpers.h"
#include "errno.h"
#include "unistd.h"

#define BUFFER_SIZE 256

char readBuf[BUFFER_SIZE];

int main() {
    ssize_t readCount;
    do {
        readCount = read_(STDIN_FILENO, readBuf, BUFFER_SIZE);
        if (readCount == -1)
            return errno;
        write_(STDOUT_FILENO, readBuf, readCount);
    } while (readCount > 0);
    return 0;
}
