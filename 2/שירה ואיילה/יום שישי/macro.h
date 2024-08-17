#ifndef MACRO_H
#define MACRO_H



#define MAX_LINE_LENGTH 81
#define MAX_MACRO_NAME_LENGTH 31
#define RESERVED_WORD_NUM 31
#define MAX_LABEL_LENGTH 31
#define FALSE 0  
#define TRUE 1

typedef struct
{
    char symbol[10];
    int value;
} Symbol;

// הצהרת המערך שמכיל את המילים השמורות בשפה
extern char strings[RESERVED_WORD_NUM][10];

typedef struct {
    char* name;
    char* content;
} Macro;

#define MAX_LABEL_LENGTH 31
// הצהרה על פונקציות עיקריות
Macro* readMacrosFromFile(const char* filename, int* macroCount, int* error);
void expandMacrosInFile(const char* inputFilename,char* outputFilename, Macro* macros, int macroCount, int* error);

// הצהרה על פונקציות עזר  
int reserved_words(char* str);
int is_register(char* str);
int is_comment(char* line);
int is_empty_line(char* line);
int is_whitespace_line(const char* line);
int is_empty_macro_line(const char* line);
char* readFile(const char* filename);//לא צריך אותה
void freeMacros(Macro* macros, int macroCount);
void removeExtension(const char* inputFilename, char* outputFilename);
void trimLeadingWhitespace(char *str);
char *trimLeadingWhitespaceFromFile(const char *fileContent);
bool has_leading_whitespace(const char *line);
void addExtension(char* filename, const char* extension, char* result);

//פונקציות לבדיקות תקינות
bool isInSet(const char* word, const char* set[], int setSize);
bool isValidLabel(const char* word, Macro* macros, int macroCount);
void processFile(char* filename, Macro* macros, int macroCount);
void processLine(char* line, Macro* macros, int macroCount);


//פונקציות של נעה
//קובץ new
char* trim(char* str);

void print_memory();

char* int_to_binary(int num, int length);
int get_register_number(const char* reg);
void add_label(const char* name, int address);
int get_label_address2(const char* name);
void assemble_instruction(const char *label, const char *op, const char *operand1, const char *operand2);
void process_line(char* line);
void init_assembler();
void assemble_file(const char* filename);

//קובץ data
char *int_to_binary2(int num, int length);
void add_label2(const char* name, int address);
void handle_data_directive(const char *label, const char *params);
void handle_string_directive(const char *label, const char *params);
void handle_entry_directive2(const char *label);
void handle_extern_directive(const char *label);
void parse_line(char *line);
void print_memory2();
void print_entry_table();
void print_extern_table();
void resolve_entries2();

/*קובץ lable
void label_add(int count, char *label_name, int address, char *attribute, size_t *array_size, TypeLabel **symbols_table);
void handle_entry_directive(const char *label);
void handle_extern(const char *label, TypeLabel *symbols_table);
void resolve_entries(TypeLabel *symbols_table);
int get_label_address(const char *name, TypeLabel *symbols_table);
int is_defined_ext(char *token, TypeLabel *symbols_table);
int is_defined_ent(char *token, TypeLabel *symbols_table);
int is_label_exists(char *label_name, TypeLabel *symbols_table);
int get_label_values(char *token, int *label_base_val, int *label_offset_val, int line_number, TypeLabel *symbols_table);
int base_address(int address);
void update_data_labels_address(int last_address, TypeLabel *symbols_table);
void first_pass(char *filename);
void print_label_table(TypeLabel *symbols_table);

char *strtok_r(char *str, const char *delim, char **saveptr);*/

//פונקציות של המעבר השני
void createEntryFile(const char* filename, Symbol* entryTable, Symbol* symbolTable, int entryTableSize, int symbolTableSize, bool errorOccurred);
void createExternFile(const char* filename, Symbol* externTable, int externTableSize);
int findLabelAddress(Symbol* table, int tableSize, const char* label);
void decimalToBinary12(int decimal, char* binary);
int binaryToDecimal(const char* binary);
void decimalToOctal(int decimal, char* octal);
void createExtension(const char* filename, char* outputFilename, const char* extension);
bool checkForDuplicateLabels(Symbol* entryTable, int entryTableSize, Symbol* externTable, int externTableSize);
bool isLabelInTable(Symbol* table, int tableSize, const char* label);
#endif