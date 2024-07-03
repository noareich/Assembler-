#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "first_pass.c"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <input_file>\n", argv[0]);
        return 1;
    }

    const char* input_file = argv[1];
    char output_file[256];
    
    // יצירת שם קובץ הפלט על ידי הוספת סיומת ".out" לשם קובץ הקלט
    snprintf(output_file, sizeof(output_file), "%s.out", input_file);
    
    Line* content = read_preprocessed_file(input_file);
    if (content) {
        if (write_to_new_file(output_file, content)) {
            printf("File successfully copied to %s\n", output_file);
        } else {
            printf("Failed to write to new file\n");
        }
        freeLines(content);
    } else {
        printf("Failed to read input file\n");
    }
    
    return 0;
}