#include "check.h"
#include "assembler.h"


/*
 * Initializes the assembler state.
 * 
 * This function allocates memory for the assembler state and initializes
 * all its components, including memory for instructions, extern labels,
 * label table, and entry labels. It also sets up initial counters.
 *
 * Returns a pointer to the initialized AssemblerState, or NULL if allocation fails.
 */

#define WORD_SIZE 15
#define OPCODE_SHIFT 11
#define SRC_ADDRESSING_SHIFT 7
#define DST_ADDRESSING_SHIFT 3
#define ARE_BITS 0x4
#define REG_WORD_MASK 0x4
#define SRC_REG_SHIFT 6
#define DST_REG_SHIFT 3
#define EXTERNAL_LABEL_ARE 0x1 
#define RELOCATABLE_LABEL_ARE 0x2
AssemblerState *init_assembler_state()
{
 AssemblerState *state;
    
    state = malloc(sizeof(AssemblerState));
        if (!state)
    {
        perror("Failed to allocate memory for AssemblerState");
        return NULL;
    }

    /* Allocate memory for instructions*/
    state->memory = malloc(INITIAL_MEMORY_SIZE * sizeof(Instruction));
    if (!state->memory)
    {
        perror("Failed to allocate memory for instructions");
        free(state);
        return NULL;
    }
    state->memory_size = 0;
    state->memory_capacity = INITIAL_MEMORY_SIZE;

    /* Allocate memory for extern labels*/
    state->extern_table = malloc(INITIAL_TABLE_SIZE * sizeof(ExternLabel));
    if (!state->extern_table)
    {
        perror("Failed to allocate memory for extern table");
        free(state->memory);
        free(state);
        return NULL;
    }
    state->extern_count = 0;
    state->extern_capacity = INITIAL_TABLE_SIZE;

    /* Allocate memory for label table*/
    state->label_table = malloc(INITIAL_TABLE_SIZE * sizeof(Label));
    if (!state->label_table)
    {
        perror("Failed to allocate memory for label table");
        free(state->extern_table);
        free(state->memory);
        free(state);
        return NULL;
    }
    state->label_count = 0;
    state->label_capacity = INITIAL_TABLE_SIZE;

    /* Allocate memory for entry labels*/
    state->entry_table = malloc(INITIAL_TABLE_SIZE * sizeof(EntryLabel));
    if (!state->entry_table)
    {
        perror("Failed to allocate memory for entry table");
        free(state->label_table);
        free(state->extern_table);
        free(state->memory);
        free(state);
        return NULL;
    }
    state->entry_count = 0;
    state->entry_capacity = INITIAL_TABLE_SIZE;

    /* Initialize counters*/
    state->IC = 100;
    state->DC = 0;

    return state;
}
/* This function processes a data directive in an assembler, parsing comma-separated integer values.
   It adds a label if provided, converts each value to binary, and stores them in the assembler's memory. */
void handle_data_directive(AssemblerState *state, const char *label, const char *params, int validLabel)
{
    char *token;
    char *params_copy;
    char *saveptr;
    int value;
    char *word_binary;

    params_copy = my_strdup(params);  
    /* Add label to symbol table if it's valid and not empty */

    if (label && label[0] != '\0' && validLabel == 0)
    {
        add_label(state, label, state->IC + state->DC);
    }
        /* Tokenize the params string, splitting by commas */
	token = strtok_r(params_copy, ",", &saveptr);
    while (token != NULL)
    {
        token = trim(token);  /* Remove leading/trailing whitespace */
        value = atoi(token);  /* Convert token to integer */
        value &= 0x7FFF;      /* Mask to ensure 15-bit value */
        
        /* Check if memory needs to be expanded */

        if (state->memory_size >= state->memory_capacity)
        {
            state->memory_capacity *= 2;
            state->memory = realloc(state->memory, state->memory_capacity * sizeof(Instruction));
            if (!state->memory)
            {
                perror("Failed to reallocate memory");
                free(params_copy);   
                return;
            }
        }
        /* Convert integer to 15-bit binary string */

        word_binary = int_to_binary(value, 15);  
        if (word_binary != NULL)   
        {            /* Store binary string in memory */

            strcpy(state->memory[state->IC + state->DC].binary, word_binary);
            state->memory[state->IC + state->DC].address = state->IC + state->DC;
            free(word_binary);
            state->DC++;
            state->memory_size++;
        }
        else
        {
            perror("Failed to allocate memory for word_binary");
        }

        token = strtok_r(NULL, ",", &saveptr);
    }

    free(params_copy);
}



