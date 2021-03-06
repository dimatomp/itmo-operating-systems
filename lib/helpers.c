#include "helpers.h"
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <signal.h>
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

struct execargs_t {
    char **argv;
};

struct execargs_t *eargs_create(char **argv) {
    struct execargs_t *result = malloc(sizeof(struct execargs_t));
    if (result != NULL) {
        result->argv = argv;
    }
    return result;
}

void eargs_delete(struct execargs_t *eargs) {
    free(eargs);
}

int exec(struct execargs_t *eargs) {
    return execvp(eargs->argv[0], eargs->argv);
}

static int processCnt;
static int *processIds;

static void handleSigint(int signal) {
    for (int i = 0; i < processCnt; i++) {
        kill(processIds[i], SIGINT);
    }
}

int runpiped(struct execargs_t **programs, size_t n) {
    int pids[n];
    processCnt = n;
    processIds = pids;
    int prevStdin = dup(STDIN_FILENO);
    int stdout_before = dup(STDOUT_FILENO);
    if (prevStdin == -1 || stdout_before == -1) {
        int result = errno;
        close(prevStdin);
        close(stdout_before);
        errno = result;
        return -1;
    }
    struct sigaction prevInt;
    struct sigaction action;
    action.sa_handler = handleSigint;
    sigemptyset(&action.sa_mask);
    action.sa_flags = SA_RESTART;
    sigaction(SIGINT, &action, &prevInt);
    int pipes[2];
    for (int i = 0; i < n; i++) {
        if (i < n - 1) {
            if (pipe(pipes) == -1) {
                int result = errno;
                for (int j = 0; j < i; j++) {
                    kill(pids[i], SIGKILL);
                    waitpid(pids[i], NULL, 0);
                }
                close(prevStdin);
                close(stdout_before);
                errno = result;
                return -1;
            }
        } else {
            pipes[1] = stdout_before;
        }
        int pid = fork();
        if (pid == -1) {
            int result = errno;
            for (int j = 0; j < i; j++) {
                kill(pids[i], SIGKILL);
                waitpid(pids[i], NULL, 0);
            }
            close(prevStdin);
            close(stdout_before);
            errno = result;
            return -1;
        } else if (pid == 0) {
            if (i < n - 1)
                close(pipes[0]);
            if (dup2(prevStdin, STDIN_FILENO) == -1
                    || dup2(pipes[1], STDOUT_FILENO) == -1
                    || close(prevStdin) == -1
                    || close(pipes[1]) == -1
                    || exec(programs[i]) == -1) {
                _exit(errno);
            }
        } else {
            if (close(prevStdin) == -1 || close(pipes[1]) == -1) {
                int result = errno;
                for (int j = 0; j <= i; j++) {
                    kill(pids[i], SIGKILL);
                    waitpid(pids[i], NULL, 0);
                }
                close(stdout_before);
                errno = result;
                return -1;
            }
            prevStdin = pipes[0];
            pids[i] = pid;
        }
    }
    for (int i = 0; i < n; i++)
        wait(NULL);
    sigaction(SIGINT, &prevInt, NULL);
    return 0;
}
