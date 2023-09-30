#include "../include/tabu.h"
#include <stdlib.h>
#include <stdio.h>


TabuList* initializeTabuList(int size, int tenure) {
    TabuList *list = (TabuList *) malloc(sizeof(TabuList));
    list->entries = (TabuEntry *) malloc(sizeof(TabuEntry) * size);
    list->size = size;
    list->tenure = tenure;
    list->head = 0;
    list->tail = 0;

    for (int i = 0; i < size; i++) {
        list->entries[i].value = -1;
        list->entries[i].age = 0;
    }

    return list;
}

bool isTabu(TabuList *list, int value) {
    for (int i = 0; i < list->size; i++) {
        if (list->entries[i].value == value) {
            return true;
        }
    }
    return false;
}

void addTabu(TabuList *list, int value) {
    list->entries[list->tail].value = value;
    list->entries[list->tail].age = 1;

    list->tail = (list->tail + 1) % list->size;
    if (list->tail == list->head) {
        list->head = (list->head + 1) % list->size;
    }
}

void updateTabuList(TabuList *list) {
    int i = list->head;
    while (true) {
        if (list->entries[i].value != -1) {
            list->entries[i].age++;
            if (list->entries[i].age > list->tenure) {
                list->entries[i].value = -1;
                list->entries[i].age = 0;
            }
        }

        if (i == list->tail) {
            break;
        }
        i = (i + 1) % list->size;
    }
}

void freeTabuList(TabuList *list) {
    free(list->entries);
    free(list);
}

bool areAnyValuesTabu(TabuList *list, int v1, int v2, int v3, int v4) {
    return isTabu(list, v1) || isTabu(list, v2) || isTabu(list, v3) || isTabu(list, v4);
}


void testTabuList() {
    printf("Testing Single-Integer Tabu List...\n");

    // Initialize Tabu List
    TabuList *list = initializeTabuList(5, 3); // Size: 5, Tenure: 3

    // Test 1: Adding and verifying presence
    addTabu(list, 1);
    if (isTabu(list, 1)) {
        printf("Test 1 Passed!\n");
    } else {
        printf("Test 1 Failed!\n");
    }

    // Test 2: Testing age and tenure
    addTabu(list, 2);
    addTabu(list, 3);
    updateTabuList(list); // Iteration 1
    updateTabuList(list); // Iteration 2
    updateTabuList(list); // Iteration 3
    if (!isTabu(list, 1)) { // 1 should be removed due to tenure
        printf("Test 2 Passed!\n");
    } else {
        printf("Test 2 Failed!\n");
    }

    // Test 3: Overwriting oldest value
    addTabu(list, 4);
    addTabu(list, 5);
    if (isTabu(list, 4) && !isTabu(list, 1)) { // 1 should be overwritten
        printf("Test 3 Passed!\n");
    } else {
        printf("Test 3 Failed!\n");
    }

    // Test 4: Checking multiple values for Tabu presence
    if (areAnyValuesTabu(list, 2, 10, 11, 12) && !areAnyValuesTabu(list, 10, 11, 12, 13)) {
        printf("Test 4 Passed!\n");
    } else {
        printf("Test 4 Failed!\n");
    }

    // Clean up
    freeTabuList(list);

    printf("Testing Completed!\n");

}
