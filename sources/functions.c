/****************************************************************/
/* File containing helper functions for pre-assembler, first pass, and second pass */
/****************************************************************/
#include "assembler.h"
#include <ctype.h>
#include <limits.h>

#define MAX_RESERVED_WORD_LENGTH 10
#define RESERVED_WORD_COUNT 31
#define BINARY_LENGTH 12
#define MAX_OCTAL_LENGTH 6
#define IMMEDIATE_MIN -2048
#define IMMEDIATE_MAX 2047
#define REGISTER_PREFIX 'r'
#define IMMEDIATE_PREFIX '#'
#define INDIRECT_PREFIX '*'
#define COMMENT_PREFIX ';'

char strings[RESERVED_WORD_NUM][MAX_RESERVED_WORD_LENGTH] = {
    ".data", ".string", ".entry", ".extern", "define",
    "macr", "endmacr", 
    "mov", "cmp", "add", "sub", "lea",
    "not", "clr", "inc", "dec", "jmp",
    "bne", "red", "prn", "jsr", "rts", "stop",
    "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7"
};

/*******************/
/* General functions */
/*******************/

/* Custom implementation of strtok_r for Visual Studio compatibility */
char *strtok_r(char *str, const char *delim, char **saveptr) 
	{
    char *token;

    if (str == NULL)
 	{
        str = *saveptr;
    }

    str += strspn(str, delim);
    if (*str == '\0') 
	{
        return NULL;
    }

    token = str;
    str = strpbrk(token, delim);
    if (str == NULL) 
	{
        *saveptr = strchr(token, '\0');
    } else {
        *str = '\0';
        *saveptr = str + 1;
    }

    return token;
}

/* Trims leading and trailing whitespace from a string */
char *trim(char *str)
{
    char *end;
    while (isspace((unsigned char)*str))
        str++;
    if (*str == 0)
        return str;
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end))
        end--;
    end[1] = '\0';
    return str;
}

/* Converts an integer to its binary representation */
char *int_to_binary(int value, int bits)
{
    char *binary;
    int i;

    binary = (char *)malloc(bits + 1);
    if (binary == NULL)
    {
        fprintf(stderr, "Error: Failed to allocate memory.\n");
        return NULL;
    }

    binary[bits] = '\0';
    for (i = bits - 1; i >= 0; i--)
    {
        binary[i] = (value & 1) ? '1' : '0';
        value >>= 1;
    }

    return binary;
}

/* Creates a new file extension for the output file */
void addExtension(char* filename, const char* extension, char* result) 
{
    strcpy(result, filename);
    strcat(result, extension);
}

/* Removes leading whitespace from a line */
void trimLeadingWhitespace(char *str) 
{
    char *start = str;
    int is_comment = 0;

    while (isspace((unsigned char)*start)) 
{
        start++;
    }

    if (*start == COMMENT_PREFIX) 
{
        is_comment = 1;
    }

    if (is_comment)
 {
        if (str != start) 
{
            printf("Error: Leading whitespace before comment on line: %s\n", str);
        }
    } 
else 
{
        if (start != str) 
{
            memmove(str, start, strlen(start) + 1);
        }
    }
}

/* Checks if a line has leading whitespace */
int has_leading_whitespace(const char *line)
 {
    while (*line != '\0' && isspace((unsigned char)*line)) 
{
        line++;
    }
    return *line != '\0';
}

/***************************************************************
 * Functions related to building the macro table - macro_table.c file
 ***************************************************************/

/* Frees the memory allocated for the macro table */
void freeMacros(Macro* macros, int macroCount)
 {
    int i;
    for (i = 0; i < macroCount; i++) 
{
        free(macros[i].name);
        free(macros[i].content);
    }
    free(macros);
}

/* Custom implementation of strdup */
char *my_strdup(const char *s)
 {
    char *new_str;
    size_t len;

    if (s == NULL)
 {
        return NULL;
    }

    len = strlen(s) + 1;
    new_str = (char *)malloc(len);
    
    if (new_str == NULL)
 {
        return NULL;
    }

    memcpy(new_str, s, len);
    return new_str;
}

/* Checks if a line is a comment */
int is_comment(const char* line) 
{
    char* trim_line;
    char* token;
    int result;

    trim_line = my_strdup(line);
    if (trim_line == NULL) 
{
        return 0; /* Memory allocation failed, assume it's not a comment */
    }

    token = strtok(trim_line, " \t\n");
    if (token != NULL && token[0] == COMMENT_PREFIX)
 {
        result = 1; /* The line is a comment */
    } 
else
 {
        result = 0; /* The line is not a comment */
    }

    free(trim_line);
    return result;
}

