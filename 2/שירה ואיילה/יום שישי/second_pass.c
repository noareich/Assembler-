#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "assembler.h"


int second_pass(AssemblerState* state, const char* input_filename, const char* output_filename) {
   

    FILE* inputFile;
    FILE* outputFile;
    char line[MAX_LINE_LENGTH];
    char address[16];
    char code[32];
    char label[MAX_LABEL_LENGTH];
    char binaryCode[16];
    char addressBinary[13];
    int addressValue;
    int binaryDecimal;
    char octalStr[8];
    bool errorOccurred = false;

    inputFile = fopen(input_filename, "r");
    if (!inputFile) {
        perror("שגיאה בפתיחת קובץ הקלט");
        return 1;
    }

    outputFile = fopen(output_filename, "w");
    if (!outputFile) {
        perror("שגיאה ביצירת קובץ הפלט");
        fclose(inputFile);
        return 1;
    }

    // כתיבת ערכי IC ו-DC בתחילת קובץ הפלט
    fprintf(outputFile, "%d %d\n", state->IC - 100, state->DC);
for (int i = 100; i < state->IC + state->DC; i++) {
        if (state->memory[i].label[0] != '\0') {
            bool is_extern = false;
            for (int j = 0; j < state->extern_count; j++) {
                if (strcmp(state->extern_table[j].name, state->memory[i].label) == 0) {
                    is_extern = true;
                    break;
                }
            }

               if (is_extern) {
                strcpy(binaryCode, "000000000000");
                strcat(binaryCode, "001");
                binaryCode[15] = '\0';
                binaryDecimal = binaryToDecimal(binaryCode);
                decimalToOctal(binaryDecimal, octalStr);
                fprintf(outputFile, "%04d %s\n", i, octalStr);
            } else {
                addressValue = get_label_address(state, state->memory[i].label);
                if (addressValue != -1) {
                    decimalToBinary12(addressValue, addressBinary);
                    strcpy(binaryCode, addressBinary);
                    strcat(binaryCode, "010");
                    binaryCode[15] = '\0';
                    binaryDecimal = binaryToDecimal(binaryCode);
                    decimalToOctal(binaryDecimal, octalStr);
                    fprintf(outputFile, "%04d %s\n", i, octalStr);
                } else {
                    fprintf(stderr, "שגיאה: תווית לא מוגדרת %s\n", state->memory[i].label);
                    errorOccurred = true;
                }
            }
        } else {
            binaryDecimal = binaryToDecimal(state->memory[i].binary);
            decimalToOctal(binaryDecimal, octalStr);
            fprintf(outputFile, "%04d %s\n", i, octalStr);
        }
    }
    fclose(inputFile);
    fclose(outputFile);

    if (!errorOccurred) {
        createEntryFile(state, input_filename);
        createExternFile(state, input_filename);
    } else {
        remove(output_filename);
        return 1;
    }

    return 0;
}
   

void createEntryFile(AssemblerState* state, const char* filename) {
    char entryFilename[MAX_FILENAME];
    FILE* entFile;
    int i;

    if (state->entry_count > 0) {
        createExtension(filename, entryFilename, ".ent");
        entFile = fopen(entryFilename, "w");
        if (!entFile) {
            perror("שגיאה ביצירת קובץ entry");
            return;
        }

        for (i = 0; i < state->entry_count; i++) {
            if (get_label_address(state, state->entry_table[i].name) != -1) {
                fprintf(entFile, "%s %04d\n", state->entry_table[i].name, state->entry_table[i].address);
            } else {
                fprintf(stderr, "שגיאה: תווית entry %s לא נמצאה בטבלת הסמלים\n", state->entry_table[i].name);
            }
        }
        fclose(entFile);
    }
}

void createExternFile(AssemblerState* state, const char* filename) {
    char externFilename[MAX_FILENAME];
    FILE* extFile;
    int i;

    if (state->extern_count > 0) {
        createExtension(filename, externFilename, ".ext");
        extFile = fopen(externFilename, "w");
        if (!extFile) {
            perror("שגיאה ביצירת קובץ extern");
            return;
        }

        for (i = 0; i < state->extern_count; i++) {
            fprintf(extFile, "%s %04d\n", state->extern_table[i].name, 0);
        }
        fclose(extFile);
    }
}

// פונקציות עזר

void decimalToBinary12(int n, char* binaryStr) {
    int i;
    binaryStr[0] = '\0';
    for (i = 11; i >= 0; i--) {
        int bit = (n >> i) & 1;
        strcat(binaryStr, bit ? "1" : "0");
    }
}

int binaryToDecimal(const char* binaryStr) {
    return (int)strtol(binaryStr, NULL, 2);
}

void decimalToOctal(int decimal, char* octalStr) {
    sprintf(octalStr, "%05o", decimal);
}

void createExtension(const char* filename, char* outputFilename, const char* extension) {
    strcpy(outputFilename, filename);
    char* dot = strrchr(outputFilename, '.');
    if (dot != NULL) {
        *dot = '\0';
    }
    strcat(outputFilename, extension);
}