#include <helpers.h>
#include <bufio.h>
#include <unistd.h>
#include <memory.h>
#include <stdbool.h>

#define CAPACITY 4096

struct buf_t *inBuf, *outBuf;
char line[CAPACITY];

int main(int argc, char *argv[]) {
    inBuf = buf_new(CAPACITY);
    outBuf = buf_new(CAPACITY);
    if (inBuf == 0 || outBuf == 0)
        return 1;
    memmove(argv, argv + 1, sizeof(char*) * (argc - 1));
    argv[argc - 1] = line;
    while (1) {
        ssize_t newLine = buf_getline(STDIN_FILENO, inBuf, line);
        if (newLine == -1 || (newLine == 1 && line[0] == '\0'))
            break;
        bool eof = (line[newLine - 1] == '\0');
        line[newLine - 1] = '\0';
        int result = spawn(argv[0], argv);
        if (!eof)
            line[newLine - 1] = '\n';
        else
            newLine--;
        if (result == 0)
            buf_write(STDOUT_FILENO, outBuf, line, newLine);
    }
    buf_flush(STDOUT_FILENO, outBuf, buf_size(outBuf));
    buf_free(inBuf);
    buf_free(outBuf);
    return 0;
}
