#ifndef ASSEMBLER_H
#define ASSEMBLER_H

/*
 * This header file contains declarations for an assembler implementation.
 * It includes various data structures and function prototypes for handling
 * assembly code, macros, labels, and memory management.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <stddef.h>

/* Constants */
#define MAX_LINE_LENGTH 100
#define MAX_LABEL_LENGTH 31
#define INITIAL_MEMORY_SIZE 1000
#define MAX_LABELS 1000
#define INITIAL_TABLE_SIZE 10
#define MAX_MACRO_NAME_LENGTH 31
#define RESERVED_WORD_NUM 31
#define MAX_FILENAME 100
#define FALSE 0  
#define TRUE 1

/* Data Structures */

/**
 * Represents an assembly instruction.
 */
typedef struct {
    int address;
    char binary[16];
    char label[MAX_LABEL_LENGTH + 1];
} Instruction;

/**
 * Represents a label in the assembly code.
 */
typedef struct {
    char name[MAX_LABEL_LENGTH + 1];
    int address;
    int is_entry;
    int is_extern;
} Label;

/**
 * Represents an entry label.
 */
typedef struct {
    char name[MAX_LABEL_LENGTH + 1];
    int address;
} EntryLabel;

/**
 * Represents an external label.
 */
typedef struct {
    char name[MAX_LABEL_LENGTH + 1];
    int address;
} ExternLabel;

/**
 * Represents a macro definition.
 */
typedef struct {
    char* name;
    char* content;
} Macro;

/**
 * Represents the overall state of the assembler.
 */
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
    int current_line;

    int IC;
    int DC;
} AssemblerState;

/* General Functions */

/**
 * @brief A reentrant version of strtok.
 * 
 * @param str The string to be parsed.
 * @param delim The delimiter string.
 * @param saveptr A pointer to a char * variable that is used internally to maintain context between successive calls.
 * @return A pointer to the next token in the string.
 */
char *strtok_r(char *str, const char *delim, char **saveptr);

/**
 * @brief Trims leading and trailing whitespace from a string.
 * 
 * @param str The string to be trimmed.
 * @return A pointer to the trimmed string.
 */
char *trim(char *str);

/**
 * @brief Converts an integer to its binary representation.
 * 
 * @param value The integer to convert.
 * @param bits The number of bits in the binary representation.
 * @return A string containing the binary representation.
 */
char *int_to_binary(int value, int bits);

/**
 * @brief Adds a file extension to a filename.
 * 
 * @param filename The original filename.
 * @param extension The extension to be added.
 * @param result The resulting filename with the added extension.
 */
void addExtension(char *filename, const char *extension, char *result);

/**
 * @brief Removes leading whitespace from a string.
 * 
 * @param str The string to be trimmed.
 */
void trimLeadingWhitespace(char *str);

/**
 * @brief Checks if a string has leading whitespace.
 * 
 * @param line The string to check.
 * @return 1 if the string has leading whitespace, 0 otherwise.
 */
int has_leading_whitespace(const char *line);

/* Macro Table Functions */

/**
 * @brief Reads macros from a file.
 * 
 * @param filename The name of the file to read from.
 * @param macroCount A pointer to store the number of macros read.
 * @param error A pointer to store any error code.
 * @return An array of Macro structures.
 */
Macro* readMacrosFromFile(const char* filename, int* macroCount, int* error);

/**
 * @brief Frees the memory allocated for macros.
 * 
 * @param macros The array of Macro structures to free.
 * @param macroCount The number of macros in the array.
 */
void freeMacros(Macro *macros, int macroCount);

/**
 * @brief Checks if a line is a comment.
 * 
 * @param line The line to check.
 * @return 1 if the line is a comment, 0 otherwise.
 */
int is_comment(const char *line);

/**
 * @brief Checks if a line is empty.
 * 
 * @param line The line to check.
 * @return 1 if the line is empty, 0 otherwise.
 */
int is_empty_line(const char *line);

/**
 * @brief Checks if a line contains only whitespace.
 * 
 * @param line The line to check.
 * @return 1 if the line contains only whitespace, 0 otherwise.
 */
