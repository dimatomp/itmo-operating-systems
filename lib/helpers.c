#include "helpers.h"
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <string.h>
#include <memory.h>
#include <stdlib.h>

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
    char *data = (char*) buf;
    ssize_t result = count;
    while (count > 0) {
        ssize_t writeResult = write(fd, data + result - count, count);
        if (writeResult == -1)
            return writeResult;
        count -= writeResult;
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
            if (errno != EAGAIN && errno != EWOULDBLOCK)
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

int spawn(const char *file, char* const argv[]) {
    int fileLen = strlen(file);
    char *PATH = getenv("PATH");
    if (PATH == 0)
        return -1;
    char *fullName = calloc(2 * strlen(PATH), sizeof(char));
    char *last = PATH;
    struct stat buf;
    for (;; PATH++) {
        if (*PATH == ':' || *PATH == '\0') {
            strncpy(fullName, last, PATH - last);
            fullName[PATH - last] = '/';
            strncpy(fullName + (PATH - last) + 1, file, fileLen);
            fullName[PATH - last + 1 + fileLen] = 0;
            int statResult = stat(fullName, &buf);
            if (statResult != -1 && S_ISREG(buf.st_mode)) {
                break;
            }
            last = PATH + 1;
        }
        if (*PATH == '\0') {
            break;
        }
    }
    if (S_ISREG(buf.st_mode)) {
        pid_t child = fork();
        if (child == -1) {
            free(fullName);
            return child;
        }
        if (child == 0) {
            execv(fullName, argv);
            int failure = errno;
            free(fullName);
            _exit(failure);
        }
        int c;
        waitpid(child, &c, 0);
        free(fullName);
        return WEXITSTATUS(c);
    }
    return -1;
}
