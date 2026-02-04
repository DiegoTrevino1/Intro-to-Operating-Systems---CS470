#include <unistd.h>     // fork, execvp, getpid
#include <sys/wait.h>   // waitpid, WIFEXITED, WEXITSTATUS, WIFSIGNALED, WTERMSIG
#include <stdio.h>      // printf, perror
#include <stdlib.h>     // exit, abort
#include <signal.h>     // SIGABRT (for clarity)

#define NUM_CHILDREN 15

// Runs a command using execvp. If execvp fails, this function exits with 127.
static void run_exec(char *argv[]) {
    execvp(argv[0], argv);          // Replace current process image with new program
    perror("execvp failed");        // Only runs if execvp returns (meaning it failed)
    exit(127);                      // Non-zero exit code for command failure
}

int main(void) {
    pid_t childPids[NUM_CHILDREN];  // Store child PIDs in creation order
    int status;                     // Status returned by waitpid

    // Summary counts
    int exit0_count = 0;
    int exit_nonzero_count = 0;
    int signal_term_count = 0;

    // Print parent PID at start 
    printf("Parent PID: %d\n\n", (int)getpid());

    // 15 "unique" actions: 11 valid commands, 2 invalid execvp, 2 abort()
    // Each child index i corresponds to one of these actions.
    char *cmd0[]  = {"ls", "-l", NULL};
    char *cmd1[]  = {"date", NULL};
    char *cmd2[]  = {"pwd", NULL};
    char *cmd3[]  = {"whoami", NULL};
    char *cmd4[]  = {"uname", "-a", NULL};
    char *cmd5[]  = {"id", NULL};
    char *cmd6[]  = {"echo", "Hello Diego Trevino", NULL}; // required echo
    char *cmd7[]  = {"uptime", NULL};
    char *cmd8[]  = {"ps", "aux", NULL};
    char *cmd9[]  = {"true", NULL};   // exits normally with 0
    char *cmd10[] = {"false", NULL};  // exits normally with non-zero

    // Two intentionally invalid commands (execvp will fail -> exit 127)
    char *cmd11[] = {"not_a_real_cmd_470", NULL};
    char *cmd12[] = {"definitely_fake_cmd_470", NULL};

    // Create 15 children using fork inside a loop
    for (int i = 0; i < NUM_CHILDREN; i++) {
        pid_t pid = fork();

        // fork error handling
        if (pid < 0) {
            perror("fork failed");
            exit(1);
        }

        if (pid == 0) {
            // CHILD PROCESS
            pid_t myPid = getpid();

            // Print child index, PID, and the command it will execute.
            printf("Child %d | PID=%d | ", i, (int)myPid);

            // Two children terminate by signal using abort() 
            if (i == 13 || i == 14) {
                printf("Command=abort() (intentional SIGABRT)\n");
                fflush(stdout);
                abort(); // terminates by signal-based termination
            }

            // All other children execvp a command
            if (i == 0)  { printf("Command=ls -l\n"); fflush(stdout); run_exec(cmd0); }
            if (i == 1)  { printf("Command=date\n"); fflush(stdout); run_exec(cmd1); }
            if (i == 2)  { printf("Command=pwd\n"); fflush(stdout); run_exec(cmd2); }
            if (i == 3)  { printf("Command=whoami\n"); fflush(stdout); run_exec(cmd3); }
            if (i == 4)  { printf("Command=uname -a\n"); fflush(stdout); run_exec(cmd4); }
            if (i == 5)  { printf("Command=id\n"); fflush(stdout); run_exec(cmd5); }
            if (i == 6)  { printf("Command=echo \"Hello Diego Trevino\"\n"); fflush(stdout); run_exec(cmd6); }
            if (i == 7)  { printf("Command=uptime\n"); fflush(stdout); run_exec(cmd7); }
            if (i == 8)  { printf("Command=ps aux\n"); fflush(stdout); run_exec(cmd8); }
            if (i == 9)  { printf("Command=true\n"); fflush(stdout); run_exec(cmd9); }
            if (i == 10) { printf("Command=false\n"); fflush(stdout); run_exec(cmd10); }

            // Two invalid execvp calls 
            if (i == 11) { printf("Command=not_a_real_cmd_470 (intentional fail)\n"); fflush(stdout); run_exec(cmd11); }
            if (i == 12) { printf("Command=definitely_fake_cmd_470 (intentional fail)\n"); fflush(stdout); run_exec(cmd12); }

            // Should never reach here
            exit(0);
        } else {
            // PARENT PROCESS stores PID in array in creation order.
            childPids[i] = pid;
        }
    }

    printf("\n--- Parent waiting in CREATION order (waitpid on stored PIDs) ---\n");

    // Parent waits for children in the order created 
    for (int i = 0; i < NUM_CHILDREN; i++) {
        pid_t w = waitpid(childPids[i], &status, 0);

        if (w < 0) {
            // waitpid error handling
            perror("waitpid failed");
            exit(1);
        }

        // Report how each child terminated (required)
        if (WIFEXITED(status)) {
            int code = WEXITSTATUS(status);
            printf("Child %d (PID=%d) EXITED normally | code=%d\n",
                   i, (int)childPids[i], code);

            if (code == 0) exit0_count++;
            else exit_nonzero_count++;
        } else if (WIFSIGNALED(status)) {
            int sig = WTERMSIG(status);
            printf("Child %d (PID=%d) TERMINATED by signal | signal=%d\n",
                   i, (int)childPids[i], sig);
            signal_term_count++;
        } 
    }

    // Print summary counts
    printf("\n--- Summary ---\n");
    printf("Exit normally with code 0: %d\n", exit0_count);
    printf("Exit normally with non-zero code: %d\n", exit_nonzero_count);
    printf("Terminated by signal: %d\n", signal_term_count);

    // Small note about order difference 
    printf("\nNote: Children are created in a fixed order, but they may finish in a different order.\n");

    return 0;
}