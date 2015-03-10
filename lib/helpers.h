#ifndef HELPERS_H
#define HELPERS_H

#include "sys/types.h"

#ifdef __cplusplus
extern "C" {
#endif
    ssize_t read_(int fd, void *buf, size_t count);
    ssize_t write_(int fd, const void *buf, size_t count);
#ifdef __cplusplus
}
#endif

#endif /* HELPERS_H */
