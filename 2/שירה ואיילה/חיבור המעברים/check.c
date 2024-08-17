#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <ctype.h>
#include <stdbool.h>
#include <limits.h>

#include "check.h"

#define MAX_LINE_LENGTH 81
#define MAX_LABEL_LENGTH 31
#define MAX_LABELS 1000

typedef struct {
    char name[MAX_LABEL_LENGTH + 1];
    int address;
    bool is_external;
    bool is_entry;
} Label;

typedef struct {
    Label labels[MAX_LABELS];
    int label_count;
    bool stop_encountered;
    int line_count;
} AssemblyState;

const char* reserved_words[] = {"mov", "cmp", "add", "sub", "lea", "clr", "not", "inc", "dec", "jmp", "bne", "red", "prn", "jsr", "rts", "stop"};
const int num_reserved_words = sizeof(reserved_words) / sizeof(reserved_words[0]);

// Function prototypes
bool check_line_length(const char* line);
bool is_empty_or_whitespace(const char* line);
bool is_valid_comment(const char* line);
bool is_valid_label(const char* label);
bool is_reserved_word(const char* word);
bool is_duplicate_label(const char* label, const AssemblyState* state);
bool is_valid_instruction(const char* instruction);
bool is_valid_immediate(const char* operand);
bool check_operand_count(const char* instruction, int operand_count);
bool is_valid_operand(const char* operand);
bool is_valid_addressing_mode(const char* instruction, const char* operand, bool is_source);
void add_label(AssemblyState* state, const char* name, int address);
bool label_exists(const AssemblyState* state, const char* name);
void analyze_line(const char* line, int line_number, AssemblyState* state);
bool is_valid_entry_label(const char* label);
bool is_entry_label_defined(const AssemblyState* state, const char* label);
bool is_extern_label_not_defined(const AssemblyState* state, const char* label);

/**
 * Checks if the line length is valid (<=MAX_LINE_LENGTH)
 */
bool check_line_length(const char* line) {
    return strlen(line) <= MAX_LINE_LENGTH;
}

/**
 * Checks if the line is empty or contains only whitespace
 */
bool is_empty_or_whitespace(const char* line) {
    while (*line) {
        if (!isspace((unsigned char)*line))
            return false;
        line++;
    }
    return true;
}

/**
 * Checks if the line is a valid comment (starts with ';')
 */
bool is_valid_comment(const char* line) {
    return line[0] == ';';
}

/**
 * Checks if the label is valid (starts with a letter, contains only alphanumeric characters, and <=MAX_LABEL_LENGTH)
 */
bool is_valid_label(const char* label) {
    int len = 0;
    if (!isalpha(label[0]))
        return false;
    
    while (label[len] && label[len] != ':') {
        if (!isalnum(label[len]))
            return false;
        len++;
    }
    
    return len > 0 && len <= MAX_LABEL_LENGTH;
}

/**
 * Checks if the word is a reserved word
 */
bool is_reserved_word(const char* word) {
    for (int i = 0; i < num_reserved_words; i++) {
        if (strcmp(word, reserved_words[i]) == 0)
            return true;
    }
    return false;
}

/**
 * Checks if the label is already defined in the assembly state
 */
bool is_duplicate_label(const char* label, const AssemblyState* state) {
    for (int i = 0; i < state->label_count; i++) {
        if (strcmp(state->labels[i].name, label) == 0) {
            return true;
        }
    }
    return false;
}

/**
 * Checks if the instruction is valid (is a reserved word)
 */
bool is_valid_instruction(const char* instruction) {
    return is_reserved_word(instruction);
}

/**
 * Checks if the immediate operand is valid (-2078 <= value <= 2047)
 */
bool is_valid_immediate(const char* operand) {
    char* endptr;
    long value = strtol(operand + 1, &endptr, 10);
    return *endptr == '\0' && value >= -2078 && value <= 2047;
}

/**
 * Checks if the number of operands is correct for the given instruction
 */
bool check_operand_count(const char* instruction, int operand_count) {
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
    
    return false;
}

/**
 * Checks if the operand is valid (immediate, register, or label)
 */
bool is_valid_operand(const char* operand) {
    if (operand[0] == '#') {
        return is_valid_immediate(operand);
    }
    
    if (operand[0] == '*') {
        return (strlen(operand) == 2 && operand[1] == 'r') ||
               (strlen(operand) == 3 && operand[1] == 'r' && operand[2] >= '0' && operand[2] <= '7');
    }
    
    if (operand[0] == 'r' && strlen(operand) == 2) {
        return operand[1] >= '0' && operand[1] <= '7';
    }
    
    return is_valid_label(operand);
}