int is_whitespace_line(const char *line);

/**
 * @brief Checks if a line is an empty macro line.
 * 
 * @param line The line to check.
 * @return 1 if the line is an empty macro line, 0 otherwise.
 */
int is_empty_macro_line(const char *line);

/**
 * @brief Expands macros in a file and writes the result to a new file.
 * 
 * @param inputFilename The name of the input file.
 * @param outputFilename The name of the output file.
 * @param macros An array of Macro structures.
 * @param macroCount The number of macros in the array.
 * @param error A pointer to store any error code.
 */
void expandMacrosInFile(const char* inputFilename, char* outputFilename, Macro* macros, int macroCount, int* error);

/* First Pass Functions */

/**
 * @brief Initializes the assembler state.
 * 
 * @return A pointer to the initialized AssemblerState.
 */
AssemblerState *init_assembler_state();

/**
 * @brief Processes a single line of assembly code.
 * 
 * @param state The current assembler state.
 * @param line The line to process.
 * @param macros An array of Macro structures.
 * @param macroCount A pointer to the number of macros.
 * @return An integer indicating success or failure.
 */
int process_line(AssemblerState* state, char* line, Macro *macros, int *macroCount);

/**
 * @brief Performs the first pass of the assembly process.
 * 
 * @param state The current assembler state.
 * @param filename The name of the input file.
 * @param macros An array of Macro structures.
 * @param macroCount A pointer to the number of macros.
 * @param error A pointer to store any error code.
 */
void first_pass(AssemblerState *state, const char *filename, Macro *macros, int *macroCount, int *error);

/* Directive Handling Functions */

/**
 * @brief Handles the .data directive.
 * 
 * @param state The current assembler state.
 * @param label The label associated with the directive.
 * @param params The parameters of the directive.
 * @param validLabel Indicates if the label is valid.
 */
void handle_data_directive(AssemblerState *state, const char *label, const char *params, int validLabel);

/**
 * @brief Handles the .string directive.
 * 
 * @param state The current assembler state.
 * @param label The label associated with the directive.
 * @param params The parameters of the directive.
 * @param validLabel Indicates if the label is valid.
 */
void handle_string_directive(AssemblerState *state, const char *label, const char *params, int validLabel);

/**
 * @brief Handles the .entry directive.
 * 
 * @param state The current assembler state.
 * @param label The label associated with the directive.
 */
void handle_entry_directive(AssemblerState *state, const char *label);

/**
 * @brief Handles the .extern directive.
 * 
 * @param state The current assembler state.
 * @param label The label associated with the directive.
 */
void handle_extern_directive(AssemblerState *state, const char *label);

/**
 * @brief Assembles an instruction.
 * 
 * @param state The current assembler state.
 * @param label The label associated with the instruction.
 * @param op The operation code.
 * @param operand1 The first operand.
 * @param operand2 The second operand.
 * @param validLabel Indicates if the label is valid.
 * @return An integer indicating success or failure.
 */
int assemble_instruction(AssemblerState *state, const char *label, const char *op, const char *operand1, const char *operand2, int validLabel);

/* First Pass Helper Functions */

/**
 * @brief Adds a label to the label table for checking.
 * 
 * @param state The current assembler state.
 * @param name The name of the label.
 * @param address The address of the label.
 */
void add_label_for_check(AssemblerState *state, const char *name, int address);

/**
 * @brief Gets the register number from an operand.
 * 
 * @param operand The operand string.
 * @return The register number.
 */
int get_register_number(const char *operand);

/**
 * @brief Gets the opcode for an operation.
 * 
 * @param operation The operation string.
 * @return The opcode.
 */
int get_opcode(const char *operation);

/**
 * @brief Encodes an immediate operand.
 * 
 * @param operand The operand string.
 * @return The encoded operand.
 */
char *encode_immediate_operand(const char *operand);

/**
 * @brief Gets the addressing mode for an operand.
 * 
 * @param operand The operand string.
 * @return The addressing mode.
 */
int get_addressing_mode(const char *operand);

/**
 * @brief Frees the memory allocated for the assembler state.
 * 
 * @param state The assembler state to free.
 */
