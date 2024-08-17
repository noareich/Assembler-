#include "assembler.h"

#define MAX_LINE_LENGTH 100
#define MAX_FILENAME 100

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
        perror("Error opening input file");
        return 1;
    }

    outputFile = fopen(output_filename, "w");
    if (!outputFile) {
        perror("Error creating output file");
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
                    fprintf(stderr, "Error: undefined label %s\n", state->memory[i].label);
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
            perror("Error creating entry file");
            return;
        }

        for (i = 0; i < state->entry_count; i++) {
            if (get_label_address(state, state->entry_table[i].name) != -1) {
                fprintf(entFile, "%s %04d\n", state->entry_table[i].name, state->entry_table[i].address);
            } else {
                fprintf(stderr, "Error: label entry %s not found in symbol table\n", state->entry_table[i].name);
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
            perror("Error: label extern %s found in symbol table\n");
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
/* פונקציה לבדוק אם תווית קיימת בטבלה 
int isLabelInTable(Label* label_table, const char* label,int* tableSize) {

    for (int i = 0; i < tableSize; i++) {
        if (strcmp(label_table[i].name, label) == 0) {
            return 1; // התווית נמצאת בטבלה
        }
    }
    return 0; // התווית לא נמצאת בטבלה
}
/* פונקציה לבדוק אם התווית אנטרי נמצאת בטבלת הסמלים 
int is_entry_label_is_defined(Label* label_table,EntryLabel* entry_table) {
    int tableSize = sizeof(label_table) / sizeof(label_table[0]);
    int EntrytableSize = sizeof(label_table) / sizeof(label_table[0]);
    for (int i = 0; i < EntrytableSize; i++) {
        if (isLabelInTable(label_table,entry_table[i].name,tableSize)) {
            return 1; // נמצאה תווית כפולה
        }
    }
    return 0; // לא נמצאה תווית כפולה
}
/* פונקציה לבדוק אם התווית אקסטרן נמצאת בטבלת הסמלים 
int is_extern_label_is_not_defined(Label* label_table,ExternLabel* extern_table) {
    int tableSize = sizeof(label_table) / sizeof(label_table[0]);
    int ExterntableSize = sizeof(label_table) / sizeof(label_table[0]);
    for (int i = 0; i < ExterntableSize; i++) {
        if (isLabelInTable(label_table,extern_table[i].name,tableSize)) {
            return 1; // נמצאה תווית כפולה
        }
    }
    return 0; // לא נמצאה תווית כפולה
}*/