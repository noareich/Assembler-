/*
 * קובץ: assembler.c
 * תיאור: מימוש של אסמבלר לשפת אסמבלי דמיונית עם הקצאה דינמית ללא משתנים גלובליים
 */
#include "check.h"
#include "assembler.h"


/*פונקציות עזר */
// הצהרות על פונקציות המעבר השני
// בקובץ first_pass.c

char *trim(char *str) {
    char *end;
    while(isspace((unsigned char)*str)) str++;
    if(*str == 0) return str;
    end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
    return str;
}

int get_opcode(const char *operation) {
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
    return -1; /* פעולה לא חוקית */
}

int get_addressing_mode(const char *operand) {
    if (operand[0] == '#') return 1; /* מיידי */
    if (operand[0] == 'r') return 8; /* רגיסטר ישיר */
    if (operand[0] == '*') return 4; /* רגיסטר עקיף */
    return 2; /* ישיר (תווית) */
}

int get_register_number(const char *operand) {
    if (operand[0] == 'r' && isdigit(operand[1])) {
        return operand[1] - '0';
    }
    return -1; /* לא רגיסטר */
}

char* int_to_binary(int value, int bits) {
    char* binary = (char*)malloc(bits + 1);
    if (binary == NULL) {
        fprintf(stderr, "שגיאה: הקצאת זיכרון נכשלה.\n");
        exit(1);
    }
    
    binary[bits] = '\0';
    for (int i = bits - 1; i >= 0; i--) {
        binary[i] = (value & 1) ? '1' : '0';
        value >>= 1;
    }
    
    return binary;
}

/* פונקציות לניהול מצב האסמבלר */

AssemblerState* init_assembler_state() {
   /* printf("Size of AssemblerState: %zu bytes\n", sizeof(AssemblerState));*/
    AssemblerState* state = malloc(sizeof(AssemblerState));
    if (!state) {
        exit(1);
    }

    state->memory = malloc(INITIAL_MEMORY_SIZE * sizeof(Instruction));
    if (!state->memory) {
        perror("Failed to allocate memory for instructions");
        free(state);
        exit(1);
    }
    state->memory_size = 0;
    state->memory_capacity = INITIAL_MEMORY_SIZE;

     state->extern_table = malloc(INITIAL_TABLE_SIZE * sizeof(ExternLabel));
    state->extern_count = 0;
    state->extern_capacity = INITIAL_TABLE_SIZE;
    state->label_table = malloc(INITIAL_TABLE_SIZE * sizeof(Label));
    state->label_count = 0;
    state->label_capacity = INITIAL_TABLE_SIZE;
    state->entry_table = malloc(INITIAL_TABLE_SIZE * sizeof(EntryLabel));
    state->entry_count = 0;
    state->entry_capacity = INITIAL_TABLE_SIZE;
    state->extern_table = malloc(INITIAL_TABLE_SIZE * sizeof(char*));
    state->extern_count = 0;
    state->extern_capacity = INITIAL_TABLE_SIZE;
    state->IC = 100;
    state->DC = 0;

    state->memory = malloc(INITIAL_MEMORY_SIZE * sizeof(Instruction));
if (!state->memory) {
    perror("Failed to allocate memory for instructions");
    free(state);
    exit(1);
}
    return state;
}

void free_assembler_state(AssemblerState* state) {
    free(state->memory);
    free(state->label_table);
    free(state->entry_table);
    free(state->extern_table);  /* Changed: no need to free individual strings */
    free(state);
}

void add_label(AssemblerState* state, const char* name, int address) {
    if (state->label_count == state->label_capacity) {
        state->label_capacity *= 2;
        state->label_table = realloc(state->label_table, state->label_capacity * sizeof(Label));
        if (!state->label_table) {
            perror("Failed to reallocate label table");
            exit(1);
        }
    }

    strncpy(state->label_table[state->label_count].name, name, MAX_LABEL_LENGTH);
    state->label_table[state->label_count].name[MAX_LABEL_LENGTH] = '\0';
    state->label_table[state->label_count].address = address;
    state->label_table[state->label_count].is_entry = 0;
    state->label_table[state->label_count].is_extern = 0;
    state->label_count++;
}

