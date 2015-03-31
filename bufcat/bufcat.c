#include <bufio.h>
#include <unistd.h>

#define CAPACITY 4096

int main() {
    struct buf_t *buffer = buf_new(CAPACITY);
    while (1) {
        ssize_t readCount = buf_fill(STDIN_FILENO, buffer, CAPACITY);
        ssize_t writeCount = buf_flush(STDOUT_FILENO, buffer, CAPACITY);
        if (readCount == -1 || readCount == 0 || writeCount == -1) {
            break;
        }
    }
    buf_free(buffer);
    return 0;
}
