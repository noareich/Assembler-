/****************************************************************/
/* File containing integrity checks for pre-assembler, first pass, and second pass */
/****************************************************************/
#include "check.h"

/* Constants for numeric values */
#define MAX_IMMEDIATE_VALUE 2047
#define MIN_IMMEDIATE_VALUE -2078
#define MAX_REGISTER_NUMBER 7
#define MIN_REGISTER_NUMBER 0

/* Arrays for reserved words */
const char *instruction_set[] = {
    "mov", "cmp", "add", "sub", "lea",
    "not", "clr", "inc", "dec", "jmp",
    "bne", "red", "prn", "jsr", "rts", "stop"};
int const instruction_set_size = sizeof(instruction_set) / sizeof(instruction_set[0]);

const char *registers[] = {
    "r0", "r1", "r2", "r3", "r4", "r5",
    "r6", "r7"};
int const registers_size = sizeof(registers) / sizeof(registers[0]);

const char *Guidelines_set[] = {
    ".data", ".string", ".entry", ".extern", "define"};
int const Guidelines_set_size = sizeof(Guidelines_set) / sizeof(Guidelines_set[0]);

/******************************************/
/* General validity functions for all three passes */
/******************************************/

/* Checks if the line length is valid (<=MAX_LINE_LENGTH) */
int check_line_length(const char *line)
{
    return strlen(line) <= MAX_LINE_LENGTH;
}

/* Checks if the line is empty or contains only whitespace */
int is_empty_or_whitespace(const char *line)
{
    while (*line)
    {
        if (!isspace((unsigned char)*line))
            return 0;
        line++;
    }
    return 1;
}

/* Checks if the line is a valid comment (starts with ';') */
int is_valid_comment(const char *line)
{
    return line[0] == ';';
}

/********************************/
/* First pass validity functions */
/*******************************/

/* Checks if a word is in a given set of words */
int isInSet(const char *word, const char *set[], int setSize)
{
    int i;
    for (i = 0; i < setSize; i++)
    {
        if (strcmp(word, set[i]) == 0)
        {
            return 1; /* 1 - The word is in the array */
        }
    }
    return 0; /* 0 - The word is not in the array */
}

/* Checks if the label is valid */
int isValidLabel(const char *word, Macro *macros, int *macroCount)
{
    int length = strlen(word);
    int i;
    char label[MAX_LABEL_LENGTH + 1];

    /* Check label length */
    if (length > MAX_LABEL_LENGTH)
    { 

        fprintf(stderr, "Error at line %d: Label '%s' is invalid: It is too long\n", __LINE__, word);
        return 0; /* False */
    }

    /* Check first character */
    if (!isalpha(word[0]))
    {
        fprintf(stderr, "Error at line %d: Label '%s' is invalid: It must start with a letter\n", __LINE__, word);
        return 0; /* False */
    }

    /* Check remaining characters */
    for (i = 1; i < length - 1; i++)
    {

        if (!isalnum(word[i]))
        {
            fprintf(stderr, "Error at line %d: Label '%s' is invalid: It contains non-alphanumeric characters\n", __LINE__, word);
            return 0; /* False */
        }
    }

    /* Remove colon for reserved word check */
    strncpy(label, word, length - 1);
    label[length - 1] = '\0';

    /* Check if label is a reserved word */
    if (isInSet(label, instruction_set, instruction_set_size) ||
        isInSet(label, Guidelines_set, Guidelines_set_size) ||
        isInSet(label, registers, registers_size))
    {

        fprintf(stderr, "Error at line %d: Label '%s' is invalid: It is a reserved word or a macro name\n", __LINE__, word);
        return 0; /* False */
    }

    /* Check if label is a macro */
    for (i = 0; i < *macroCount; i++)
    {
        if (strcmp(label, macros[i].name) == 0)
        {
            fprintf(stderr, "Error at line %d: Label '%s' is invalid: It is already defined as a macro\n", __LINE__, word);
            return 0; /* False */
        }
    }


    return 1; /* True */

}

