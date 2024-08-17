#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "first_pass.h" // נניח שזה הקובץ המכיל את ההגדרות מהמעבר הראשון

#define MAX_LINE_LENGTH 100
#define MAX_FILENAME 100

// פונקציות עזר
void decimalToBinary12(int n, char* binaryStr);
int binaryToDecimal(const char* binaryStr);
void decimalToOctal(int decimal, char* octalStr);
void createExtension(const char* filename, char* outputFilename, const char* extension);

// פונקציות ראשיות
int second_pass(const char* input_filename, const char* output_filename);
void createEntryFile(const char* filename, EntryLabel* entryTable, Label* symbolTable, int entryTableSize, int symbolTableSize);
void createExternFile(const char* filename, char externTable[MAX_EXTERNS][MAX_LABEL_LENGTH + 1], int externTableSize);

// פונקציה ראשית למעבר השני
int second_pass(const char* input_filename, const char* output_filename) {
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

    while (fgets(line, sizeof(line), inputFile)) {
        if (sscanf(line, "%s %s", address, code) == 2) {
            if (code[0] == '"') {
                sscanf(code, "\"%[^\"]\"", label);
                addressValue = get_label_address(label);
                if (addressValue != -1) {
                    decimalToBinary12(addressValue, addressBinary);
                    strcpy(binaryCode, addressBinary);
                    strcat(binaryCode, "010");
                    binaryCode[15] = '\0';
                    binaryDecimal = binaryToDecimal(binaryCode);
                    decimalToOctal(binaryDecimal, octalStr);
                    fprintf(outputFile, "%s %s\n", address, octalStr);
                } else {
                    fprintf(stderr, "שגיאה: תווית לא מוגדרת %s\n", label);
                    errorOccurred = true;
                }
            } else {
                binaryDecimal = binaryToDecimal(code);
                decimalToOctal(binaryDecimal, octalStr);
                fprintf(outputFile, "%s %s\n", address, octalStr);
            }
        }
    }

    fclose(inputFile);
    fclose(outputFile);

    if (!errorOccurred) {
        createEntryFile(input_filename, entry_table, label_table, entry_count, label_count);
        createExternFile(input_filename, extern_table, extern_count);
    } else {
        remove(output_filename);
        return 1;
    }

    return 0;
}

void createEntryFile(const char* filename, EntryLabel* entryTable, Label* symbolTable, int entryTableSize, int symbolTableSize) {
    char entryFilename[MAX_FILENAME];
    FILE* entFile;
    int i;

    if (entryTableSize > 0) {
        createExtension(filename, entryFilename, ".ent");
        entFile = fopen(entryFilename, "w");
        if (!entFile) {
            perror("שגיאה ביצירת קובץ entry");
            return;
        }

        for (i = 0; i < entryTableSize; i++) {
            if (get_label_address(entryTable[i].name) != -1) {
                fprintf(entFile, "%s %04d\n", entryTable[i].name, entryTable[i].address);
            } else {
                fprintf(stderr, "שגיאה: תווית entry %s לא נמצאה בטבלת הסמלים\n", entryTable[i].name);
            }
        }
        fclose(entFile);
    }
}

void createExternFile(const char* filename, char externTable[MAX_EXTERNS][MAX_LABEL_LENGTH + 1], int externTableSize) {
    char externFilename[MAX_FILENAME];
    FILE* extFile;
    int i;

    if (externTableSize > 0) {
        createExtension(filename, externFilename, ".ext");
        extFile = fopen(externFilename, "w");
        if (!extFile) {
            perror("שגיאה ביצירת קובץ extern");
            return;
        }

        for (i = 0; i < externTableSize; i++) {
            fprintf(extFile, "%s %04d\n", externTable[i], 0); // כתובת 0 עבור תוויות חיצוניות
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