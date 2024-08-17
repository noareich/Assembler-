typedef struct
{
    char symbol[10];
    int value;
} Symbol;

#define MAX_LABEL_LENGTH 31

// פונקציה שממירה מספר עשרוני לבינארי באורך 12 סיביות
void decimalToBinary12(int n, char *binaryStr)
{
    // Initialize the binary string
    binaryStr[0] = '\0';

    // Iterate over each bit (12 bits)
    for (int i = 11; i >= 0; i--)
    {
        int bit = (n >> i) & 1;
        // Append the bit to the binary string
        strcat(binaryStr, bit ? "1" : "0");
    }
}
// פונקציה לחיפוש תווית בטבלת סמלים
int findLabelAddress(Symbol *table, int tableSize, const char *label)
{
    for (int i = 0; i < tableSize; i++)
    {
        if (strcmp(table[i].symbol, label) == 0)
        {
            return table[i].value;
        }
    }
    return -1; // לא נמצא
}
/*פונקציה שיוצרת סיומת חדשה לקובץ פלט*/
void createOutputFilename(const char *filename, char *outputFilename, const char *extension) {
    strcpy(outputFilename, filename);
    strcat(outputFilename, extension);
}
פונקציה ראשית:
void processFile(const char *filename, Symbol *symbolTable, Symbol *entryTable, Symbol *externTable, int symbolTableSize, int entryTableSize, int externTableSize) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open file");
        return;
    }

    char outputFilename[256];
    createOutputFilename(filename, outputFilename, ".ob");
    FILE *outputFile = fopen(outputFilename, "w");
    if (!outputFile) {
        perror("Failed to create output file");
        fclose(file);
        return;
    }

    char entFilename[256], extFilename[256];
    createOutputFilename(filename, entFilename, ".ent");
    createOutputFilename(filename, extFilename, ".ext");
    FILE *entFile = NULL;
    FILE *extFile = NULL;

    bool errorOccurred = false;
    char line[256];
    
    while (fgets(line, sizeof(line), file)) {
        char address[16], code[32];
        if (sscanf(line, "%s %s", address, code) == 2) {
            if (code[0] == '"') {
                char label[MAX_LABEL_LENGTH];
                sscanf(code, "\"%[^\"]\"", label);

                int addressValue = findLabelAddress(entryTable, entryTableSize, label);
                char binaryCode[16]; // משתמש באורך 16 כדי להבטיח מספיק מקום
                if (addressValue != -1) {
                    // מצאנו תווית בטבלת ה-entry
                    if (findLabelAddress(symbolTable, symbolTableSize, label) == -1) {
                        // תווית לא נמצאת בטבלת הסמלים
                        errorOccurred = true;
                        fprintf(stderr, "Error: Entry label %s not found in symbol table\n", label);
                        continue;
                    }

                    // קידוד עבור טבלת ה-entry
                    char addressBinary[13];
                    decimalToBinary12(addressValue, addressBinary);
                    strcpy(binaryCode, addressBinary);
                    strcat(binaryCode, "010"); // יצירת קוד באורך 15 סיביות
                    binaryCode[15] = '\0'; // לוודא שהמחרוזת מסתיימת ב-null
                    fprintf(outputFile, "%s %s\n", address, binaryCode);
                    if (!entFile) {
                        entFile = fopen(entFilename, "w");
                        if (!entFile) {
                            perror("Failed to create entry file");
                            fclose(file);
                            fclose(outputFile);
                            return;
                        }
                    }
                    fprintf(entFile, "%s %d\n", label, addressValue); // כתיבה של התווית והכתובת בעשרוני
                } else {
addressValue = findLabelAddress(symbolTable, symbolTableSize, label);
                    if (addressValue != -1) {
                        // קידוד עבור טבלת הסמלים
                        char addressBinary[13];
                        decimalToBinary12(addressValue, addressBinary);
                        strcpy(binaryCode, addressBinary);
                        strcat(binaryCode, "100"); // יצירת קוד באורך 15 סיביות
                        binaryCode[15] = '\0'; // לוודא שהמחרוזת מסתיימת ב-null
                        fprintf(outputFile, "%s %s\n", address, binaryCode);
                    } else {
                        addressValue = findLabelAddress(externTable, externTableSize, label);
                        if (addressValue != -1) {
                            // קידוד עבור טבלת ה-extern
                            strcpy(binaryCode, "000000000000");
                            strcat(binaryCode, "001"); // קוד באורך 15 סיביות
                            fprintf(outputFile, "%s %s\n", address, binaryCode);
                            if (!extFile) {
                                extFile = fopen(extFilename, "w");
                                if (!extFile) {
                                    perror("Failed to create extern file");
                                    fclose(file);
                                    fclose(outputFile);
                                    if (entFile) fclose(entFile);
                                    return;
                                }
                            }
                            fprintf(extFile, "%s %d\n", label, addressValue); // כתיבה של התווית והכתובת בעשרוני
                        } else {
                            errorOccurred = true;
                            fprintf(stderr, "Error: Undefined label %s\n", label);
                        }
                    }
                }
            } else {
                fprintf(outputFile, "%s %s\n", address, code);
            }
        }
    }

    fclose(file);
    fclose(outputFile);
    if (entFile) fclose(entFile);
    if (extFile) fclose(extFile);

    if (errorOccurred) {
        remove(outputFilename);
        if (entFile) remove(entFilename);
        if (extFile) remove(extFilename);
    }
}
המיין:
int main()
{
    // יצירת מערך של סמלים וערכים לפי הטבלה
    Symbol symbolTable[] = {
        {"MAIN", 100},
        {"LOOP", 105},
        {"END", 131},
        {"STR", 132},
       {"LIST", 137},
        {"K", 140}
    };
    Symbol entryTable[] = {
        {"MAIN", 100},
        {"LIST", 137},

    };
    Symbol externTable[] = {
        {"fn1", 104},
        {"L3", 114},
        {"L3", 127},
        {"L3", 128},
    };

    int symbolTableSize = sizeof(symbolTable) / sizeof(symbolTable[0]);
    int entryTableSize = sizeof(entryTable) / sizeof(entryTable[0]);
    int externTableSize = sizeof(externTable) / sizeof(externTable[0]);

    // קריאה והחלפת הערכים בקובץ
    processFile("text", symbolTable, entryTable, externTable, symbolTableSize, entryTableSize, externTableSize);

    return 0;
}