/* Checks the integrity of data directive */
int data_intergity_check(char *line)
{
    char *operands = my_strdup(line);
    char *original_operands = operands; 
    int error = 0;
    char *token;
    char *endptr;

    while (isspace(*operands))
        operands++;
    if (*operands == '\0')
    {
        fprintf(stderr, "Error at line %d: No data values provided after .data\n", __LINE__);
        error = 1;
    }

    if (has_invalid_characters(operands))
    {
        fprintf(stderr, "Error at line %d: Invalid characters in .data directive operands\n", __LINE__);
        error = 1;
    }

    token = strtok(operands, ",");
    while (token != NULL)
    {
        strtol(token, &endptr, 10);
        if (*endptr != '\0' && !isspace(*endptr))
        {
            fprintf(stderr, "Error at line %d: Invalid number format in .data directive operands\n", __LINE__);
            error = 1;
        }
        token = strtok(NULL, ",");
    }

    free(original_operands);  

    if (error == 1)
    {
        return 1;
    }

    return 0;
}

/* Checks for invalid characters in data directive */
int has_invalid_characters(const char *str)
{
    while (*str)
    {
        if (!isdigit(*str) && *str != '-' && *str != '+' && !isspace(*str) && *str != ',' && *str != ' ')
        {
            return 1; /* TRUE */
        }
        str++;
    }
    return 0; /* FALSE */
}

/* Checks the integrity of string directive */
int string_intergity_check(char *line)
{
    int error = 0;
    char *operands = my_strdup(line);
    char *original_operands = operands;  

    if (operands == NULL)
    {
        fprintf(stderr, "Error at line %d: There is no string.\n", __LINE__);
        return 1;
    }

    if (*operands != '"')
    {
        fprintf(stderr, "Error at line %d: String must start with a double quote (\").\n", __LINE__);
        error = 1;
    }
    else
    {
        operands++;

        while (*operands != '"' && *operands != '\0')
        {
            if (!isprint(*operands))
            {
                fprintf(stderr, "Error at line %d: Non-printable character found in string.\n", __LINE__);
                error = 1;
            }
            operands++;
        }

        if (*operands != '"')
        {
            fprintf(stderr, "Error at line %d: String must end with a double quote (\").\n", __LINE__);
            error = 1;
        }
        else
        {
            operands++;
            while (*operands != '\0')
            {
                if (!isspace(*operands))
                {
                    fprintf(stderr, "Error at line %d: Additional characters found after the string.\n", __LINE__);
                    error = 1;
                }
                operands++;
            }
        }
    }

    free(original_operands);  

    return error;
}

/* Checks the integrity of entry directive */
int entry_intergity_check(char *line, AssemblerState *state, Macro *macros, int *macroCount)

