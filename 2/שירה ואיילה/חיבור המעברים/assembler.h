#ifndef ASSEMBLER_H
#define ASSEMBLER_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <errno.h>
#include <stddef.h>

/* הגדרת קבועים */
#define MAX_LINE_LENGTH 100
#define MAX_LABEL_LENGTH 31
#define INITIAL_MEMORY_SIZE 1000
#define INITIAL_TABLE_SIZE 10
#define MAX_MACRO_NAME_LENGTH 31
#define RESERVED_WORD_NUM 31
#define FALSE 0  
#define TRUE 1

// הצהרת המערך שמכיל את המילים השמורות בשפה
/*זה נקרא משתנים גלובליים - לבדוק אם זה בסדר*/
extern char strings[RESERVED_WORD_NUM][10];
extern const char* instruction_set[];
extern const char* registers[];
extern const char* Guidelines_set[];
/* הגדרת מבני נתונים */
typedef struct {
    int address;
    char binary[16];
    char label[MAX_LABEL_LENGTH + 1];
} Instruction;

typedef struct {
    char name[MAX_LABEL_LENGTH + 1];
    int address;
    int is_entry;
    int is_extern;
} Label;

typedef struct {
    char name[MAX_LABEL_LENGTH + 1];
    int address;
} EntryLabel;

typedef struct {
    char name[MAX_LABEL_LENGTH + 1];
    int address;
} ExternLabel;

typedef struct {
    char* name;
    char* content;
} Macro;

typedef struct {
    Instruction* memory;
    int memory_size;
    int memory_capacity;
    Label* label_table;
    int label_count;
    int label_capacity;
    EntryLabel* entry_table;
    int entry_count;
    int entry_capacity;
    ExternLabel* extern_table;
    int extern_count;
    int extern_capacity;
    int IC;
    int DC;
} AssemblerState;

/*הצהרות על פונקציות במעבר הראשון*/
AssemblerState* init_assembler_state();
void free_assembler_state(AssemblerState* state);
void add_label(AssemblerState* state, const char* name, int address);
int get_label_address(AssemblerState* state, const char* name);
void handle_data_directive(AssemblerState* state, const char* label, const char* params);
void handle_string_directive(AssemblerState* state, const char* label, const char* params);
void handle_entry_directive(AssemblerState* state, const char* label);
void handle_extern_directive(AssemblerState* state, const char* label);
void assemble_instruction(AssemblerState* state, const char* label, const char* op, const char* operand1, const char* operand2);
void process_line(AssemblerState* state, char* line);
void first_pass(AssemblerState* state, const char* filename);
void update_entry_addresses(AssemblerState* state);
void print_memory(AssemblerState* state);
void print_label_table(AssemblerState* state);
void print_entry_table(AssemblerState* state);
void print_extern_table(AssemblerState* state);
char *strtok_r(char *str, const char *delim, char **saveptr);

/*הצהרות על הפונקציות במעבר השני*/
void createEntryFile(AssemblerState* state, const char* filename);
void createExternFile(AssemblerState* state, const char* filename);


/* הצהרות על פונקציות עזר */
char* trim(char* str);
int get_opcode(const char* operation);
int get_addressing_mode(const char* operand);
int get_register_number(const char* operand);
char* int_to_binary(int value, int bits);
int is_single_operand_instruction(const char* op);
char* encode_immediate_operand(const char* operand);
int second_pass(AssemblerState* state, const char* input_filename, const char* output_filename);
void decimalToBinary12(int n, char* binaryStr);
int binaryToDecimal(const char* binaryStr);
void decimalToOctal(int decimal, char* octalStr);
void createExtension(const char* filename, char* outputFilename, const char* extension);

/*הצהרה על פונקציות בד+קדם אסמבלר*/
Macro* readMacrosFromFile(const char* filename, int* macroCount, int* error);
void expandMacrosInFile(const char* inputFilename, const char* outputFilename, Macro* macros, int macroCount, int* error);





/*פונקציות עזר בקדם אסמבלר*/
int reserved_words(char* str);
int is_register(char* str);
int is_comment(const char* line);
int is_empty_line(const char* line);
int is_whitespace_line(const char* line);
int is_empty_macro_line(const char* line);
void freeMacros(Macro* macros, int macroCount);
void removeExtension(const char* inputFilename, char* outputFilename);
void trimLeadingWhitespace(char *str);
char *trimLeadingWhitespaceFromFile(const char *fileContent);
bool has_leading_whitespace(const char *line);
void addExtension(char* filename, const char* extension, char* result);

/*הצהרות על בדיקות תקינות למעבר הראשון*/
int has_invalid_characters(const char *str);
int data_intergity_check(char *operands, int* error);
int string_intergity_check(char *operands, int *error);
int entry_intergity_check(char *line, int *error);
int extern_intergity_check(char *line, int *error);
int isInSet(const char* word, const char* set[], int setSize);
int isValidLabel(const char *word, Macro *macros, int* macroCount);
#endif // ASSEMBLER_H