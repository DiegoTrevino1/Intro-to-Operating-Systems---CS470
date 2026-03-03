#include "Diego_libFC.h"
#include <stdio.h>

static FILE *g_fp = NULL;   // only one open file at a time

// Create a file
int fileCreate(const char *filename) {
    if (!filename || filename[0] == '\0') return -1;

    FILE *fp = fopen(filename, "w");  // create/truncate
    if (!fp) return -2;

    fclose(fp);
    return 0;
}

// Open a file
int fileOpen(const char *filename) {
    if (!filename || filename[0] == '\0') return -1;
    if (g_fp) return -3; // already open

    g_fp = fopen(filename, "r+");     // open existing file for read/write
    if (!g_fp) return -2;

    return 1; // our “fd” handle (simple single-file design)
}

// Write a file
int fileWrite(int fd, const void *buffer, int size) {
    if (fd != 1) return -1;
    if (!g_fp) return -2;
    if (!buffer || size <= 0) return -3;

    // write from the beginning so the file contains only the intro text
    if (fseek(g_fp, 0, SEEK_SET) != 0) return -4;

    size_t written = fwrite(buffer, 1, (size_t)size, g_fp);
    fflush(g_fp);

    if ((int)written != size) return -5;
    return (int)written;
}

// Read a file
int fileRead(int fd, void *buffer, int size) {
    if (fd != 1) return -1;
    if (!g_fp) return -2;
    if (!buffer || size <= 0) return -3;

    size_t r = fread(buffer, 1, (size_t)size, g_fp);
    if (ferror(g_fp)) return -4;

    return (int)r; // 0 == EOF
}

// Close a file
int fileClose(int fd) {
    if (fd != 1) return -1;
    if (!g_fp) return -2;

    int rc = fclose(g_fp);
    g_fp = NULL;

    return (rc == 0) ? 0 : -3;
}

// Delete a file
int fileDelete(const char *filename) {
    if (!filename || filename[0] == '\0') return -1;
    if (g_fp) return -2; // must close before delete

    return (remove(filename) == 0) ? 0 : -3;
}