{
    int error = 0;
    char *operands = NULL;
    char *label = NULL;
    char *label_end;
    int label_length;
    int extraCharsFound = 0;
char *trimmed_operands;
    operands = my_strdup(line);
    if (operands == NULL) {
        fprintf(stderr, "Error at line %d: Memory allocation failed.\n", __LINE__);
        return 1;
    }

     trimmed_operands = operands;
    while (isspace(*trimmed_operands))
        trimmed_operands++; /* Remove leading spaces */

    /* Check for additional characters between the directive and the label */
    if (*trimmed_operands != '\0' && !isalpha(*trimmed_operands))
    {
        fprintf(stderr, "Error at line %d: Invalid character '%c' found between .entry and the label.\n", __LINE__, *trimmed_operands);
        error = 1;
        goto cleanup;
    }

    /* Find the end of the label */
    label_end = trimmed_operands;
    while (*label_end != '\0' && !isspace(*label_end))
    {
        label_end++;
    }

    label_length = label_end - trimmed_operands;
    label = (char *)malloc(label_length + 1);
    if (label == NULL)
    {
        fprintf(stderr, "Error at line %d: Memory allocation failed.\n", __LINE__);
        error = 1;
        goto cleanup;
    }
    strncpy(label, trimmed_operands, label_length);
    label[label_length] = '\0';

    /* Check for additional characters after the label */
    while (*label_end != '\0')
    {
        if (!isspace(*label_end))
        {
            extraCharsFound = 1;
            break;
        }
        label_end++;
    }

    if (extraCharsFound)
    {
        fprintf(stderr, "Error at line %d: Additional characters found after the label '%s'.\n", __LINE__, label);
        error = 1;
    }

    /* Check label validity */
    if (!error && !isValidLabel(label, macros, macroCount))
    {
        fprintf(stderr, "Error at line %d: Invalid label '%s' in .entry directive.\n", __LINE__, label);
        error = 1;
    }

cleanup:
    free(label);
    free(operands);

    return error;
}
/* Checks the integrity of extern directive */
int extern_intergity_check(char *line, AssemblerState *state)
{
    int error = 0;
    char *operands = my_strdup(line);
    char *label_end;
    int label_length;
    char *label = NULL;
    int extraCharsFound = 0;

    if (operands == NULL) {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        return 1;
    }

    while (isspace(*operands))
        operands++;

    /* Check for invalid characters before the label */
    if (*operands != '\0' && !isalpha(*operands))
    {
        fprintf(stderr, "Error: Invalid character '%c' found between .extern and the label.\n", *operands);
        error = 1;
        goto cleanup;
    }

    label_end = operands;
    while (*label_end != '\0' && !isspace(*label_end))
    {
        label_end++;
    }

    label_length = label_end - operands;
    label = (char *)malloc(label_length + 1);
    if (label == NULL)
    {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        error = 1;
        goto cleanup;
    }
    strncpy(label, operands, label_length);
    label[label_length] = '\0';

    while (*label_end != '\0')
    {
        if (!isspace(*label_end))
        {
            extraCharsFound = 1;
            break;
        }
        label_end++;
    }

    if (extraCharsFound)
    {
        fprintf(stderr, "Error: Additional characters found after the label '%s'.\n", label);
        error = 1;
    }

cleanup:
    free(label);
    free(operands);
    return error;
}

/* Checks if the word is a reserved word */
int is_reserved_word(const char *word)
{
    int i;
    for (i = 0; i < instruction_set_size; i++)
    {
        if (strcmp(word, instruction_set[i]) == 0)
            return 1;
    }
    return 0;
}

/* Checks if the label is already defined in the assembly state */
int is_duplicate_label(const char *label, const AssemblerState *state)
{
    int i;
    for (i = 0; i < state->label_count; i++)
    {
        if (strcmp(state->label_table[i].name, label) == 0)
        {
            return 1;
        }
    }
    return 0;
}

/* Checks if the instruction is valid (is a reserved word) */
int is_valid_instruction(const char *instruction)
{
    return is_reserved_word(instruction);
}

/* Checks if the immediate operand is valid (MIN_IMMEDIATE_VALUE <= value <= MAX_IMMEDIATE_VALUE) */
int is_valid_immediate(const char *operand)
{
    char *endptr;
    long value = strtol(operand + 1, &endptr, 10);
    return *endptr == '\0' && value >= MIN_IMMEDIATE_VALUE && value <= MAX_IMMEDIATE_VALUE;
}

/* Checks if the number of operands is correct for the given instruction */
int check_operand_count(const char *instruction, int operand_count)
{
    if (strcmp(instruction, "mov") == 0 || strcmp(instruction, "cmp") == 0 ||
        strcmp(instruction, "add") == 0 || strcmp(instruction, "sub") == 0 ||
        strcmp(instruction, "lea") == 0)
        return operand_count == 2;

    if (strcmp(instruction, "clr") == 0 || strcmp(instruction, "not") == 0 ||
        strcmp(instruction, "inc") == 0 || strcmp(instruction, "dec") == 0 ||
        strcmp(instruction, "jmp") == 0 || strcmp(instruction, "bne") == 0 ||
        strcmp(instruction, "red") == 0 || strcmp(instruction, "prn") == 0 ||
        strcmp(instruction, "jsr") == 0)
        return operand_count == 1;

    if (strcmp(instruction, "rts") == 0 || strcmp(instruction, "stop") == 0)
        return operand_count == 0;

    return 0;
}