/* Checks if a line is empty */
int is_empty_line(const char* line)
 {
    char* trim_line;
    char* token;
    int result;

    trim_line = my_strdup(line);
    if (trim_line == NULL) {
        return 1; /* Memory allocation failed, assume it's an empty line */
    }

    token = strtok(trim_line, " \t\n");
    if (token == NULL) {
        result = 1; /* The line is empty */
    } else {
        result = 0; /* The line is not empty */
    }

    free(trim_line);
    return result;
}

/* Checks if a line contains only whitespace characters */
int is_whitespace_line(const char* line)
 {
    while (*line) 
{
        if (!isspace(*line)) 
{
            return 0; /* Found a non-space character */
        }
        line++;
    }
    return 1; /* The entire line contains only whitespace characters */
}

/* Checks if a line inside a macro is empty or contains only whitespace */
int is_empty_macro_line(const char* line) 
{
    return is_whitespace_line(line);
}

/*****************************************************
 * Functions related to the first pass - first_pass.c file
 *****************************************************/

/* Adds a label to the assembly state for checking */
void add_label_for_check(AssemblerState* state, const char* name, int address)
 {
    if (state->label_count < MAX_LABELS)
 {
        strncpy(state->label_table[state->label_count].name, name, MAX_LABEL_LENGTH);
        state->label_table[state->label_count].name[MAX_LABEL_LENGTH] = '\0';
        state->label_table[state->label_count].address = address;
        state->label_table[state->label_count].is_extern = 0;
        state->label_table[state->label_count].is_entry = 0;
        state->label_count++;
    }
}

/* Gets the register number from an operand */
int get_register_number(const char *operand)
{
    if (operand[0] == REGISTER_PREFIX && isdigit(operand[1]))
    {
        return operand[1] - '0';
    }
    return -1; /* Not a register */
}

/* Gets the opcode for a given operation */
int get_opcode(const char *operation)
{
    if (strcmp(operation, "mov") == 0) return 0;
    if (strcmp(operation, "cmp") == 0) return 1;
    if (strcmp(operation, "add") == 0) return 2;
    if (strcmp(operation, "sub") == 0) return 3;
    if (strcmp(operation, "lea") == 0) return 4;
    if (strcmp(operation, "clr") == 0) return 5;
    if (strcmp(operation, "not") == 0) return 6;
    if (strcmp(operation, "inc") == 0) return 7;
    if (strcmp(operation, "dec") == 0) return 8;
    if (strcmp(operation, "jmp") == 0) return 9;
    if (strcmp(operation, "bne") == 0) return 10;
    if (strcmp(operation, "red") == 0) return 11;
    if (strcmp(operation, "prn") == 0) return 12;
    if (strcmp(operation, "jsr") == 0) return 13;
    if (strcmp(operation, "rts") == 0) return 14;
    if (strcmp(operation, "stop") == 0) return 15;
    return -1; /* Invalid operation */
}

char *encode_immediate_operand(const char *operand)
{
    const char *number_str;
    char *endptr;
    long number;
    char *result;
    unsigned short unsigned_number;
    int i;

    if (operand == NULL || operand[0] != IMMEDIATE_PREFIX)
    {
        return NULL; /* Not an immediate operand */
    }

    number_str = operand + 1;
    number = strtol(number_str, &endptr, 10);

    if (endptr == number_str || *endptr != '\0')
    {
        return NULL; /* Invalid number */
    }

    if (number < IMMEDIATE_MIN || number > IMMEDIATE_MAX)
    {
        return NULL; /* Number out of range */
    }

    result = (char *)calloc(16, sizeof(char));
    if (result == NULL)
    {
        return NULL; /* Memory allocation failure */
    }

    unsigned_number = (unsigned short)(number & 0xFFF);
    for (i = 11; i >= 0; i--)
    {
        result[11 - i] = (unsigned_number & (1 << i)) ? '1' : '0';
    }

    result[12] = '1';
    result[13] = '0';
    result[14] = '0';
    result[15] = '\0';

    return result;
}


/* Gets the addressing mode for an operand */
int get_addressing_mode(const char *operand)
{
    if (operand[0] == IMMEDIATE_PREFIX)
        return 1; /* Immediate */
    if (operand[0] == REGISTER_PREFIX)
        return 8; /* Direct register */
    if (operand[0] == INDIRECT_PREFIX)
        return 4; /* Indirect register */
    return 2;     /* Direct (label) */
}