/* Function to handle .string directive */
void handle_string_directive(AssemblerState *state, const char *label, const char *params, int validLabel)
{
    char *start;
    char *end;
    char *word_binary;
    char *null_binary;
    /* Add label to symbol table if it's valid and not empty */

    if (label && label[0] != '\0' && validLabel == 0)
    {
        add_label(state, label, state->IC + state->DC);
    }

    start = strchr(params, '"');
    end = strrchr(params, '"');

    start++;
    *end = '\0';
    /* Process each character in the string */

    while (*start != '\0')
    {
        if (state->memory_size >= state->memory_capacity)
        {
            state->memory_capacity *= 2;
            state->memory = realloc(state->memory, state->memory_capacity * sizeof(Instruction));
            if (!state->memory)
            {
                perror("Failed to reallocate memory");
                return;
            }
        }
        /* Convert character to binary and store in memory */

        word_binary = int_to_binary((int)*start, WORD_SIZE);
        strcpy(state->memory[state->IC + state->DC].binary, word_binary);
        state->memory[state->IC + state->DC].address = state->IC + state->DC;
        free(word_binary);
        state->DC++;
        state->memory_size++;
        start++;
    }

    /* Add null terminator */
    if (state->memory_size >= state->memory_capacity)
    {
        state->memory_capacity *= 2;
        state->memory = realloc(state->memory, state->memory_capacity * sizeof(Instruction));
        if (!state->memory)
        {
            perror("Failed to reallocate memory");
            return;
        }
    }

    null_binary = int_to_binary(0, WORD_SIZE);
    strcpy(state->memory[state->IC + state->DC].binary, null_binary);
    state->memory[state->IC + state->DC].address = state->IC + state->DC;
    free(null_binary);
    state->DC++;
    state->memory_size++;
}


/* Function to handle .entry directive */
void handle_entry_directive(AssemblerState *state, const char *label)
{
    int i;

    for (i = 0; i < state->extern_count; i++)
    {
        if (strcmp(state->extern_table[i].name, label) == 0)
        {
            fprintf(stderr, "Error: label '%s' has already been declared as extern\n", label);
            return;
        }
    }

    for (i = 0; i < state->entry_count; i++)
    {
        if (strcmp(state->entry_table[i].name, label) == 0)
        {
            fprintf(stderr, "Warning: label '%s' has already been declared as an entry\n", label);
            return;
        }
    }

    if (state->entry_count == state->entry_capacity)
    {
        state->entry_capacity *= 2;
        state->entry_table = realloc(state->entry_table, state->entry_capacity * sizeof(EntryLabel));
        if (!state->entry_table)
        {
            perror("Failed to reallocate entry table");
            return;
        }
    }
    
    state->entry_table[state->entry_count].name[MAX_LABEL_LENGTH] = '\0';
    strncpy(state->entry_table[state->entry_count].name, label, MAX_LABEL_LENGTH);
    state->entry_table[state->entry_count].address = -1; /* Initialize to -1 and update later */
    state->entry_count++;
}