/* Checks if the operand is valid (immediate, register, or label) */
int is_valid_operand(const char *operand, Macro *macros, int *macroCount)
{
    if (operand[0] == '#')
    {
        return is_valid_immediate(operand);
    }

    if (operand[0] == '*')
    {
        return (strlen(operand) == 2 && operand[1] == 'r') ||
               (strlen(operand) == 3 && operand[1] == 'r' && operand[2] >= '0' && operand[2] <= '7');
    }

    if (operand[0] == 'r' && strlen(operand) == 2)
    {
        return operand[1] >= '0' && operand[1] <= '7';
    }

    return isValidLabel(operand, macros, macroCount);
}

/* Checks if the addressing mode is valid for the given instruction and operand */
int is_valid_addressing_mode(const char *instruction, const char *operand, int is_source)
{
    int is_immediate = (operand[0] == '#');
    int is_direct_register = (operand[0] == 'r' && strlen(operand) == 2 && operand[1] >= '0' && operand[1] <= '7');
    int is_indirect_register = (operand[0] == '*' && strlen(operand) == 3 && operand[1] == 'r' && operand[2] >= '0' && operand[2] <= '7');
    int is_direct = (!is_immediate && !is_direct_register && !is_indirect_register);

    if (strcmp(instruction, "lea") == 0)
    {
        return is_source ? is_direct : !is_immediate;
    }

    if (strcmp(instruction, "cmp") == 0)
    {
        return 1;
    }

    if (strcmp(instruction, "mov") == 0 || strcmp(instruction, "add") == 0 || strcmp(instruction, "sub") == 0)
    {
        return is_source || !is_immediate;
    }

    if (strcmp(instruction, "clr") == 0 || strcmp(instruction, "not") == 0 ||
        strcmp(instruction, "inc") == 0 || strcmp(instruction, "dec") == 0 ||
        strcmp(instruction, "red") == 0)
    {
        return !is_immediate;
    }

    if (strcmp(instruction, "jmp") == 0 || strcmp(instruction, "bne") == 0 ||
        strcmp(instruction, "jsr") == 0)
    {
        return is_direct || is_indirect_register;
    }

    if (strcmp(instruction, "prn") == 0)
    {
        return 1;
    }

    return 0;
}

/* Checks if a label exists in the assembly state */
int label_exists(const AssemblerState *state, const char *name)
{
    int i;
    for (i = 0; i < state->label_count; i++)
    {
        if (strcmp(state->label_table[i].name, name) == 0)
        {
            return 0;
        }
    }
    fprintf(stderr, "Error at line %d: Label '%s' does not exist in the assembly state.\n", __LINE__, name);
    return 1;
}

/********************************/
/* Second pass validity functions */
/*******************************/

/* Checks if the entry label is valid (contains only uppercase letters) */
int is_valid_entry_label(const char *label)
{
    while (*label)
    {
        if (!isupper(*label))
        {
            fprintf(stderr, "Error at line %d: Invalid entry label '%s'. Entry labels must contain only uppercase letters.\n", __LINE__, label);
            return 0;
        }
        label++;
    }
    return 1;
}

/* Checks if the entry label is defined in the assembly state */
int is_entry_label_defined(const AssemblerState *state, const char *label)
{
    return label_exists(state, label);
}


/* Checks if the extern label is defined as an entry in the assembly state */
int is_extern_label_defined_as_entry(const AssemblerState *state, const char *label)
{
    int i;
    for (i = 0; i < state->entry_count; i++)
    {
        if (strcmp(state->entry_table[i].name, label) == 0)
        {
            fprintf(stderr, "Error at line %d: Extern label '%s' is also defined as an entry label.\n", __LINE__, label);
            return 0;
        }
    }
    return 1;
}
