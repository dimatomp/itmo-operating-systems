#include "helpers.h"
#include "unistd.h"
#include "errno.h"
#include "stdbool.h"

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
        buf += writeResult;
    }
    return result;
}

ssize_t read_until(int fd, void *buf, size_t count, char delimiter) {
    char *text = (char*) buf;
    ssize_t sum = 0;
    bool spaceEncountered = false;
    do {
        ssize_t result = read(fd, buf + sum, count - sum);
        if (result == 0)
            break;
        if (result == -1) {
            if (errno != EAGAIN)
                return result;
            result = 0;
        }
        for (ssize_t i = sum; i < sum + result && !spaceEncountered; i++) {
            spaceEncountered |= (text[i] == delimiter);
        }
        sum += result;
    } while (!spaceEncountered && sum < count);
    return sum;
}