/* Frees the memory allocated for the assembler state */
void free_assembler_state(AssemblerState *state)
{
    if (state)
    {
        free(state->memory);
        free(state->extern_table);
        free(state->label_table);
        free(state->entry_table);
        free(state);
    }
}

/* Adds a label to the assembler state */
void add_label(AssemblerState *state, const char *name, int address)
{
    if (state->label_count == state->label_capacity)
    {
        state->label_capacity *= 2;
        state->label_table = realloc(state->label_table, state->label_capacity * sizeof(Label));
        if (!state->label_table)
        {
            perror("Failed to reallocate label table");
            return;
        }
    }

    strncpy(state->label_table[state->label_count].name, name, MAX_LABEL_LENGTH);
    state->label_table[state->label_count].name[MAX_LABEL_LENGTH] = '\0';
    state->label_table[state->label_count].address = address;
    state->label_table[state->label_count].is_entry = 0;
    state->label_table[state->label_count].is_extern = 0;
    state->label_count++;
}

/* Checks if an instruction has a single operand */
int is_single_operand_instruction(const char *op)
{
    int i;
    const char *single_operand_instructions[] = {
        "clr", "not", "inc", "dec", "jmp", "bne", "red", "prn", "jsr"};
    int num_instructions = sizeof(single_operand_instructions) / sizeof(single_operand_instructions[0]);

    for (i = 0; i < num_instructions; i++)
    {
        if (strcmp(op, single_operand_instructions[i]) == 0)
        {
            return 1;
        }
    }
    return 0;
}

/* Updates the addresses of entry labels */
void update_entry_addresses(AssemblerState *state)
{
    int i;
    for (i = 0; i < state->entry_count; i++)
    {
        int add_to_update = get_label_address(state, state->entry_table[i].name);
        int address = state->entry_table[i].address;
        if (address != -1)
        {
            fprintf(stderr, "Error: Entry label '%s' not found in symbol table\n", state->entry_table[i].name);
        }
        else
        {
            state->entry_table[i].address = add_to_update;
        }
    }
}

/* Prints the memory content of the assembler state */
void print_memory(AssemblerState *state)
{
    int i, j;
    for (i = 100; i < state->IC + state->DC; i++)
    {
        if (state->memory[i].label[0] != '\0')
        {
            int is_extern = 0;
            for (j = 0; j < state->extern_count; j++)
            {
                if (strcmp(state->extern_table[j].name, state->memory[i].label) == 0)
                {
                    is_extern = 1;
                    break;
                }
            }

            if (is_extern)
            {
                printf("%04d \"%s\"\n", i, state->memory[i].label);
            }
            else
            {
                printf("%04d \"%s\"\n", i, state->memory[i].label);
            }
        }
        else
        {
            printf("%04d %s\n", i, state->memory[i].binary);
        }
    }
}

/*****************************************************
 * Functions related to the second pass - second_pass.c file
 *****************************************************/

/* Gets the address of a label from the assembler state */
int get_label_address(AssemblerState *state, const char *label)
{
    int i;
    for (i = 0; i < state->label_count; i++)
    {
        if (strcmp(state->label_table[i].name, label) == 0)
        {
            return state->label_table[i].address;
        }
    }

    return -1; /* Return -1 if the label is not found */
}

/* Converts a decimal number to its 12-bit binary representation */
void decimalToBinary12(int n, char *binaryStr)
{
    int i;
    binaryStr[0] = '\0';
    for (i = 11; i >= 0; i--)
    {
        int bit = (n >> i) & 1;
        if (bit)
            strcat(binaryStr, "1");
        else
            strcat(binaryStr, "0");
    }
}

/* Converts a binary string to its decimal representation */
int binaryToDecimal(const char *binaryStr)
{
    return (int)strtol(binaryStr, NULL, 2);
}

/* Converts a decimal number to its octal representation */
void decimalToOctal(int decimal, char *octalStr)
{
    sprintf(octalStr, "%05o", decimal);
}

/* Creates a new file extension for the output file */
void createExtension(const char *filename, char *outputFilename, const char *extension)
{
    char *dot;
    strcpy(outputFilename, filename);
    dot = strrchr(outputFilename, '.');
    if (dot != NULL)
    {
        *dot = '\0';
    }
    strcat(outputFilename, extension);
}
/* This function formats a string with one argument into a buffer of limited size.
   It returns the number of characters that would have been written if the buffer was large enough. */
int my_snprintf(char *str, size_t size, const char *format, const char *arg) 
{
    char temp[1024];
    int written;

    written = sprintf(temp, format, arg);

    if (size > 0) {
        strncpy(str, temp, size - 1);
        str[size - 1] = '\0';
    }

    return written;
}
