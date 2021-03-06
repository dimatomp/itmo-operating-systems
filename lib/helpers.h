#ifndef HELPERS_H
#define HELPERS_H

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif
    struct execargs_t;

    ssize_t read_(int fd, void *buf, size_t count);
    ssize_t write_(int fd, const void *buf, size_t count);

    ssize_t read_until(int fd, void *buf, size_t count, char delimiter);

    int spawn(const char *file, char* const argv []);

    struct execargs_t *eargs_create(char **argv);
    void eargs_delete(struct execargs_t *eargs);

    int exec(struct execargs_t *eargs);
    int runpiped(struct execargs_t **programs, size_t n);
#ifdef __cplusplus
}
#endif

#endif /* HELPERS_H */
