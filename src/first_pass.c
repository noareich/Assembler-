#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "first_pass.h"


//קורא את הקובץ המעובד ומחזיר קובץ חדש 
//1
Line* createLine(const char* content) {
    Line* newLine = (Line*)malloc(sizeof(Line));
    if (newLine) {
        newLine->content = strdup(content);
        newLine->next = NULL;
    }
    return newLine;
}

void freeLines(Line* head) {
    Line* current = head;
    while (current) {
        Line* next = current->next;
        free(current->content);
        free(current);
        current = next;
    }
}

Line* read_preprocessed_file(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Cannot open file");
        return NULL;
    }

    Line* head = NULL;
    Line* tail = NULL;
    char lineBuffer[256];

    while (fgets(lineBuffer, sizeof(lineBuffer), file)) {
        size_t len = strlen(lineBuffer);
        if (len > 0 && (lineBuffer[len - 1] == '\n' || lineBuffer[len - 1] == '\r')) {
            lineBuffer[len - 1] = '\0';
        }

        Line* newLine = createLine(lineBuffer);
        if (!newLine) {
            perror("Memory allocation failed");
            freeLines(head);
            fclose(file);
            return NULL;
        }

        if (!head) {
            head = newLine;
        } else {
            tail->next = newLine;
        }
        tail = newLine;
    }

    fclose(file);
    return head;
}

int write_to_new_file(const char* filename, Line* head) {
    FILE* file = fopen(filename, "w");
    if (!file) {
        perror("Cannot open file for writing");
        return 0;
    }
    
    Line* current = head;
    while (current) {
        fprintf(file, "%s\n", current->content);
        current = current->next;
    }
    
    fclose(file);
    return 1;

    //2


}