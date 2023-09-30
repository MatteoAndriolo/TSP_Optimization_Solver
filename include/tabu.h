#ifndef TABU_SEARCH_SINGLE_H
#define TABU_SEARCH_SINGLE_H
#include <stdbool.h>
#include "logger.h"

// Structure for Tabu list entry
typedef struct {
    int value;
    int age;
} TabuEntry;

// Structure for Tabu list
typedef struct {
    TabuEntry *entries;
    int size;
    int tenure;
    int head;
    int tail;
} TabuList;

// Function declarations
TabuList* initializeTabuList(int size, int tenure);
bool isTabu(TabuList *list, int value);
void addTabu(TabuList *list, int value);
void incAgeTabuList(TabuList *list);
void freeTabuList(TabuList *list);
bool areAnyValuesTabu(TabuList *list, int v1, int v2, int v3, int v4);

void testTabuList();

#endif // TABU_SEARCH_SINGLE_H
