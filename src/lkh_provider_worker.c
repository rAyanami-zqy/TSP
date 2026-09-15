/*
 * Persistent, process-isolated launcher for a locally linked LKH build.
 *
 * The provider parent does not call LKH. Each parameter-file request is run
 * by a forked child, preventing LKH's global/static state from leaking into
 * the next instance while avoiding a new exec and dynamic-loader pass.
 */
#include <errno.h>
#include <fcntl.h>
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
            char *log_path = malloc(length + 5);
            if (log_path != NULL) {
                memcpy(log_path, line, length);
                memcpy(log_path + length, ".log", 5);
                const int log_fd = open(log_path, O_WRONLY | O_CREAT | O_TRUNC, 0600);
                if (log_fd >= 0) {
                    (void)dup2(log_fd, STDOUT_FILENO);
                    (void)dup2(log_fd, STDERR_FILENO);
                    close(log_fd);
                }
            }
            char *arguments[] = {"LKH", line, NULL};
            _exit(LKH_entry(2, arguments));
        }
        if (child < 0) {
            printf("DONE 255 0 fork:%s\n", strerror(errno));
            continue;
        }
        int status = 0;
        while (waitpid(child, &status, 0) < 0 && errno == EINTR) {
        }
        const int code = WIFEXITED(status) ? WEXITSTATUS(status)
                                          : 128 + WTERMSIG(status);
        printf("DONE %d %.9f\n", code, now_seconds() - started);
    }
    free(line);
    return 0;
}