void free_assembler_state(AssemblerState *state);

/**
 * @brief Adds a label to the label table.
 * 
 * @param state The current assembler state.
 * @param name The name of the label.
 * @param address The address of the label.
 */
void add_label(AssemblerState *state, const char *name, int address);

/**
 * @brief Checks if an instruction is a single-operand instruction.
 * 
 * @param op The operation string.
 * @return 1 if it's a single-operand instruction, 0 otherwise.
 */
int is_single_operand_instruction(const char *op);

/**
 * @brief Updates the addresses of entry labels.
 * 
 * @param state The current assembler state.
 */
void update_entry_addresses(AssemblerState *state);

/**
 * @brief Prints the contents of the memory.
 * 
 * @param state The current assembler state.
 */
void print_memory(AssemblerState *state);

/* Second Pass Functions */

/**
 * @brief Performs the second pass of the assembly process.
 * 
 * @param state The current assembler state.
 * @param input_filename The name of the input file.
 * @param output_filename The name of the output file.
 * @param error A pointer to store any error code.
 */
void second_pass(AssemblerState *state, const char *input_filename, const char *output_filename, int *error);

/**
 * @brief Creates the entry file.
 * 
 * @param state The current assembler state.
 * @param filename The base filename.
 * @param entFilename The name of the entry file to be created.
 * @return An integer indicating success or failure.
 */
int createEntryFile(AssemblerState *state, const char *filename, char *entFilename);

/**
 * @brief Creates the extern file.
 * 
 * @param state The current assembler state.
 * @param filename The base filename.
 * @param extFilename The name of the extern file to be created.
 * @return An integer indicating success or failure.
 */
int createExternFile(AssemblerState *state, const char *filename, char *extFilename);

/* Utility Functions */

/**
 * @brief Converts a decimal number to its octal representation.
 * 
 * @param decimal The decimal number to convert.
 * @param octalStr The string to store the octal representation.
 */
void decimalToOctal(int decimal, char *octalStr);

/**
 * @brief Converts a binary string to its decimal equivalent.
 * 
 * @param binaryStr The binary string to convert.
 * @return The decimal equivalent.
 */
int binaryToDecimal(const char *binaryStr);

/**
 * @brief Gets the address of a label.
 * 
 * @param state The current assembler state.
 * @param label The label to look up.
 * @return The address of the label.
 */
int get_label_address(AssemblerState *state, const char *label);

/**
 * @brief Converts a decimal number to its 12-bit binary representation.
 * 
 * @param n The decimal number to convert.
 * @param binaryStr The string to store the binary representation.
 */
void decimalToBinary12(int n, char *binaryStr);

/**
 * @brief Creates a filename with a specific extension.
 * 
 * @param filename The base filename.
 * @param outputFilename The resulting filename with the extension.
 * @param extension The extension to add.
 */
void createExtension(const char *filename, char *outputFilename, const char *extension);


/**
 * @brief A custom implementation of snprintf with limited functionality.
 *
 * This function writes a formatted string to a character array, similar to snprintf,
 * but with limited format specifiers. It only supports %s for string formatting.
 *
 * @param str The buffer to write the formatted string to.
 * @param size The maximum number of bytes to be written to the buffer.
 * @param format The format string (only %s is supported).
 * @param arg The string argument to be formatted.
 *
 * @return The number of characters that would have been written if size had been sufficiently large,
 *         not counting the terminating null byte. If an encoding error occurs, a negative number is returned.
 *
 * @note This function only supports the %s format specifier for string formatting.
 * @note The resulting string is always null-terminated, unless size is 0.
 */
int my_snprintf(char *str, size_t size, const char *format, const char *arg);

/**
 * @brief Creates a duplicate of the input string.
 *
 * This function allocates memory for a new string and copies the contents
 * of the input string into it.
 *
 * @param s The input string to be duplicated.
 *
 * @return A pointer to the newly allocated string containing a copy of 's',
 *         or NULL if insufficient memory was available.
 *
 * @note The caller is responsible for freeing the memory allocated by this function.
 */
char *my_strdup(const char *s);
#endif