int get_label_address(AssemblerState* state, const char* name) {
    for (int i = 0; i < state->label_count; i++) {
        if (strcmp(state->label_table[i].name, name) == 0) {
            return state->label_table[i].address;
        }
    }
    for (int i = 0; i < state->extern_count; i++) {
        if (strcmp(state->extern_table[i].name, name) == 0) {
            return 0;  // תוויות חיצוניות מקבלות כתובת 0
        }
    }
    return -1; /* התווית לא נמצאה */
}

void handle_data_directive(AssemblerState* state, const char* label, const char* params) {
    char *token;
    char *params_copy = strdup(params);
    char *saveptr;

    if (label && label[0] != '\0') {
        add_label(state, label, state->IC + state->DC);
    }

    token = strtok_r(params_copy, ",", &saveptr);
    while (token != NULL) {
        token = trim(token);
        int value = atoi(token);
        value &= 0x7FFF; /* הבטחת ערך 15 ביט */
        
        if (state->memory_size >= state->memory_capacity) {
            state->memory_capacity *= 2;
            state->memory = realloc(state->memory, state->memory_capacity * sizeof(Instruction));
            if (!state->memory) {
                perror("Failed to reallocate memory");
                exit(1);
            }
        }

        char *word_binary = int_to_binary(value, 15);
        strcpy(state->memory[state->IC + state->DC].binary, word_binary);
        state->memory[state->IC + state->DC].address = state->IC + state->DC;
        free(word_binary);
        state->DC++;
        state->memory_size++;

        token = strtok_r(NULL, ",", &saveptr);
    }

    free(params_copy);
}

void handle_string_directive(AssemblerState* state, const char* label, const char* params) {
    if (label && label[0] != '\0') {
        add_label(state, label, state->IC + state->DC);
    }

    char *start = strchr(params, '"');
    char *end = strrchr(params, '"');
    if (start == NULL || end == NULL || start == end) {
        fprintf(stderr, "מחרוזת לא חוקית\n");
        return;
    }

    start++;
    *end = '\0';

    while (*start != '\0') {
        if (state->memory_size >= state->memory_capacity) {
            state->memory_capacity *= 2;
            state->memory = realloc(state->memory, state->memory_capacity * sizeof(Instruction));
            if (!state->memory) {
                perror("Failed to reallocate memory");
                exit(1);
            }
        }

        char *word_binary = int_to_binary((int)*start, 15);
        strcpy(state->memory[state->IC + state->DC].binary, word_binary);
        state->memory[state->IC + state->DC].address = state->IC + state->DC;
        free(word_binary);
        state->DC++;
        state->memory_size++;
        start++;
    }

    /* הוספת תו סיום */
    if (state->memory_size >= state->memory_capacity) {
        state->memory_capacity *= 2;
        state->memory = realloc(state->memory, state->memory_capacity * sizeof(Instruction));
        if (!state->memory) {
            perror("Failed to reallocate memory");
            exit(1);
        }
    }

    char *null_binary = int_to_binary(0, 15);
    strcpy(state->memory[state->IC + state->DC].binary, null_binary);
    state->memory[state->IC + state->DC].address = state->IC + state->DC;
    free(null_binary);
    state->DC++;
    state->memory_size++;
}

void handle_entry_directive(AssemblerState* state, const char* label) {
    for (int i = 0; i < state->extern_count; i++) {
        if (strcmp(state->extern_table[i].name, label) == 0) {  // שינוי כאן
            fprintf(stderr, "שגיאה: התווית '%s' כבר הוכרזה כ-extern\n", label);
            return;
        }
    }

    for (int i = 0; i < state->entry_count; i++) {
        if (strcmp(state->entry_table[i].name, label) == 0) {
            fprintf(stderr, "אזהרה: התווית '%s' כבר הוכרזה כ-entry\n", label);
            return;
        }
    }

    if (state->entry_count == state->entry_capacity) {
        state->entry_capacity *= 2;
        state->entry_table = realloc(state->entry_table, state->entry_capacity * sizeof(EntryLabel));
        if (!state->entry_table) {
            perror("Failed to reallocate entry table");
            exit(1);
        }
    }

    strncpy(state->entry_table[state->entry_count].name, label, MAX_LABEL_LENGTH);
    state->entry_table[state->entry_count].name[MAX_LABEL_LENGTH] = '\0';
    state->entry_table[state->entry_count].address = -1; /* נאתחל ל-1- ונעדכן בשלב מאוחר יותר */
    state->entry_count++;
}

