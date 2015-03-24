#include <helpers.h>
#include <errno.h>
#include <unistd.h>
#include <memory.h>
#include <stdbool.h>

#define BUFFER_SIZE 4096

const char DELIMITER = '\n';

char buf[BUFFER_SIZE];
char *exec, **argValues;
int argCount;

bool processWord(char *buf, int len) {
    buf[len] = '\0';
    argValues[argCount - 1] = buf;
    bool result = (spawn(exec, argValues) == 0);
    if (result) {
        write_(STDOUT_FILENO, buf, len);
    }
    buf[len] = DELIMITER;
    return result;
}

int main(int argc, char *argv[]) {
    argCount = argc;
    exec = argv[1];
    memmove(argv, argv + 1, argc - 1);
    argValues = argv;
    int prevPos = 0;
    ssize_t readCount;
    while (true) {
        readCount = read_until(STDIN_FILENO, buf + prevPos, BUFFER_SIZE - prevPos, DELIMITER);
        if (readCount == -1) {
            return errno;
        }
        if (readCount == 0) {
            processWord(buf, prevPos);
            break;
        }
        int last = 0;
        for (int i = 0; i < prevPos + readCount; i++) {
            if (buf[i] == DELIMITER) {
                if (processWord(buf + last, i - last)) {
                    write_(STDOUT_FILENO, &DELIMITER, 1);
                }
                last = i + 1;
            }
        }
        memmove(buf, buf + last, prevPos + readCount - last);
        prevPos = prevPos + readCount - last;
    }
    return 0;
}