/* Function to handle .extern directive */
void handle_extern_directive(AssemblerState *state, const char *label)
{
    int i;
    /* Check if the label has already been declared as extern */

    for (i = 0; i < state->extern_count; i++)
    {
        if (strcmp(state->extern_table[i].name, label) == 0)
        {
            fprintf(stderr, "Error: label '%s' has already been declared as extern\n", label);
            return;
        }
    }
    /* Check if the label has already been declared as entry */

    for (i = 0; i < state->extern_count; i++)
    {
        if (strcmp(state->extern_table[i].name, label) == 0)
        {
            fprintf(stderr, "Warning: label '%s' has already been declared as external\n", label);
            return;
        }
    }
    /* Expand entry table if it's full */

    if (state->extern_count == state->extern_capacity)
    {
        state->extern_capacity *= 2;
        state->extern_table = realloc(state->extern_table, state->extern_capacity * sizeof(ExternLabel));
        if (!state->extern_table)
        {
            perror("Failed to reallocate extern table");
            return;
        }
    }

    strncpy(state->extern_table[state->extern_count].name, label, MAX_LABEL_LENGTH);
    state->extern_table[state->extern_count].name[MAX_LABEL_LENGTH] = '\0';
    state->extern_table[state->extern_count].address = -1; /* Initialize to -1 and update later */
    state->extern_count++;
}