void handle_extern_directive(AssemblerState* state, const char* label) {
    for (int i = 0; i < state->entry_count; i++) {
        if (strcmp(state->entry_table[i].name, label) == 0) {
            fprintf(stderr, "שגיאה: התווית '%s' כבר הוכרזה כ-entry\n", label);
            return;
        }
    }

    for (int i = 0; i < state->extern_count; i++) {
        if (strcmp(state->extern_table[i].name, label) == 0) {  // שינוי כאן
            fprintf(stderr, "אזהרה: התווית '%s' כבר הוכרזה כ-extern\n", label);
            return;
        }
    }

   if (state->extern_count == state->extern_capacity) {
        state->extern_capacity *= 2;
        state->extern_table = realloc(state->extern_table, state->extern_capacity * sizeof(ExternLabel));
        if (!state->extern_table) {
            perror("Failed to reallocate extern table");
            exit(1);
        }
    }

    strncpy(state->extern_table[state->extern_count].name, label, MAX_LABEL_LENGTH);
    state->extern_table[state->extern_count].name[MAX_LABEL_LENGTH] = '\0';
state->extern_table[state->extern_count].addresses = NULL;  /* Initialize addresses to NULL */    state->extern_count++;
}
int is_single_operand_instruction(const char *op) {
    const char *single_operand_instructions[] = {
        "clr", "not", "inc", "dec", "jmp", "bne", "red", "prn", "jsr"
    };
    int num_instructions = sizeof(single_operand_instructions) / sizeof(single_operand_instructions[0]);

    for (int i = 0; i < num_instructions; i++) {
        if (strcmp(op, single_operand_instructions[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

char* encode_immediate_operand(const char* operand) {
    if (operand == NULL || operand[0] != '#') {
        return NULL;  /* לא אופרנד מיידי */
    }

    const char* number_str = operand + 1;
    
    char* endptr;
    long number = strtol(number_str, &endptr, 10);
    
    if (endptr == number_str || *endptr != '\0') {
        return NULL;  /* מספר לא חוקי */
    }

    if (number < -2048 || number > 2047) {
        return NULL;  /* מספר מחוץ לטווח */
    }

    char* result = (char*)malloc(16 * sizeof(char));
    if (result == NULL) {
        return NULL;  /* כשלון בהקצאת זיכרון */
    }

    unsigned short unsigned_number = (unsigned short)(number & 0xFFF);
    for (int i = 11; i >= 0; i--) {
        result[11 - i] = (unsigned_number & (1 << i)) ? '1' : '0';
    }

    strcat(result, "100");
    result[15] = '\0';

    return result;
}

void assemble_instruction(AssemblerState* state, const char* label, const char* op, const char* operand1, const char* operand2) {
    int opcode = get_opcode(op);
    int src_addressing = 0, dst_addressing = 0;
    int src_reg = -1, dst_reg = -1;
    int immediate_value = 0;
    char src_label[MAX_LABEL_LENGTH + 3] = "";
    char dst_label[MAX_LABEL_LENGTH + 3] = "";

    if (label && label[0] != '\0' && strncmp(label, "r", 1) != 0) {
        add_label(state, label, state->IC);
    }

    if (is_single_operand_instruction(op)) {
        if (operand2 != NULL) {
            return;
        }
        operand2 = operand1;
        operand1 = NULL;
    }

    if (operand1) {
        src_addressing = get_addressing_mode(operand1);
        if (src_addressing == 1) {
            immediate_value = atoi(operand1 + 1);
        } else if (src_addressing == 4 || src_addressing == 8) {
            src_reg = get_register_number(src_addressing == 4 ? operand1 + 1 : operand1);
        } else {
            snprintf(src_label, sizeof(src_label), "\"%s\"", operand1);
        }
    }

    if (operand2) {
        dst_addressing = get_addressing_mode(operand2);
        if (dst_addressing == 1) {
            immediate_value = atoi(operand2 + 1);
        } else if (dst_addressing == 4 || dst_addressing == 8) {
            dst_reg = get_register_number(dst_addressing == 4 ? operand2 + 1 : operand2);
        } else {
            snprintf(dst_label, sizeof(dst_label), "\"%s\"", operand2);
        }
    }
    
    int first_word = (opcode << 11) | (src_addressing << 7) | (dst_addressing << 3) | 0b100;
    char *first_word_binary = int_to_binary(first_word, 15);

    if (state->memory_size >= state->memory_capacity) {
        state->memory_capacity *= 2;
        Instruction* new_memory = realloc(state->memory, state->memory_capacity * sizeof(Instruction));
        if (!new_memory) {
            perror("Failed to reallocate memory");
            free(state->memory);
            exit(1);
        }
        state->memory = new_memory;
    }

    strcpy(state->memory[state->IC].binary, first_word_binary);
    free(first_word_binary);
    state->memory[state->IC].address = state->IC;
    strcpy(state->memory[state->IC].label, "");
    state->IC++;
    state->memory_size++;
    
    if ((src_addressing == 4 || src_addressing == 8) && (dst_addressing == 4 || dst_addressing == 8)) {
        int reg_word = 0b100 | (src_reg << 6) | (dst_reg << 3);
        char *reg_word_binary = int_to_binary(reg_word, 15);

        if (state->memory_size >= state->memory_capacity) {
            state->memory_capacity *= 2;
            state->memory = realloc(state->memory, state->memory_capacity * sizeof(Instruction));
            if (!state->memory) {
                perror("Failed to reallocate memory");
                exit(1);
            }
        }

        strcpy(state->memory[state->IC].binary, reg_word_binary);
        free(reg_word_binary);
        state->memory[state->IC].address = state->IC;
        strcpy(state->memory[state->IC].label, "");
        state->IC++;
        state->memory_size++;
    } else {
        if (src_addressing == 1) {
            char* encoded_immediate = encode_immediate_operand(operand1);
            if (encoded_immediate != NULL) {
                if (state->memory_size >= state->memory_capacity) {
                    state->memory_capacity *= 2;
                    state->memory = realloc(state->memory, state->memory_capacity * sizeof(Instruction));
                    if (!state->memory) {
                        perror("Failed to reallocate memory");
                        exit(1);
                    }
                }

                strcpy(state->memory[state->IC].binary, encoded_immediate);
                free(encoded_immediate);
                state->memory[state->IC].address = state->IC;
                strcpy(state->memory[state->IC].label, "");
                state->IC++;
                state->memory_size++;
            } else {
                fprintf(stderr, "שגיאה: אופרנד מיידי לא חוקי %s\n", operand1);
            }
        } else if (src_addressing == 2) {
            if (state->memory_size >= state->memory_capacity) {
                state->memory_capacity *= 2;
                state->memory = realloc(state->memory, state->memory_capacity * sizeof(Instruction));
                if (!state->memory) {
                    perror("Failed to reallocate memory");
                    exit(1);
                }
            }

            // בדיקה אם האופרנד הוא תווית חיצונית
            bool is_extern = false;
            for (int i = 0; i < state->extern_count; i++) {
                if (strcmp(state->extern_table[i].name, operand1) == 0) {
                    is_extern = true;
                    break;
                }
            }

            if (is_extern) {
                strcpy(state->memory[state->IC].binary, "000000000000001");  // השתמש ב-001 עבור תוויות חיצוניות
            } else {
                strcpy(state->memory[state->IC].binary, "000000000000010");  // השתמש ב-010 עבור תוויות שאינן חיצוניות
            }
            state->memory[state->IC].address = state->IC;
            snprintf(state->memory[state->IC].label, sizeof(state->memory[state->IC].label), "%s", operand1);
            state->IC++;
            state->memory_size++;
        } else if (src_addressing == 4 || src_addressing == 8) {
            int reg_word = 0b100 | (src_reg << 6);
            char *reg_word_binary = int_to_binary(reg_word, 15);

            if (state->memory_size >= state->memory_capacity) {
                state->memory_capacity *= 2;
state->memory_capacity * sizeof(Instruction);
                if (!state->memory) {
                    perror("Failed to reallocate memory");
                    exit(1);
                }
            }

            strcpy(state->memory[state->IC].binary, reg_word_binary);
            free(reg_word_binary);
            state->memory[state->IC].address = state->IC;
            strcpy(state->memory[state->IC].label, "");
            state->IC++;
            state->memory_size++;
        }
        
        if (dst_addressing != 0) {
            if (dst_addressing == 1) {
                char* encoded_immediate = encode_immediate_operand(operand2);
                if (encoded_immediate != NULL) {
                    if (state->memory_size >= state->memory_capacity) {
                        state->memory_capacity *= 2;
                        state->memory = realloc(state->memory, state->memory_capacity * sizeof(Instruction));
                        if (!state->memory) {
                            perror("Failed to reallocate memory");
                            exit(1);
                        }
                    }

                    strcpy(state->memory[state->IC].binary, encoded_immediate);
                    free(encoded_immediate);
                    state->memory[state->IC].address = state->IC;
                    strcpy(state->memory[state->IC].label, "");
                    state->IC++;
                    state->memory_size++;
                } else {
                    fprintf(stderr, "שגיאה: אופרנד מיידי לא חוקי %s\n", operand2);
                }
            } else if (dst_addressing == 2) {
                if (state->memory_size >= state->memory_capacity) {
                    state->memory_capacity *= 2;
                    state->memory = realloc(state->memory, state->memory_capacity * sizeof(Instruction));
                    if (!state->memory) {
                        perror("Failed to reallocate memory");
                        exit(1);
                    }
                }

                // בדיקה אם האופרנד הוא תווית חיצונית
                bool is_extern = false;
                for (int i = 0; i < state->extern_count; i++) {
                    if (strcmp(state->extern_table[i].name, operand2) == 0) {
                        is_extern = true;
                        break;
                    }
                }

                if (is_extern) {
                    strcpy(state->memory[state->IC].binary, "000000000000001");  // השתמש ב-001 עבור תוויות חיצוניות
                } else {
                    strcpy(state->memory[state->IC].binary, "000000000000010");  // השתמש ב-010 עבור תוויות שאינן חיצוניות
                }
                state->memory[state->IC].address = state->IC;
                snprintf(state->memory[state->IC].label, sizeof(state->memory[state->IC].label), "%s", operand2);
                state->IC++;
                state->memory_size++;
            } else if (dst_addressing == 4 || dst_addressing == 8) {
                int reg_word = 0b100 | (dst_reg << 3);
                char *reg_word_binary = int_to_binary(reg_word, 15);

                if (state->memory_size >= state->memory_capacity) {
                    state->memory_capacity *= 2;
                    state->memory = realloc(state->memory, state->memory_capacity * sizeof(Instruction));
                    if (!state->memory) {
                        perror("Failed to reallocate memory");
                        exit(1);
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
}
void process_line(AssemblerState* state, char* line) {
 
    char *label = NULL;
    char *op = NULL;
    char *operand1 = NULL;
    char *operand2 = NULL;
   

    // בדיקת אורך שורה
    if (!check_line_length(line)) {
        fprintf(stderr, "שגיאה: אורך השורה חורג מהמותר\n");
        return;
    }

    // הסרת רווחים מתחילת וסוף השורה
    char* trimmed_line = trim(line);

    // בדיקה אם השורה ריקה או הערה
    if (is_empty_or_whitespace(trimmed_line) || is_valid_comment(trimmed_line)) {
        return;
    }

    char *token = strtok(trimmed_line, " \t");
    if (token == NULL) return;

    // בדיקת תווית
    if (strchr(token, ':') != NULL) {
        label = token;
        label[strlen(label) - 1] = '\0';  // הסרת הנקודותיים
        
        if (!is_valid_label(label)) {
            fprintf(stderr, "שגיאה: תווית לא חוקית\n");
            return;
        }
        /*if (is_reserved_word(label)) {
            fprintf(stderr, "שגיאה: שימוש במילה שמורה כתווית\n");
            return;
        }*/
        if (is_duplicate_label(label, state)) {
            fprintf(stderr, "שגיאה: הגדרה כפולה של תווית\n");
            return;
        }
        //add_label(state, label, state->IC);
        token = strtok(NULL, " \t");
        if (token == NULL) return;
    }

    op = token;

    

    // טיפול בהוראות מיוחדות
    if (op[0] == '.') {
        operand1 = strtok(NULL, "");
        if (operand1) operand1 = trim(operand1);

        if (strcmp(op, ".data") == 0) {
            handle_data_directive(state, label ? label : "", operand1);
        } else if (strcmp(op, ".string") == 0) {
            handle_string_directive(state, label ? label : "", operand1);
        } else if (strcmp(op, ".entry") == 0) {
            if (label) {
                fprintf(stderr, "אזהרה: התווית לפני הנחיית .entry נתעלמה\n");
            }
            handle_entry_directive(state, operand1);
        } else if (strcmp(op, ".extern") == 0) {
            if (label) {
                fprintf(stderr, "אזהרה: התווית לפני הנחיית .extern נתעלמה\n");
            }
            handle_extern_directive(state, operand1);
        }
    } 
    else 
    {
        // בדיקת תקינות ההוראה
        if (!is_valid_instruction(op)) {
            printf("שגיאה: הוראה לא חוקית ");
            printf("at op - %s\n", op);
            return;
        }
        // טיפול בהוראות רגילות
        operand1 = strtok(NULL, ",");
        operand2 = strtok(NULL, "");

        if (operand1) operand1 = trim(operand1);
        if (operand2) operand2 = trim(operand2);

        int operand_count = 0;
        if (operand1) operand_count++;
        if (operand2) operand_count++;

        // בדיקת מספר האופרנדים
        if (!check_operand_count(op, operand_count)) {
            fprintf(stderr, "שגיאה: מספר אופרנדים שגוי להוראה\n");
            return;
        }

        // בדיקת תקינות האופרנדים
        char* operands[2] = {operand1, operand2};
        for (int i = 0; i < operand_count; i++) {
            if (!is_valid_operand(operands[i])) {
                fprintf(stderr, "שגיאה: אופרנד לא חוקי\n");
                return;
            }
            
            if (!is_valid_addressing_mode(op, operands[i], i == 0)) {
                fprintf(stderr, "שגיאה: שיטת מיעון לא חוקית לאופרנד %s\n", i == 0 ? "מקור" : "יעד");
                return;
            }
            
            if (operands[i][0] == '#' && !is_valid_immediate(operands[i])) {
                fprintf(stderr, "שגיאה: ערך מיידי לא חוקי\n");
                return;
            }
        }

       
        // הרכבת ההוראה
        assemble_instruction(state, label, op, operand1, operand2);
    }


}

void first_pass(AssemblerState* state, const char* filename) {
   /* printf("Trying to open file: %s\n", filename);*/
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }

    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), file)) {
        char* newline = strchr(line, '\n');
        if (newline) *newline = '\0';  // Remove newline character
        
        char* trimmed_line = trim(line);
         
        if (trimmed_line[0] == '\0' || trimmed_line[0] == ';') {
            continue;
        }
        process_line(state, trimmed_line);
    }

    fclose(file);
}
void update_entry_addresses(AssemblerState* state) {
    for (int i = 0; i < state->entry_count; i++) {
         int address = get_label_address(state, state->entry_table[i].name);
        if (address == -1) {
            fprintf(stderr, "שגיאה: תווית Entry '%s' לא נמצאה בטבלת הסמלים\n", state->entry_table[i].name);
        } else {
            state->entry_table[i].address = address;
         }
    }
}
void print_memory(AssemblerState* state) {
    for (int i = 100; i < state->IC + state->DC; i++) {
        if (state->memory[i].label[0] != '\0') {
            int is_extern = 0;
            for (int j = 0; j < state->extern_count; j++) {
                if (strcmp(state->extern_table[j].name, state->memory[i].label) == 0) {  // שינוי כאן
                    is_extern = 1;
                    break;
                }
            }

            if (is_extern) {
                printf("%04d \"%s\"\n", i, state->memory[i].label);
            } else {
                printf("%04d \"%s\"\n", i, state->memory[i].label);
            }
        } else {
            printf("%04d %s\n", i, state->memory[i].binary);
        }
    }
}

void print_label_table(AssemblerState* state) {
    printf("Label Table:\n");
    printf("Name\tAddress\n");
    for (int i = 0; i < state->label_count; i++) {
        if (!state->label_table[i].is_extern) {
            printf("%s\t%d\n", state->label_table[i].name, state->label_table[i].address);
        }
    }
}

void print_entry_table(AssemblerState* state) {
    printf("Entry Table:\n");
    printf("Name\tAddress\n");
    for (int i = 0; i < state->entry_count; i++) {
        printf("%s\t%d\n", state->entry_table[i].name, state->entry_table[i].address);
    }
}

/* Modify the print_extern_table function */
void print_extern_table(AssemblerState* state) {
    printf("Extern Table:\n");
    printf("Name\tAddress\n");
    for (int i = 0; i < state->extern_count; i++) {
printf("%s\t", state->extern_table[i].name);
if (state->extern_table[i].addresses != NULL) {
    printf("%04d", state->extern_table[i].addresses[0]);
} else {
    printf("N/A");
}
printf("\n");    }
}
/*
int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <example.as>\n", argv[0]);
        return 1;
    }

    AssemblerState* state = init_assembler_state();
    first_pass(state, argv[1]);
    update_entry_addresses(state);

    print_memory(state);
    print_label_table(state);
    print_entry_table(state);
    print_extern_table(state);

    // קריאה למעבר השני
    char output_filename[256];
    strcpy(output_filename, argv[1]);
    char* dot = strrchr(output_filename, '.');
    if (dot != NULL) {
        *dot = '\0';
    }
    strcat(output_filename, ".ob");
    
    int result = second_pass(state, argv[1], output_filename);
    
    free_assembler_state(state);

    return result;
}*/
/*
int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <example.as>\n", argv[0]);
        return 1;
    }

    AssemblerState* state = init_assembler_state();
    first_pass(state, argv[1]);
    update_entry_addresses(state);

    print_memory(state);
    print_label_table(state);
    print_entry_table(state);
    print_extern_table(state);

    // קריאה למעבר השני
    char output_filename[256];
    strcpy(output_filename, argv[1]);
    char* dot = strrchr(output_filename, '.');
    if (dot != NULL) {
        *dot = '\0';
    }
    strcat(output_filename, ".ob");
    
    int result = second_pass(state, argv[1], output_filename);
    
    free_assembler_state(state);

    return result;
}*/
/*void process_line(AssemblerState *state, char *line)
{

    int error = 0;
    char *label = NULL;
    char *op = NULL;
    char *operand1 = NULL;
    char *operand2 = NULL;
    trimLeadingWhitespace(line);
    char *token = strtok(line, " \t");

    if (token == NULL)
        return;

    if (strchr(token, ':') != NULL)
    {
        label = token;
        label[strlen(label) - 1] = '\0';
        token = strtok(NULL, " \t");
    }

    if (token == NULL)
        return;

    op = token;
    if (op[0] == '.')
    {
        operand1 = strtok(NULL, "");

        if (operand1)
            operand1 = trim(operand1);

        if (strncmp(line, ".data", 5) == 0)
        {
            if (line[5] == ' ' || line[5] == '\t')
            {
                char *afterData = line + 6;
                data_intergity_check(afterData, &error);
                if (error == 0)
                {
                    handle_data_directive(state, label ? label : "", operand1);
                }
                else
                {
                    fprintf(stderr, "Invalid data instruction\n");
                }
            }
            else
            {
                fprintf(stderr, "Error: Missing space after .data directive\n");
                return;
            }
        }
        else if (strncmp(op, ".string", 7) == 0)
        {
            if (op[7] == ' ' || op[7] == '\t')
            {
                char *afterString = op + 8;
                string_intergity_check(afterString, &error);
                if (error == 0)
                {
                    handle_string_directive(state, label ? label : "", operand1);
                }
                else
                {
                    fprintf(stderr, "Invalid String instruction\n");
                }
            }
            else
            {
                fprintf(stderr, "Error: Missing space after .string directive\n");
                return;
            }
        }
        else if (strncmp(op, ".entry", 6) == 0)
        {
            if (label)
            {
                fprintf(stderr, "אזהרה: התווית לפני הנחיית .entry נתעלמה\n");
            }
            if (op[6] == ' ' || op[6] == '\t')
            {
                char *afterEntry = op + 7;
                entry_intergity_check(afterEntry, &error);
                if (error == 0)
                {
                    handle_entry_directive(state, operand1);
                }
                else
                {
                    fprintf(stderr, "Invalid Entry instruction\n");
                }
            }
            else
            {
                fprintf(stderr, "Error: Missing space after .entry directive\n");
            }
        }
        else if (strncmp(op, ".extern", 7) == 0)
        {
            if (label)
            {
                fprintf(stderr, "אזהרה: התווית לפני הנחיית .extern נתעלמה\n");
            }
            if (op[7] == ' ' || op[7] == '\t')
            {
                char *afterExtern = op + 8;
                extern_intergity_check(afterExtern, &error);
                if (error == 0)
                {
                    fprintf(stderr, "handle_extern_directive(state, operand1);\n");
                }
                else
                {
                    fprintf(stderr, "Invalid Extern instruction\n");
                }
            }
            else
            {
                fprintf(stderr, "Error: Missing space after .extern directive\n");
            }
        }
        else
        {
            operand1 = strtok(NULL, ",");
            operand2 = strtok(NULL, "");

            if (operand1)
                operand1 = trim(operand1);
            if (operand2)
                operand2 = trim(operand2);

            assemble_instruction(state, label, op, operand1, operand2);
        }
    }
}*/