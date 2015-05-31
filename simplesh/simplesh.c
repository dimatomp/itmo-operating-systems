#include <unistd.h>
#include "helpers.h"
#include "bufio.h"
#include <string.h>

#define BUFSIZE 4097

const char * GREETING = "$";
char command[BUFSIZE];
char *pointers[BUFSIZE];
struct execargs_t *programs[BUFSIZE];

int main() {
    struct buf_t *buffer = buf_new(BUFSIZE);
    int greetLen = strlen(GREETING);
    while (1) {
        int result = write_(STDOUT_FILENO, GREETING, greetLen);
        if (result == -1)
            break;
        int size = buf_getline(STDIN_FILENO, buffer, command);
        if (command[size - 1] == '\0') {
            break;
        }
        int cProg = 0, cWord = 0;
        int i = 0;
        while (i < size && (command[i] == ' ' || command[i] == '\n'))
            i++;
        pointers[cWord] = command + i;
        programs[cProg] = eargs_create(pointers);
        while (i < size) {
            if (command[i] == '|') {
                if (pointers[cWord] != command + i)
                    cWord++;
                pointers[cWord] = NULL;
                do {
                    command[i] = '\0';
                    i++;
                } while (i < size && (command[i] == ' ' || command[i] == '\n'));
                pointers[++cWord] = command + i;
                programs[++cProg] = eargs_create(pointers + cWord);
            } else if (command[i] == ' ' || command[i] == '\n') {
                while (i < size && (command[i] == ' ' || command[i] == '\n')) {
                    command[i] = '\0';
                    i++;
                }
                pointers[++cWord] = command + i;
            } else
                i++;
        }
        if (pointers[cWord] != command + size)
            cWord++;
        pointers[cWord] = NULL;
        runpiped(programs, cProg + 1);
        for (int i = 0; i <= cProg; i++)
            eargs_delete(programs[i]);
    }
    buf_free(buffer);
    return 0;
}