int assemble_instruction(AssemblerState *state, const char *label, const char *op, const char *operand1, const char *operand2, int validLabel)
{ 
    /* Variable declarations */
    int error ;
    int opcode ;
    int src_addressing ;
    int dst_addressing ;
    int src_reg ;
    int dst_reg ;
    int immediate_value ;
    char src_label[MAX_LABEL_LENGTH + 3] ;
    char dst_label[MAX_LABEL_LENGTH + 3] ;
    int first_word;
    char *first_word_binary;
    int reg_word;
    char *reg_word_binary;
    char *encoded_immediate;
    int is_extern;
    int i;

    Instruction *new_memory; 
    immediate_value = 0;
     error = 0;
     opcode = get_opcode(op);
    src_addressing =0;
     dst_addressing =0;
    src_reg =-1;
      dst_reg =-1;
     memset(src_label, 0, sizeof(src_label));
memset(dst_label, 0, sizeof(dst_label));


    /* Add label to symbol table if it's valid and not empty */
    if (label && label[0] != '\0' && validLabel == 0)
    {
        add_label(state, label, state->IC);
    }

    /* Handle single operand instructions */
    if (is_single_operand_instruction(op))
    {
        if (operand2 != NULL)
        {
            return 0;
        }
        operand2 = operand1;
        operand1 = NULL;
    }

    /* Process source operand */
    if (operand1)
    {
        src_addressing = get_addressing_mode(operand1);
        if (src_addressing == 1)
        {
            immediate_value = atoi(operand1 + 1);
        }
        else if (src_addressing == 4 || src_addressing == 8)
        {
            src_reg = get_register_number(src_addressing == 4 ? operand1 + 1 : operand1);
        }
        else
        {
            my_snprintf(src_label, sizeof(src_label), "\"%s\"", operand1);
        }
    }

    /* Process destination operand */
    if (operand2)
    {
        dst_addressing = get_addressing_mode(operand2);
        if (dst_addressing == 1)
        {
            immediate_value = atoi(operand2 + 1);
        }
        else if (dst_addressing == 4 || dst_addressing == 8)
        {
            dst_reg = get_register_number(dst_addressing == 4 ? operand2 + 1 : operand2);
        }
        else
        {
            my_snprintf(dst_label, sizeof(dst_label), "\"%s\"", operand2);
        }
    }

    /* Construct first word of the instruction */
    first_word = (opcode << OPCODE_SHIFT) | (src_addressing << SRC_ADDRESSING_SHIFT) | (dst_addressing << DST_ADDRESSING_SHIFT) | ARE_BITS;
    first_word_binary = int_to_binary(first_word, WORD_SIZE);

    /* Expand memory if needed */
    if (state->memory_size >= state->memory_capacity)
    {
        state->memory_capacity *= 2;
         new_memory = realloc(state->memory, state->memory_capacity * sizeof(Instruction));
        if (!new_memory)
        {
            perror("Failed to reallocate memory");
            free(state->memory);
            return 1;
        }
        state->memory = new_memory;
    }

    /* Store first word in memory */
    strcpy(state->memory[state->IC].binary, first_word_binary);
    free(first_word_binary);
    state->memory[state->IC].address = state->IC;
    strcpy(state->memory[state->IC].label, "");
    state->IC++;
    state->memory_size++;

    /* Handle register-to-register instruction */
    if ((src_addressing == 4 || src_addressing == 8) && (dst_addressing == 4 || dst_addressing == 8))
    {
        reg_word = REG_WORD_MASK | (src_reg << SRC_REG_SHIFT) | (dst_reg << DST_REG_SHIFT);
        reg_word_binary = int_to_binary(reg_word, WORD_SIZE);

        if (state->memory_size >= state->memory_capacity)
        {
            state->memory_capacity *= 2;
            state->memory = realloc(state->memory, state->memory_capacity * sizeof(Instruction));
            if (!state->memory)
            {
                perror("Failed to reallocate memory");
                return 1;
            }
        }

        strcpy(state->memory[state->IC].binary, reg_word_binary);
        free(reg_word_binary);
        state->memory[state->IC].address = state->IC;
        strcpy(state->memory[state->IC].label, "");
        state->IC++;
        state->memory_size++;
    }
    else
    {
        /* Handle source operand */
        if (src_addressing == 1)
        {
            encoded_immediate = encode_immediate_operand(operand1);
            if (encoded_immediate != NULL)
            {
                if (state->memory_size >= state->memory_capacity)
                {
                    state->memory_capacity *= 2;
                    state->memory = realloc(state->memory, state->memory_capacity * sizeof(Instruction));
                    if (!state->memory)
                    {
                        perror("Failed to reallocate memory");
                        return 1;
                    }
                }

                strcpy(state->memory[state->IC].binary, encoded_immediate);
                free(encoded_immediate);
                state->memory[state->IC].address = state->IC;
                strcpy(state->memory[state->IC].label, "");
                state->IC++;
                state->memory_size++;
            }
            else
            {
                fprintf(stderr, "Error: Invalid immediate operand %s\n", operand1);
                error = 1;
            }
        }
        else if (src_addressing == 2)
        {
            if (state->memory_size >= state->memory_capacity)
            {
                state->memory_capacity *= 2;
                state->memory = realloc(state->memory, state->memory_capacity * sizeof(Instruction));
                if (!state->memory)
                {
                    perror("Failed to reallocate memory");
                    return 1;
                }
            }

            /* Check if the operand is an external label */
            is_extern = 0;
            for (i = 0; i < state->extern_count; i++)
            {
                if (strcmp(state->extern_table[i].name, operand1) == 0)
                {
                    is_extern = 1;
                    break;
                }
            }

            if (is_extern)
            {
                strcpy(state->memory[state->IC].binary, "000000000000001"); /* Use 001 for external labels */
            }
            else
            {
                strcpy(state->memory[state->IC].binary, "000000000000010"); /* Use 010 for non-external labels */
            }
            state->memory[state->IC].address = state->IC;
            my_snprintf(state->memory[state->IC].label, sizeof(state->memory[state->IC].label), "%s", operand1);
            state->IC++;
            state->memory_size++;
        }
        else if (src_addressing == 4 || src_addressing == 8)
        {
            reg_word = REG_WORD_MASK | (src_reg << SRC_REG_SHIFT);
            reg_word_binary = int_to_binary(reg_word, WORD_SIZE);

            if (state->memory_size >= state->memory_capacity)
            {
                state->memory_capacity *= 2;
                state->memory = realloc(state->memory, state->memory_capacity * sizeof(Instruction));
                if (!state->memory)
                {
                    perror("Failed to reallocate memory");
                    return 1;
                }
            }

            strcpy(state->memory[state->IC].binary, reg_word_binary);
            free(reg_word_binary);
            state->memory[state->IC].address = state->IC;
            strcpy(state->memory[state->IC].label, "");
            state->IC++;
            state->memory_size++;
        }

        /* Handle destination operand */
        if (dst_addressing != 0)
        {
            if (dst_addressing == 1)
            {
                encoded_immediate = encode_immediate_operand(operand2);
                if (encoded_immediate != NULL)
                {
                    if (state->memory_size >= state->memory_capacity)
                    {
                        state->memory_capacity *= 2;
                        state->memory = realloc(state->memory, state->memory_capacity * sizeof(Instruction));
                        if (!state->memory)
                        {
                            perror("Failed to reallocate memory");
                            return 1;
                        }
                    }

                    strcpy(state->memory[state->IC].binary, encoded_immediate);
                    free(encoded_immediate);
                    state->memory[state->IC].address = state->IC;
                    strcpy(state->memory[state->IC].label, "");
                    state->IC++;
                    state->memory_size++;
                }
                else
                {
                    fprintf(stderr, "Error: Invalid immediate operand %s\n", operand2);
                    error = 1;
                }
            }
            else if (dst_addressing == 2)
            {
                if (state->memory_size >= state->memory_capacity)
                {
                    state->memory_capacity *= 2;
                    state->memory = realloc(state->memory, state->memory_capacity * sizeof(Instruction));
                    if (!state->memory)
                    {
                        perror("Failed to reallocate memory");
                        return 1;
                    }
                }

                /* Check if the operand is an external label */
                is_extern = 0;
                for (i = 0; i < state->extern_count; i++)
                {
                    if (strcmp(state->extern_table[i].name, operand2) == 0)
                    {
                        is_extern = 1;
                        break;
                    }
                }

                if (is_extern)
                {
                    strcpy(state->memory[state->IC].binary, "000000000000001"); /* Use 001 for external labels */
                }
                else
                {
                    strcpy(state->memory[state->IC].binary, "000000000000010"); /* Use 010 for non-external labels */
                }
                state->memory[state->IC].address = state->IC;
                my_snprintf(state->memory[state->IC].label, sizeof(state->memory[state->IC].label), "%s", operand2);
                state->IC++;
                state->memory_size++;
            }
            else if (dst_addressing == 4 || dst_addressing == 8)
            {
                reg_word = REG_WORD_MASK | (dst_reg << DST_REG_SHIFT);
                reg_word_binary = int_to_binary(reg_word, WORD_SIZE);

                if (state->memory_size >= state->memory_capacity)
                {
                    state->memory_capacity *= 2;
                    state->memory = realloc(state->memory, state->memory_capacity * sizeof(Instruction));
                    if (!state->memory)
                    {
                        perror("Failed to reallocate memory");
                        return 1;
                    }
                }

                strcpy(state->memory[state->IC].binary, reg_word_binary);
                free(reg_word_binary);
                state->memory[state->IC].address = state->IC;
                strcpy(state->memory[state->IC].label, "");
                state->IC++;
                state->memory_size++;
            }
        }
    }
    if(error == 1)
    {
        return 1;
    }
    return 0;
}
            
