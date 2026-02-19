
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>

#define MAX_LINE 1024
#define MAX_ARGS 128

// remove trailing newline
static void trim_newline(char *s) {
    s[strcspn(s, "\n")] = 0;
}

// free malloc'd args and redirection strings
static void cleanup(char *argv[], int argc, char *inFile, char *outFile) {
    for (int i = 0; i < argc; i++) {
        free(argv[i]);
    }
    if (inFile) free(inFile);
    if (outFile) free(outFile);
}

// parse input line into argv and detect <, >, >>
static int parse_line(const char *line, char *argv[], int max_args,
                      char **inFile, char **outFile, int *append) {
    int argc = 0;
    int i = 0;
    int len = (int)strlen(line);

    *inFile = NULL;
    *outFile = NULL;
    *append = 0;

    while (i < len) {
        // skip spaces/tabs
        while (i < len && (line[i] == ' ' || line[i] == '\t')) i++;
        if (i >= len) break;

        // input redirect <
        if (line[i] == '<') {
            i++;
            while (i < len && (line[i] == ' ' || line[i] == '\t')) i++;
            if (i >= len) return -1;

            char *fname = malloc(MAX_LINE);
            if (!fname) return -1;

            int k = 0;
            if (line[i] == '"') {
                i++;
                while (i < len && line[i] != '"' && k < MAX_LINE - 1) fname[k++] = line[i++];
                if (i < len && line[i] == '"') i++;
            } else {
                while (i < len && line[i] != ' ' && line[i] != '\t' && k < MAX_LINE - 1) fname[k++] = line[i++];
            }
            fname[k] = '\0';
            *inFile = fname;
            continue;
        }

        // output redirect > or >>
        if (line[i] == '>') {
            i++;
            if (i < len && line[i] == '>') {
                *append = 1;
                i++;
            } else {
                *append = 0;
            }

            while (i < len && (line[i] == ' ' || line[i] == '\t')) i++;
            if (i >= len) return -1;

            char *fname = malloc(MAX_LINE);
            if (!fname) return -1;

            int k = 0;
            if (line[i] == '"') {
                i++;
                while (i < len && line[i] != '"' && k < MAX_LINE - 1) fname[k++] = line[i++];
                if (i < len && line[i] == '"') i++;
            } else {
                while (i < len && line[i] != ' ' && line[i] != '\t' && k < MAX_LINE - 1) fname[k++] = line[i++];
            }
            fname[k] = '\0';
            *outFile = fname;
            continue;
        }

        // normal argument (quoted or not)
        if (argc >= max_args - 1) break;

        char *arg = malloc(MAX_LINE);
        if (!arg) return -1;

        int k = 0;
        if (line[i] == '"') {
            i++;
            while (i < len && line[i] != '"' && k < MAX_LINE - 1) arg[k++] = line[i++];
            if (i < len && line[i] == '"') i++;
        } else {
            while (i < len && line[i] != ' ' && line[i] != '\t' && k < MAX_LINE - 1) {
                if (line[i] == '<' || line[i] == '>') break;
                arg[k++] = line[i++];
            }
        }
        arg[k] = '\0';

        if (arg[0] == '\0') {
            free(arg);
            continue;
        }

        argv[argc++] = arg;
    }

    argv[argc] = NULL;
    return argc;
}

int main() {
    char line[MAX_LINE];

    while (1) {
        printf("myshell> ");
        fflush(stdout);

        if (!fgets(line, sizeof(line), stdin)) break;
        trim_newline(line);

        if (strlen(line) == 0) continue;

        char *argv[MAX_ARGS];
        char *inFile = NULL;
        char *outFile = NULL;
        int append = 0;

        int argc = parse_line(line, argv, MAX_ARGS, &inFile, &outFile, &append);
        if (argc < 0) {
            fprintf(stderr, "Parse error.\n");
            cleanup(argv, 0, inFile, outFile);
            continue;
        }
        if (argc == 0) {
            cleanup(argv, 0, inFile, outFile);
            continue;
        }

        // built-in: exit
        if (strcmp(argv[0], "exit") == 0) {
            cleanup(argv, argc, inFile, outFile);
            break;
        }

        // built-in: cd
        if (strcmp(argv[0], "cd") == 0) {
            const char *target;

            if (argc == 1) {
                target = getenv("HOME");
                if (!target) target = "/";
            } else {
                target = argv[1];
            }

            if (chdir(target) != 0) {
                fprintf(stderr, "cd: %s: %s\n", target, strerror(errno));
            }

            cleanup(argv, argc, inFile, outFile);
            continue;
        }

        // fork for external commands
        pid_t pid = fork();
        if (pid < 0) {
            fprintf(stderr, "fork failed: %s\n", strerror(errno));
            cleanup(argv, argc, inFile, outFile);
            continue;
        }

        if (pid == 0) {
            // child handles redirection

            if (inFile) {
                int fd = open(inFile, O_RDONLY);
                if (fd < 0) {
                    fprintf(stderr, "input redirect: %s: %s\n", inFile, strerror(errno));
                    exit(1);
                }
                dup2(fd, STDIN_FILENO);
                close(fd);
            }

            if (outFile) {
                int flags = O_WRONLY | O_CREAT;
                if (append) flags |= O_APPEND;
                else flags |= O_TRUNC;

                int fd = open(outFile, flags, 0644);
                if (fd < 0) {
                    fprintf(stderr, "output redirect: %s: %s\n", outFile, strerror(errno));
                    exit(1);
                }
                dup2(fd, STDOUT_FILENO);
                close(fd);
            }

            execvp(argv[0], argv);

            // if execvp returns, it failed
            fprintf(stderr, "%s: command not found\n", argv[0]);
            exit(127);
        } else {
            int status;
            waitpid(pid, &status, 0);
        }

        cleanup(argv, argc, inFile, outFile);
    }

    printf("\nGoodbye!\n");
    return 0;
}