/**
 * Checks if the addressing mode is valid for the given instruction and operand
 */
bool is_valid_addressing_mode(const char* instruction, const char* operand, bool is_source) {
    bool is_immediate = (operand[0] == '#');
    bool is_direct_register = (operand[0] == 'r' && strlen(operand) == 2 && operand[1] >= '0' && operand[1] <= '7');
    bool is_indirect_register = (operand[0] == '*' && strlen(operand) == 3 && operand[1] == 'r' && operand[2] >= '0' && operand[2] <= '7');
    bool is_direct = (!is_immediate && !is_direct_register && !is_indirect_register);

    if (strcmp(instruction, "lea") == 0) {
        return is_source ? is_direct : !is_immediate;
    }
    
    if (strcmp(instruction, "cmp") == 0) {
        return true;
    }
    
    if (strcmp(instruction, "mov") == 0 || strcmp(instruction, "add") == 0 || strcmp(instruction, "sub") == 0) {
        return is_source || !is_immediate;
    }
    
    if (strcmp(instruction, "clr") == 0 || strcmp(instruction, "not") == 0 ||
        strcmp(instruction, "inc") == 0 || strcmp(instruction, "dec") == 0 ||
        strcmp(instruction, "red") == 0) {
        return !is_immediate;
    }
    
    if (strcmp(instruction, "jmp") == 0 || strcmp(instruction, "bne") == 0 ||
        strcmp(instruction, "jsr") == 0) {
        return is_direct || is_indirect_register;
    }
    
    if (strcmp(instruction, "prn") == 0) {
        return true;
    }

    return false;
}

/**
 * Adds a label to the assembly state
 */
void add_label(AssemblyState* state, const char* name, int address) {
    if (state->label_count < MAX_LABELS) {
        strncpy(state->labels[state->label_count].name, name, MAX_LABEL_LENGTH);
        state->labels[state->label_count].name[MAX_LABEL_LENGTH] = '\0';
        state->labels[state->label_count].address = address;
        state->labels[state->label_count].is_external = false;
        state->labels[state->label_count].is_entry = false;
        state->label_count++;
    }
}

/**
 * Checks if a label exists in the assembly state
 */
bool label_exists(const AssemblyState* state, const char* name) {
    for (int i = 0; i < state->label_count; i++) {
        if (strcmp(state->labels[i].name, name) == 0) {
            return true;
        }
    }
    return false;
}

/**
 * Checks if the entry label is valid (contains only uppercase letters)
 */
bool is_valid_entry_label(const char* label) {
    while (*label) {
        if (!isupper(*label)) {
            return false;
        }
        label++;
    }
    return true;
}

/**
 * Checks if the entry label is defined in the assembly state
 */
bool is_entry_label_defined(const AssemblyState* state, const char* label) {
    return label_exists(state, label);
}

/**
 * Checks if the extern label is not defined in the assembly state
 */
bool is_extern_label_not_defined(const AssemblyState* state, const char* label) {
    return !label_exists(state, label);
}

/**
 * Analyzes a single line of assembly code
 */
