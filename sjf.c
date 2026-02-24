/*
 * sjf.c - Preemptive SJF CPU Scheduling Simulator (SRTF)
 * Author: Diego Trevino
 *
 * SJF in preemptive form is called SRTF (Shortest Remaining Time First):
 * at each time unit, the CPU runs the arrived process with the smallest
 * remaining burst time. If a shorter job arrives, it can preempt.
 *
 * Input:
 *   n
 *   then n lines: PID ARRIVAL BURST
 *
 * Output:
 *   Execution order (Gantt-style segments) + waiting/turnaround per process + averages
 *
 * Formulas:
 *   turnaround = completion - arrival
 *   waiting    = turnaround - burst
 */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

typedef struct {
    int pid;
    int arrival;
    int burst;
    int remaining;
    int completion;
    int waiting;
    int turnaround;
} Process;

typedef struct {
    int pid;   // -1 means IDLE
    int start; // inclusive
    int end;   // exclusive
} Segment;

/* Adds/merges a timeline segment so repeated 1-unit runs print cleanly. */
static void add_segment(Segment **segs, int *count, int *cap, int pid, int start, int end) {
    if (start == end) return;

    // merge with previous segment if same PID and contiguous
    if (*count > 0 && (*segs)[*count - 1].pid == pid && (*segs)[*count - 1].end == start) {
        (*segs)[*count - 1].end = end;
        return;
    }

    // grow dynamic array if needed
    if (*count >= *cap) {
        *cap = (*cap == 0) ? 16 : (*cap * 2);
        *segs = (Segment *)realloc(*segs, (*cap) * sizeof(Segment));
        if (!*segs) { perror("realloc"); exit(1); }
    }

    (*segs)[*count].pid = pid;
    (*segs)[*count].start = start;
    (*segs)[*count].end = end;
    (*count)++;
}

static int all_done(Process *p, int n) {
    for (int i = 0; i < n; i++) {
        if (p[i].remaining > 0) return 0;
    }
    return 1;
}

int main(void) {
    int n;

    printf("Enter number of processes: ");
    if (scanf("%d", &n) != 1 || n <= 0) {
        fprintf(stderr, "Invalid n.\n");
        return 1;
    }

    Process *p = (Process *)calloc(n, sizeof(Process));
    if (!p) { perror("calloc"); return 1; }

    printf("Enter processes as: PID ARRIVAL BURST\n");
    for (int i = 0; i < n; i++) {
        if (scanf("%d %d %d", &p[i].pid, &p[i].arrival, &p[i].burst) != 3) {
            fprintf(stderr, "Invalid input line.\n");
            free(p);
            return 1;
        }
        if (p[i].arrival < 0 || p[i].burst <= 0) {
            fprintf(stderr, "Arrival must be >= 0 and burst must be > 0.\n");
            free(p);
            return 1;
        }

        p[i].remaining = p[i].burst;
        p[i].completion = -1;
    }

    Segment *segs = NULL;
    int seg_count = 0, seg_cap = 0;

    int t = 0;
    while (!all_done(p, n)) {
        // Pick the arrived process with the smallest remaining time (SRTF)
        int pick = -1;
        int best_rem = INT_MAX;

        for (int i = 0; i < n; i++) {
            if (p[i].arrival <= t && p[i].remaining > 0) {
                if (p[i].remaining < best_rem) {
                    best_rem = p[i].remaining;
                    pick = i;
                } else if (p[i].remaining == best_rem && pick != -1) {
                    // tie-break: earlier arrival, then smaller PID
                    if (p[i].arrival < p[pick].arrival ||
                        (p[i].arrival == p[pick].arrival && p[i].pid < p[pick].pid)) {
                        pick = i;
                    }
                }
            }
        }

        if (pick == -1) {
            // No ready process -> CPU idle until the next arrival
            int next_arr = INT_MAX;
            for (int i = 0; i < n; i++) {
                if (p[i].remaining > 0 && p[i].arrival > t && p[i].arrival < next_arr) {
                    next_arr = p[i].arrival;
                }
            }
            if (next_arr == INT_MAX) break; // safety
            add_segment(&segs, &seg_count, &seg_cap, -1, t, next_arr);
            t = next_arr;
            continue;
        }

        // Run the chosen process for 1 time unit (preemption happens naturally each tick)
        add_segment(&segs, &seg_count, &seg_cap, p[pick].pid, t, t + 1);
        p[pick].remaining--;
        t++;

        if (p[pick].remaining == 0) {
            // Finished -> record completion time
            p[pick].completion = t;
        }
    }

    // Compute metrics using standard formulas
    double avg_wait = 0.0, avg_tat = 0.0;
    for (int i = 0; i < n; i++) {
        p[i].turnaround = p[i].completion - p[i].arrival;
        p[i].waiting = p[i].turnaround - p[i].burst;
        avg_wait += p[i].waiting;
        avg_tat += p[i].turnaround;
    }
    avg_wait /= n;
    avg_tat /= n;

    printf("\n=== Preemptive SJF (SRTF) Execution Order ===\n");
    for (int i = 0; i < seg_count; i++) {
        if (segs[i].pid == -1)
            printf("[%d - %d] IDLE\n", segs[i].start, segs[i].end);
        else
            printf("[%d - %d] P%d\n", segs[i].start, segs[i].end, segs[i].pid);
    }

    printf("\n=== Results ===\n");
    printf("%-6s %-8s %-6s %-8s %-11s\n", "PID", "ARRIVE", "BURST", "WAIT", "TURNAROUND");
    for (int i = 0; i < n; i++) {
        printf("%-6d %-8d %-6d %-8d %-11d\n",
               p[i].pid, p[i].arrival, p[i].burst, p[i].waiting, p[i].turnaround);
    }
    printf("\nAverage waiting time: %.2f\n", avg_wait);
    printf("Average turnaround time: %.2f\n", avg_tat);

    free(segs);
    free(p);
    return 0;
}