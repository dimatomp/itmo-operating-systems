#include "helpers.h"
#include "unistd.h"

ssize_t read_(int fd, void *buf, size_t count) {
    ssize_t sum = 0, readResult;
    do {
        readResult = read(fd, buf, count - sum);
        if (readResult == -1)
            return readResult;
        buf += readResult;
        sum += readResult;
    } while (readResult > 0);
    return sum;
}

ssize_t write_(int fd, const void *buf, size_t count) {
    ssize_t result = count;
    while (count > 0) {
        ssize_t writeResult = write(fd, buf, count);
        if (writeResult == -1)
            return writeResult;
        count -= writeResult;
    }
    return result;
}
