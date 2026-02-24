/*
 * File: rr.c - Round Robin CPU Scheduling Simulator (preemptive via time quantum)
 * Author: Diego Trevino
 *
 * Input:
 *   n
 *   quantum
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

typedef struct {
    int pid;
    int arrival;
    int burst;
    int remaining;
    int completion;
    int waiting;
    int turnaround;
    int enqueued; // set to 1 once it has been added to the ready queue
} Process;

typedef struct {
    int pid;   // -1 means IDLE
    int start; // inclusive
    int end;   // exclusive
} Segment;

/* Adds/merges a timeline segment so the output stays readable. */
static void add_segment(Segment **segs, int *count, int *cap, int pid, int start, int end) {
    if (start == end) return;

    // merge with previous segment if it's the same PID and touches
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

/* Simple circular queue storing indices into the Process array. */
typedef struct {
    int *data;
    int cap;
    int head;
    int tail;
    int size;
} Queue;

static void q_init(Queue *q, int cap) {
    q->data = (int *)malloc(sizeof(int) * cap);
    if (!q->data) { perror("malloc"); exit(1); }
    q->cap = cap;
    q->head = q->tail = q->size = 0;
}

static void q_free(Queue *q) { free(q->data); }
static int q_empty(Queue *q) { return q->size == 0; }

static void q_push(Queue *q, int v) {
    // grow queue if full
    if (q->size == q->cap) {
        int newcap = q->cap * 2;
        int *nd = (int *)malloc(sizeof(int) * newcap);
        if (!nd) { perror("malloc"); exit(1); }
        for (int i = 0; i < q->size; i++) nd[i] = q->data[(q->head + i) % q->cap];
        free(q->data);
        q->data = nd;
        q->cap = newcap;
        q->head = 0;
        q->tail = q->size;
    }

    q->data[q->tail] = v;
    q->tail = (q->tail + 1) % q->cap;
    q->size++;
}

static int q_pop(Queue *q) {
    int v = q->data[q->head];
    q->head = (q->head + 1) % q->cap;
    q->size--;
    return v;
}

/* Add any processes that have arrived by time t into the ready queue. */
static void enqueue_arrivals(Process *p, int n, Queue *q, int t) {
    for (int i = 0; i < n; i++) {
        if (!p[i].enqueued && p[i].arrival <= t) {
            q_push(q, i);
            p[i].enqueued = 1;
        }
    }
}

int main(void) {
    int n, quantum;

    printf("Enter number of processes: ");
    if (scanf("%d", &n) != 1 || n <= 0) {
        fprintf(stderr, "Invalid n.\n");
        return 1;
    }

    printf("Enter time quantum: ");
    if (scanf("%d", &quantum) != 1 || quantum <= 0) {
        fprintf(stderr, "Invalid quantum.\n");
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
        p[i].enqueued = 0;
    }

    Segment *segs = NULL;
    int seg_count = 0, seg_cap = 0;

    Queue q;
    q_init(&q, (n > 4) ? n : 4);

    int t = 0;

    // If no one arrives at time 0, mark the CPU idle until the first arrival.
    int earliest = p[0].arrival;
    for (int i = 1; i < n; i++) {
        if (p[i].arrival < earliest) earliest = p[i].arrival;
    }
    if (earliest > 0) {
        add_segment(&segs, &seg_count, &seg_cap, -1, 0, earliest);
        t = earliest;
    }

    enqueue_arrivals(p, n, &q, t);

    while (!all_done(p, n)) {
        if (q_empty(&q)) {
            // CPU idle: jump forward to the next process arrival
            int next_arr = -1;
            for (int i = 0; i < n; i++) {
                if (p[i].remaining > 0 && !p[i].enqueued) {
                    if (next_arr == -1 || p[i].arrival < next_arr) next_arr = p[i].arrival;
                }
            }
            if (next_arr == -1) break; // should not happen, but safe
            if (next_arr > t) add_segment(&segs, &seg_count, &seg_cap, -1, t, next_arr);
            t = next_arr;
            enqueue_arrivals(p, n, &q, t);
            continue;
        }

        // Round Robin: take the next ready process
        int idx = q_pop(&q);
        if (p[idx].remaining <= 0) continue;

        int slice = (p[idx].remaining < quantum) ? p[idx].remaining : quantum;

        // Run for up to one quantum (we step 1 unit at a time to catch arrivals)
        int start = t;
        for (int k = 0; k < slice; k++) {
            t++;
            p[idx].remaining--;
            enqueue_arrivals(p, n, &q, t);

            if (p[idx].remaining == 0) break;
        }
        add_segment(&segs, &seg_count, &seg_cap, p[idx].pid, start, t);

        if (p[idx].remaining == 0) {
            // finished: record completion time
            p[idx].completion = t;
        } else {
            // not finished: go to the back of the queue
            q_push(&q, idx);
        }
    }

    // Compute waiting/turnaround (same formulas used in lecture)
    double avg_wait = 0.0, avg_tat = 0.0;
    for (int i = 0; i < n; i++) {
        p[i].turnaround = p[i].completion - p[i].arrival;
        p[i].waiting = p[i].turnaround - p[i].burst;
        avg_wait += p[i].waiting;
        avg_tat += p[i].turnaround;
    }
    avg_wait /= n;
    avg_tat /= n;

    printf("\n=== Round Robin Execution Order (q=%d) ===\n", quantum);
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

    q_free(&q);
    free(segs);
    free(p);
    return 0;
}