void analyze_line(const char* line, int line_number, AssemblyState* state) {
    char copy_line[MAX_LINE_LENGTH + 1];
    strncpy(copy_line, line, MAX_LINE_LENGTH);
    copy_line[MAX_LINE_LENGTH] = '\0';

    char* trimmed_line = copy_line;
    while (isspace(*trimmed_line)) trimmed_line++;
    char* end = trimmed_line + strlen(trimmed_line) - 1;
    while (end > trimmed_line && isspace(*end)) end--;
    *(end + 1) = '\0';

    if (!check_line_length(trimmed_line)) {
        printf("Error in line %d: Line length exceeds the maximum allowed\n", line_number);
        return;
    }

    if (is_empty_or_whitespace(trimmed_line) || is_valid_comment(trimmed_line)) {
        return;
    }

    char* token = strtok(trimmed_line, " \t");
    if (token == NULL) return;

    // Check for .entry and .extern directives
    if (strcmp(token, ".entry") == 0 || strcmp(token, ".extern") == 0) {
        bool is_entry = (strcmp(token, ".entry") == 0);
        token = strtok(NULL, " \t");
        if (token && (is_entry ? is_valid_entry_label(token) : is_valid_label(token))) {
            if (is_entry && !is_entry_label_defined(state, token)) {
                printf("Error in line %d: .entry label is not defined in the file\n", line_number);
            } else if (!is_entry && !is_extern_label_not_defined(state, token)) {
                printf("Error in line %d: .extern label is already defined in the file\n", line_number);
            }
        } else {
            printf("Error in line %d: Invalid %s label\n", line_number, is_entry ? ".entry" : ".extern");
        }
        return;
    }

    char label[MAX_LABEL_LENGTH + 1] = "";
    if (strchr(token, ':')) {
        strncpy(label, token, MAX_LABEL_LENGTH);
        label[MAX_LABEL_LENGTH] = '\0';
        char* colon = strchr(label, ':');
        if (colon) *colon = '\0';
        
        if (!is_valid_label(label)) {
            printf("Error in line %d: Invalid label\n", line_number);
            return;
        }
        if (is_reserved_word(label)) {
            printf("Error in line %d: Reserved word used as label\n", line_number);
            return;
        }
        if (is_duplicate_label(label, state)) {
            printf("Error in line %d: Duplicate label definition\n", line_number);
            return;
        }
        add_label(state, label, line_number);
        token = strtok(NULL, " \t");
        if (token == NULL) return;
    }

    char instruction[MAX_LABEL_LENGTH + 1];
    strncpy(instruction, token, MAX_LABEL_LENGTH);
    instruction[MAX_LABEL_LENGTH] = '\0';

    if (!is_valid_instruction(instruction)) {
        printf("Error in line %d: Invalid instruction\n", line_number);
        return;
    }

    if (strcmp(instruction, "stop") == 0) {
        if (state->stop_encountered) {
            printf("Error in line %d: STOP instruction already appeared\n", line_number);
            return;
        }
        state->stop_encountered = true;
    }

    char* rest_of_line = strtok(NULL, "\n");
    if (rest_of_line != NULL) {
        while (isspace(*rest_of_line)) rest_of_line++;
        
        if (*rest_of_line == ',') {
            printf("Error in line %d: Invalid character after instruction\n", line_number);
            return;
        }
    }

    int operand_count = 0;
    char* operands[2] = {NULL, NULL};

    if (rest_of_line != NULL) {
        char* operand = strtok(rest_of_line, ",");
        while (operand != NULL && operand_count < 2) {
            while (isspace(*operand)) operand++;
            char* end = operand + strlen(operand) - 1;
            while (end > operand && isspace(*end)) end--;
            *(end + 1) = '\0';

            operands[operand_count] = operand;
            operand_count++;

            if (!is_valid_operand(operand)) {
                printf("Error in line %d: Invalid operand\n", line_number);
                return;
            }

            operand = strtok(NULL, ",");
        }
    }

    if (!check_operand_count(instruction, operand_count)) {
        printf("Error in line %d: Incorrect number of operands for instruction\n", line_number);
        return;
    }

    if (operand_count > 0 && !is_valid_addressing_mode(instruction, operands[0], true)) {
        printf("Error in line %d: Invalid addressing mode for source operand\n", line_number);
        return;
    }
    if (operand_count > 1 && !is_valid_addressing_mode(instruction, operands[1], false)) {
        printf("Error in line %d: Invalid addressing mode for destination operand\n", line_number);
        return;
    }

    for (int i = 0; i < operand_count; i++) {
        if (operands[i][0] == '#' && !is_valid_immediate(operands[i])) {
            printf("Error in line %d: Invalid number in immediate addressing mode\n", line_number);
            return;
        }
    }

    state->line_count++;
}

/**
 * Main function - analyzes the input file
 */
int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <input_file>\n", argv[0]);
        return 1;
    }

    FILE* file = fopen(argv[1], "r");
    if (file == NULL) {
        perror("Error opening file");
        return 1;
    }

    AssemblyState state = {0};
    char line[MAX_LINE_LENGTH + 1];
    int line_number = 0;

    while (fgets(line, sizeof(line), file)) {
        line_number++;
        line[strcspn(line, "\n")] = 0;
        analyze_line(line, line_number, &state);
    }

    fclose(file);

    if (!state.stop_encountered) {
        printf("Error: STOP instruction not found in the code\n");
    }

    return 0;
}