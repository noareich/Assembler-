#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>

#define MAX_LINE_LENGTH 100
#define MAX_LABEL_LENGTH 31
#define MAX_LABELS 1000
#define MEMORY_SIZE 1000

typedef struct {
    int address;
    char binary[16];
    char label[MAX_LABEL_LENGTH + 1];
} Instruction;

typedef struct {
    char name[MAX_LABEL_LENGTH + 1];
    int address;
} Label;

Instruction memory[MEMORY_SIZE];
Label label_table[MAX_LABELS];
int label_count = 0;
int IC = 100;
int DC = 0;

char* trim(char* str);

void print_memory();

char* int_to_binary(int num, int length) {
    char* binary = (char*)malloc(length + 1);
    if (binary == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    binary[length] = '\0';
    for (int i = length - 1; i >= 0; i--) {
        binary[i] = (num & 1) ? '1' : '0';
        num >>= 1;
    }
    return binary;
}

int get_register_number(const char* reg) {
    if (reg[0] == 'r' && reg[1] >= '0' && reg[1] <= '7' && reg[2] == '\0') {
        return reg[1] - '0';
    }
    return -1;
}

void add_label(const char* name, int address) {
    if (label_count < MAX_LABELS) {
        strncpy(label_table[label_count].name, name, MAX_LABEL_LENGTH);
        label_table[label_count].name[MAX_LABEL_LENGTH] = '\0';
        label_table[label_count].address = address;
        label_count++;
    } else {
        fprintf(stderr, "Label table overflow\n");
    }
}

int get_label_address(const char* name) {
    for (int i = 0; i < label_count; i++) {
        if (strcmp(label_table[i].name, name) == 0) {
            return label_table[i].address;
        }
    }
    return -1;
}

void assemble_instruction(const char *label, const char *op, const char *operand1, const char *operand2) {
    int opcode = 0;
    int src_reg = -1, dst_reg = -1;
    int immediate_value = 0;
    char src_label[MAX_LABEL_LENGTH + 3] = ""; // +3 for quotes and null terminator
    char dst_label[MAX_LABEL_LENGTH + 3] = ""; // +3 for quotes and null terminator
    int src_addressing = 0, dst_addressing = 0;
    
    if (label && label[0] != '\0' && strncmp(label, "r", 1) != 0) {
        add_label(label, IC);
    }
    // Opcode assignment remains the same
    if (strcmp(op, "mov") == 0) opcode = 0;
    else if (strcmp(op, "cmp") == 0) opcode = 1;
    else if (strcmp(op, "add") == 0) opcode = 2;
    else if (strcmp(op, "sub") == 0) opcode = 3;
    else if (strcmp(op, "lea") == 0) opcode = 4;
    else if (strcmp(op, "clr") == 0) opcode = 5;
    else if (strcmp(op, "not") == 0) opcode = 6;
    else if (strcmp(op, "inc") == 0) opcode = 7;
    else if (strcmp(op, "dec") == 0) opcode = 8;
    else if (strcmp(op, "jmp") == 0) opcode = 9;
    else if (strcmp(op, "bne") == 0) opcode = 10;
    else if (strcmp(op, "red") == 0) opcode = 11;
    else if (strcmp(op, "prn") == 0) opcode = 12;
    else if (strcmp(op, "jsr") == 0) opcode = 13;
    else if (strcmp(op, "rts") == 0) opcode = 14;
    else if (strcmp(op, "stop") == 0) opcode = 15;

    // Analyze operands
   if (operand1) {
        if (operand1[0] == '#') {
            src_addressing = 1; // Immediate
            immediate_value = atoi(operand1 + 1);
        } else if (operand1[0] == '*') {
            src_addressing = 4; // Register indirect
            src_reg = get_register_number(operand1 + 1);
        } else if ((src_reg = get_register_number(operand1)) != -1) {
            src_addressing = 8; // Register direct
        } else {
            src_addressing = 2; // Direct (label)
            snprintf(src_label, sizeof(src_label), "\"%s\"", operand1);
        }
    }
    if (operand2) {
        if (operand2[0] == '#') {
            dst_addressing = 1; // Immediate
            immediate_value = atoi(operand2 + 1);
        } else if (operand2[0] == '*') {
            dst_addressing = 4; // Register indirect
            dst_reg = get_register_number(operand2 + 1);
        } else if ((dst_reg = get_register_number(operand2)) != -1) {
            dst_addressing = 8; // Register direct
        } else {
            dst_addressing = 2; // Direct (label)
            snprintf(dst_label, sizeof(dst_label), "\"%s\"", operand2);
        }
    }
    
    // First word
    int first_word = (opcode << 11) | (src_addressing << 7) | (dst_addressing << 3) | 0b100;
    char *first_word_binary = int_to_binary(first_word, 15);
    strcpy(memory[IC].binary, first_word_binary);
    free(first_word_binary);
    memory[IC].address = IC;
    strcpy(memory[IC].label, "");
    IC++;
    
    // Second word (if needed)
    if ((src_addressing == 4 || src_addressing == 8) && (dst_addressing == 4 || dst_addressing == 8)) {
        // Both operands are registers (direct or indirect)
        int reg_word = 0b100 | (src_reg << 6) | (dst_reg << 3);
        char *reg_word_binary = int_to_binary(reg_word, 15);
        strcpy(memory[IC].binary, reg_word_binary);
        free(reg_word_binary);
        memory[IC].address = IC;
        strcpy(memory[IC].label, "");
        IC++;
    } else {
        // Handle source operand
        if (src_addressing == 1) {
            // Immediate
            int immediate_word = ((immediate_value & 0x3FFF) << 2) | 0b100;
            char *immediate_word_binary = int_to_binary(immediate_word, 15);
            strcpy(memory[IC].binary, immediate_word_binary);
            free(immediate_word_binary);
            memory[IC].address = IC;
            strcpy(memory[IC].label, "");
            IC++;
        } else if (src_addressing == 2) {
            // Direct (label)
            strcpy(memory[IC].binary, "000000000000100");
            memory[IC].address = IC;
            strncpy(memory[IC].label, src_label, MAX_LABEL_LENGTH + 2);
            memory[IC].label[MAX_LABEL_LENGTH + 2] = '\0';
            IC++;
        } else if (src_addressing == 4 || src_addressing == 8) {
            // Register (indirect or direct)
            int reg_word = 0b100 | (src_reg << 6);
            char *reg_word_binary = int_to_binary(reg_word, 15);
            strcpy(memory[IC].binary, reg_word_binary);
            free(reg_word_binary);
            memory[IC].address = IC;
            strcpy(memory[IC].label, "");
            IC++;
        }
        
        // Handle destination operand
        if (dst_addressing != 0) {
            if (dst_addressing == 1) {
                // Immediate
                int immediate_word = ((immediate_value & 0x3FFF) << 2) | 0b100;
                char *immediate_word_binary = int_to_binary(immediate_word, 15);
                strcpy(memory[IC].binary, immediate_word_binary);
                free(immediate_word_binary);
                memory[IC].address = IC;
                strcpy(memory[IC].label, "");
                IC++;
            } else if (dst_addressing == 2) {
                // Direct (label)
                strcpy(memory[IC].binary, "000000000000100");
                memory[IC].address = IC;
                strncpy(memory[IC].label, dst_label, MAX_LABEL_LENGTH + 2);
                memory[IC].label[MAX_LABEL_LENGTH + 2] = '\0';
                IC++;
            } else if (dst_addressing == 4 || dst_addressing == 8) {
                // Register (indirect or direct)
                int reg_word = 0b100 | (dst_reg << 3);
                char *reg_word_binary = int_to_binary(reg_word, 15);
                strcpy(memory[IC].binary, reg_word_binary);
                free(reg_word_binary);
                memory[IC].address = IC;
                strcpy(memory[IC].label, "");
                IC++;
            }
        }
    }
}
void process_line(char* line) {
    char* label = NULL;
    char* op = NULL;
    char* operand1 = NULL;
    char* operand2 = NULL;
    char* token = strtok(line, " ");
    
    if (token == NULL) return;

    if (strchr(token, ':') != NULL) {
        label = token;
        label[strlen(label) - 1] = '\0';
        token = strtok(NULL, " ");
    }

    if (token == NULL) return;

    op = token;
    operand1 = strtok(NULL, ",");
    operand2 = strtok(NULL, "");

    if (operand1) operand1 = trim(operand1);
    if (operand2) operand2 = trim(operand2);

    assemble_instruction(label, op, operand1, operand2);
}

char* trim(char* str) {
    char* end;

    while (isspace((unsigned char)*str)) str++;

    if (*str == '\0') return str;

    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;

    end[1] = '\0';

    return str;
}

void print_memory() {
    for (int i = 100; i < IC; i++) {
        if (memory[i].label[0] != '\0') {
            // בדיקה אם זו תווית (לא מתחילה במרכאות) ולא אוגר
            if (memory[i].label[0] != '"' && strncmp(memory[i].label, "r", 1) != 0) {
                printf("%08d %s\n", i, memory[i].label);
            } else if (memory[i].label[0] == '"') {
                // זו תווית שמשמשת כאופרנד, מדפיסים אותה עם המרכאות
                printf("%08d %s\n", i, memory[i].label);
            }
        } else {
            // מדפיסים את ההוראה הבינארית
            printf("%08d %s\n", i, memory[i].binary);
        }
    }
}


int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <example.as>\n", argv[0]);
        return 1;
    }

    FILE* file = fopen(argv[1], "r");
    if (file == NULL) {
        perror("Error opening file");
        return 1;
    }

    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), file)) {
        char* trimmed_line = trim(line);
        if (trimmed_line[0] == '\0' || trimmed_line[0] == ';') {
            continue;
        }
        process_line(trimmed_line);
    }

    fclose(file);

    print_memory();

    return 0;
}
