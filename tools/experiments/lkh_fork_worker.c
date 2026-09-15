/*
 * Persistent launcher for a library-linked LKH build.
 *
 * The parent never calls LKH directly. Each input line is a parameter-file
 * path; a forked child calls the renamed LKH entry point and exits. This keeps
 * every job isolated from LKH's process-global/static state while avoiding a
 * fresh exec and executable loading for every instance.
 */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

int LKH_entry(int argc, char **argv);

static double now_seconds(void)
{
    struct timespec value;
    clock_gettime(CLOCK_MONOTONIC, &value);
    return value.tv_sec + value.tv_nsec * 1e-9;
}

int main(void)
{
    char *line = NULL;
    size_t capacity = 0;
    setvbuf(stdout, NULL, _IOLBF, 0);
    puts("READY");
    while (getline(&line, &capacity, stdin) >= 0) {
        size_t length = strlen(line);
        while (length
               && (line[length - 1] == '\n' || line[length - 1] == '\r')) {
            line[--length] = '\0';
        }
        if (!strcmp(line, "QUIT")) break;
        if (!length) continue;

        const double started = now_seconds();
        const pid_t child = fork();
        if (child == 0) {
            freopen("/dev/null", "w", stdout);
            freopen("/dev/null", "w", stderr);
            char *arguments[] = {"LKH", line, NULL};
            _exit(LKH_entry(2, arguments));
        }
        if (child < 0) {
            printf("DONE 255 0 fork:%s\n", strerror(errno));
            continue;
        }
        int status = 0;
        while (waitpid(child, &status, 0) < 0 && errno == EINTR) {}
        const int code = WIFEXITED(status) ? WEXITSTATUS(status) : 255;
        printf("DONE %d %.9f\n", code, now_seconds() - started);
    }
    free(line);
    return 0;
}
