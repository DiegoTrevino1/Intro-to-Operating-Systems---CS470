#ifndef DIEGO_LIBFC_H
#define DIEGO_LIBFC_H

int fileCreate(const char *filename);
int fileOpen(const char *filename);
int fileWrite(int fd, const void *buffer, int size);
int fileRead(int fd, void *buffer, int size);
int fileClose(int fd);
int fileDelete(const char *filename);

#endif