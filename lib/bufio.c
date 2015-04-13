#include "bufio.h"
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <memory.h>

#ifdef DEBUG
#define abortOnNull(buf) if (buf == NULL) abort()
#else
#define abortOnNull(_)
#endif

struct buf_t {
    size_t capacity;
    size_t size;
    char *content;
};

struct buf_t *buf_new(size_t capacity) {
    char *content = malloc(capacity);
    if (content == NULL)
        return NULL;
    struct buf_t *result = malloc(sizeof(struct buf_t));
    if (result == NULL) {
        free(content);
        return NULL;
    }
    result->capacity = capacity;
    result->size = 0;
    result->content = content;
    return result;
}

void buf_free(struct buf_t *buf) {
    abortOnNull(buf);
    free(buf->content);
    free(buf);
}

size_t buf_capacity(struct buf_t *buf) {
    abortOnNull(buf);
    return buf->capacity;
}

size_t buf_size(struct buf_t *buf) {
    abortOnNull(buf);
    return buf->size;
}

ssize_t buf_fill(int fd, struct buf_t *buf, size_t required) {
#ifdef DEBUG
    if (required > buf->capacity) {
        abort();
    }
#endif
    abortOnNull(buf);
    char *cIndex = buf->content + buf->size;
    while ((buf->size = cIndex - buf->content) < required) {
        ssize_t readCount = read(fd, cIndex, buf->capacity - buf->size);
        if (readCount == -1) {
            return -1;
        }
        if (readCount == 0) {
            break;
        }
        cIndex += readCount;
    }
    return buf->size;
}

ssize_t buf_flush(int fd, struct buf_t *buf, size_t required) {
    abortOnNull(buf);
    char *cIndex = buf->content;
    if (required > buf->size) {
        required = buf->size;
    }
    bool failure = false;
    while (cIndex - buf->content < required) {
        ssize_t writeCount = write(fd, cIndex, required - (cIndex - buf->content));
        if (writeCount == -1) {
            failure = true;
            break;
        }
        cIndex += writeCount;
    }
    size_t oldSize = buf->size;
    buf->size -= (cIndex - buf->content);
    memmove(buf->content, cIndex, buf->size);
    return failure ? -1 : (oldSize - buf->size);
}

#define NEWLINE '\n'

ssize_t buf_getline(int fd, struct buf_t *buf, char *dest) {
    size_t cPos = 0;
    bool endOfFile = false;
    for (;; cPos++) {
        if (cPos >= buf->size) {
            size_t before = buf->size;
            ssize_t result = buf_fill(fd, buf, 1);
            if (result == -1)
                return -1;
            if (result == before) {
                endOfFile = true;
                break;
            }
        }
        dest[cPos] = buf->content[cPos];
        if (buf->content[cPos] == NEWLINE)
            break;
    }
    if (!endOfFile)
        cPos++;
    memmove(buf->content, buf->content + cPos, buf->size - cPos);
    buf->size -= cPos;
    if (endOfFile)
        dest[cPos++] = '\0';
    return cPos;
}

ssize_t buf_write(int fd, struct buf_t *buf, char *src, size_t len) {
    ssize_t result = 0, remaining = len;
    while (remaining > 0) {
        if (buf->capacity - buf->size < remaining) {
            memcpy(buf->content + buf->size, src + (len - remaining),
                    buf->capacity - buf->size);
            remaining -= (buf->capacity - buf->size);
            buf->size = buf->capacity;
            ssize_t cOut = buf_flush(fd, buf, 1);
            if (cOut == -1)
                return -1;
            result += cOut;
        } else {
            memcpy(buf->content + buf->size, src + (len - remaining),
                    remaining);
            buf->size += remaining;
            remaining = 0;
        }
    }
    return result;
}