/* Function to process a single line of assembly code */
int process_line(AssemblerState *state, char *line, Macro *macros, int *macroCount)
{
    int error = 0;
    char *label = NULL;
    char *op = NULL;
    char *operand1 = NULL;
    char *operand2 = NULL;
    int validLabel = 0;
    char *token;
    char *afterData;
    char *afterString;
    char *afterEntry;
    char *afterExtern;
    int operand_count;
    char *operands[2];
    int i;
    int result;

    /* Check line length */
    if (!check_line_length(line))
    {
        fprintf(stderr, "Error: line length exceeds the limit\n");
        error = 1;
    }

    token = strtok(line, " \t");
    if (token == NULL)
    {
        return 0;
    }

    /* Check for label */
    if (strchr(token, ':') != NULL)
    {
        label = token;
        label[strlen(label) - 1] = '\0'; /* Remove colon */

        if (!isValidLabel(label, macros, macroCount))
        {
            fprintf(stderr, "Error: Invalid label\n");
            error = 1;
            validLabel = 1;
        }
        if (is_duplicate_label(label, state))
        {
            fprintf(stderr, "Error: duplicate label definition\n");
            error = 1;
        }
        /* Continue after label */
        token = strtok(NULL, " \t");
        if (token == NULL)
        {
            fprintf(stderr, "Error: Label '%s' is followed by an empty line\n", label);
            error = 1;
        }
    }

    op = token;

    /* Handle special instructions */
    if (op[0] == '.')
    {
        if (strncmp(op, ".data", 5) == 0 && strlen(op) == 5)
        {
            afterData = strtok(NULL, "");

            /* Check data integrity */
            if (data_intergity_check(afterData) == 0)
            {
                afterData = trim(afterData);
                handle_data_directive(state, label ? label : "", afterData, validLabel);
            }
            else
            {
                fprintf(stderr, "Invalid .data directive\n");
            }
        }
        else if (strncmp(op, ".string", 7) == 0 && strlen(op) == 7)
        {
            afterString = strtok(NULL, "");

            /* Check string integrity */
            if (string_intergity_check(afterString) == 0)
            {
                afterString = trim(afterString);
                handle_string_directive(state, label ? label : "", afterString, validLabel);
            }
            else
            {
                fprintf(stderr, "Invalid .string directive\n");
            }
        }
        else if (strncmp(op, ".entry", 6) == 0 && strlen(op) == 6)
        {
            afterEntry = strtok(NULL, "");

            /* Check entry integrity */
            if (entry_intergity_check(afterEntry, state, macros, macroCount) == 0)
            {
                afterEntry = trim(afterEntry);
                handle_entry_directive(state, afterEntry);
            }
            else
            {
                fprintf(stderr, "Invalid .entry directive\n");
            }
        }
        else if (strncmp(op, ".extern", 7) == 0 && strlen(op) == 7)
        {
            afterExtern = strtok(NULL, "");

            /* Check extern integrity */
            if (extern_intergity_check(afterExtern, state) == 0)
            {
                afterExtern = trim(afterExtern);
                handle_extern_directive(state, afterExtern);
            }
            else
            {
                fprintf(stderr, "Invalid .extern directive\n");
            }
        }
        else
        {
            fprintf(stderr, "Error: Invalid directive\n");
            error = 1;
        }
    }
    else
    {
        /* Check instruction validity */
        if (!is_valid_instruction(op))
        {
            fprintf(stderr, "Error: Invalid Instruction\n");
            error = 1;
        }
        /* Handle regular instructions */
        operand1 = strtok(NULL, ",");
        operand2 = strtok(NULL, "");

        if (operand1)
            operand1 = trim(operand1);
        if (operand2)
            operand2 = trim(operand2);

        operand_count = 0;
        if (operand1)
            operand_count++;
        if (operand2)
            operand_count++;

        /* Check operand count */
        if (!check_operand_count(op, operand_count))
        {
            fprintf(stderr, "Error: Incorrect number of operands for instruction\n");
            error = 1;
        }

        /* Check operand validity */
        operands[0] = operand1;
        operands[1] = operand2;
        for (i = 0; i < operand_count; i++)
        {
            if (!is_valid_operand(operands[i], macros, macroCount))
            {
                fprintf(stderr, "Error: Invalid operand\n");
                error = 1;
            }

            if (!is_valid_addressing_mode(op, operands[i], i == 0))
            {
                fprintf(stderr, "Error: Invalid addressing method for operand %s\n", i == 0 ? "Source" : "Destination");
                error = 1;
            }

            if (operands[i][0] == '#' && !is_valid_immediate(operands[i]))
            {
                fprintf(stderr, "Error: Invalid immediate value\n");
                error = 1;
            }
        }

        /* Assemble instruction */
        result = assemble_instruction(state, label, op, operand1, operand2, validLabel);
        if (result == 1)
        {
            error = 1;
        }
    }
    if (error == 1)
        return 1;
    else
        return 0;
}



/* Function to perform the first pass of the assembler */
void first_pass(AssemblerState *state, const char *filename, Macro *macros, int *macroCount, int *error)
{
    FILE *file;
    char line[MAX_LINE_LENGTH];
    char *newline;
    char *trimmed_line;
    int result;

 
    file = fopen(filename, "r");
    if (file == NULL)
    {
        perror("Error opening file");
        return;
    }

    while (fgets(line, sizeof(line), file))
    {
        newline = strchr(line, '\n');
        if (newline)
            *newline = '\0'; /* Remove newline character */

        trimmed_line = trim(line);

        if (trimmed_line[0] == '\0' || trimmed_line[0] == ';')
        {
            continue;
        }
        result = process_line(state, trimmed_line, macros, macroCount);
        if (result == 1)
 {
            *error = 1;  /* Update the error value through the pointer */
        }

    }

    fclose(file);
}
