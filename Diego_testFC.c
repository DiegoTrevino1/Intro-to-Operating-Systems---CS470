#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Diego_libFC.h"

#define FILENAME "DiegoG_Trevino_Introduction.txt"
#define READ_CHUNK 256

// clears leftover input from stdin
static void flush_stdin(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}
}

// reads menu choice safely
static int read_menu_choice(void) {
    int choice;
    printf("\nEnter choice: ");
    if (scanf("%d", &choice) != 1) {
        flush_stdin();
        return -1;
    }
    flush_stdin();
    return choice;
}

// pause between operations
static void press_enter_to_continue(void) {
    printf("\nPress ENTER to continue...");
    fflush(stdout);
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}
}

// text that will be written to the file
static const char* intro_text(void) {
    return
        "Hi, my name is Diego G. Trevino and I am a Computer Science student at Central Washington University. "
        "In this lab, I built a user-level file system library test application that demonstrates basic file operations. "
        "This program uses menu-driven prompts to guide the user through creating, opening, writing, reading, and closing a file.\n\n"
        "This project helped me understand how file system calls can be emulated at the user level while still handling errors safely. "
        "It also reinforced the importance of validating each step (like checking open handles) before proceeding to the next operation. "
        "Overall, it gave me a clearer picture of how real file systems manage data and enforce correct usage patterns.\n";
}

// prints the menu
static void print_menu(int fd_state) {
    printf("\n============================\n");
    printf(" libFC Test App (Diego)\n");
    printf("============================\n");
    printf("File: %s\n", FILENAME);
    printf("Open handle state: %s\n", (fd_state >= 0) ? "OPEN" : "CLOSED");
    printf("\n1) fileCreate  (create new file)\n");
    printf("2) fileOpen    (open existing file)\n");
    printf("3) fileWrite   (write introduction)\n");
    printf("4) fileRead    (read + print file)\n");
    printf("5) fileClose   (close file)\n");
    printf("6) fileDelete  (delete file)\n");
    printf("0) Exit\n");
}

int main(void) {
    int fd = -1;

    printf("Welcome. This program tests the file control functions.\n");

    while (1) {
        print_menu(fd);
        int choice = read_menu_choice();
        if (choice == -1) {
            printf("Invalid input. Please enter a number.\n");
            continue;
        }

        if (choice == 0) {
            if (fd >= 0) {
                fileClose(fd);
                fd = -1;
            }
            printf("Exiting program.\n");
            break;
        }

        switch (choice) {

            case 1: {
                printf("\nCreating file...\n");
                int rc = fileCreate(FILENAME);
                if (rc < 0)
                    printf("fileCreate failed (rc=%d).\n", rc);
                else
                    printf("File created successfully.\n");
                press_enter_to_continue();
                break;
            }

            case 2: {
                if (fd >= 0) {
                    printf("File already open.\n");
                    press_enter_to_continue();
                    break;
                }

                printf("\nOpening file...\n");
                fd = fileOpen(FILENAME);

                if (fd < 0) {
                    printf("fileOpen failed (fd=%d).\n", fd);
                    fd = -1;
                } else {
                    printf("File opened.\n");
                }

                press_enter_to_continue();
                break;
            }

            case 3: {
                if (fd < 0) {
                    printf("Open the file first.\n");
                    press_enter_to_continue();
                    break;
                }

                printf("\nWriting to file...\n");
                const char* text = intro_text();
                int nbytes = (int)strlen(text);

                int written = fileWrite(fd, text, nbytes);
                if (written < 0)
                    printf("fileWrite failed (rc=%d).\n", written);
                else
                    printf("Wrote %d bytes.\n", written);

                press_enter_to_continue();
                break;
            }

            case 4: {
                if (fd < 0) {
                    printf("Open the file first.\n");
                    press_enter_to_continue();
                    break;
                }

                // reset by closing and reopening
                fileClose(fd);
                fd = fileOpen(FILENAME);

                if (fd < 0) {
                    printf("Reopen failed.\n");
                    press_enter_to_continue();
                    break;
                }

                printf("\nReading file...\n");
                printf("---- START ----\n");

                char buf[READ_CHUNK + 1];

                while (1) {
                    int r = fileRead(fd, buf, READ_CHUNK);
                    if (r < 0) {
                        printf("Read error.\n");
                        break;
                    }
                    if (r == 0)
                        break;

                    buf[r] = '\0';
                    printf("%s", buf);
                }

                printf("\n---- END ----\n");
                press_enter_to_continue();
                break;
            }

            case 5: {
                if (fd < 0) {
                    printf("No file open.\n");
                    press_enter_to_continue();
                    break;
                }

                int rc = fileClose(fd);
                if (rc < 0)
                    printf("fileClose failed.\n");
                else {
                    printf("File closed.\n");
                    fd = -1;
                }

                press_enter_to_continue();
                break;
            }

            case 6: {
                if (fd >= 0) {
                    printf("Close file before deleting.\n");
                    press_enter_to_continue();
                    break;
                }

                int rc = fileDelete(FILENAME);
                if (rc < 0)
                    printf("fileDelete failed.\n");
                else
                    printf("File deleted.\n");

                press_enter_to_continue();
                break;
            }

            default:
                printf("Invalid option.\n");
                break;
        }
    }

    return 0;
}