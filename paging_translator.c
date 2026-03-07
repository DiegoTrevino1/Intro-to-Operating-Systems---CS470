/*
    paging_translator.c
    CS470 Quiz

    Simple program that translates logical addresses into physical addresses
    using paging. Page size is 1024 bytes and the page table is predefined.
*/

#include <stdio.h>

#define PAGE_SIZE 1024
#define NUM_PAGES 4

int main(void) {

    // predefined page table for this process
    int pageTable[NUM_PAGES] = {5, 2, 9, 1};
    int n;
    int logicalAddress;

    printf("Enter number of logical addresses (N):\n");
    scanf("%d", &n);

    printf("Enter logical address(es), one per line:\n");

    for (int i = 0; i < n; i++) {

        scanf("%d", &logicalAddress);

        // calculate page number and offset
        int page = logicalAddress / PAGE_SIZE;
        int offset = logicalAddress % PAGE_SIZE;

        // chheck if page number is valid
        if (page < 0 || page >= NUM_PAGES) {
            printf("Logical: %d | INVALID (page out of range)\n", logicalAddress);
        }
        else {

            // get frame number from page table
            int frame = pageTable[page];
            // calculate physical address
            int physical = frame * PAGE_SIZE + offset;

            printf("Logical: %d | Page: %d | Offset: %d | Frame: %d | Physical: %d\n",
                   logicalAddress, page, offset, frame, physical);
        }
    }

    return 0;
}