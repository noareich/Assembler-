#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// הגדרת קבועים
#define MAX_LINE_LENGTH 100
#define MAX_LABEL_LENGTH 31
#define MAX_LABELS 1000
#define MEMORY_SIZE 1000
#define MAX_ENTRIES 100
#define MAX_EXTERNS 100

// הגדרת מבנים
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

// משתנים גלובליים
Instruction memory[MEMORY_SIZE];
Label label_table[MAX_LABELS];
int label_count = 0;
int IC = 100; // מונה פקודות
int DC = 0;   // מונה נתונים

char entry_table[MAX_ENTRIES][MAX_LABEL_LENGTH + 1];
int entry_count = 0;

char extern_table[MAX_EXTERNS][MAX_LABEL_LENGTH + 1];
int extern_count = 0;

// הצהרות על פונקציות
char* trim(char* str);
int get_opcode(const char* operation);
int get_addressing_mode(const char* operand);
int get_register_number(const char* operand);
char* int_to_binary(int num, int length);
void add_label(const char* name, int address);
int get_label_address(const char* name);
void handle_data_directive(const char* label, const char* params);
void handle_string_directive(const char* label, const char* params);
void handle_entry_directive(const char* label);
void handle_extern_directive(const char* label);
void assemble_instruction(const char *label, const char *op, const char *operand1, const char *operand2);
void process_line(char* line);
void first_pass(const char* filename);
void print_memory(void);
void print_label_table(void);
void print_entry_table(void);
void print_extern_table(void);
void update_data_labels(void);
void reset_assembler(void);

// פונקציות עזר

// הסרת רווחים מתחילת וסוף המחרוזת
char* trim(char* str) {
    char* end;
    while(isspace((unsigned char)*str)) str++;
    if(*str == 0) return str;
    end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
    return str;
}

// קבלת קוד האופרציה לפי שם הפעולה
int get_opcode(const char* operation) {
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
    return -1;  // פעולה לא חוקית
}

// קבלת מצב הכתובת של האופרנד
int get_addressing_mode(const char* operand) {
    if (operand[0] == '#') return 1;  // מיידי
    if (operand[0] == 'r') return 8;  // רגיסטר ישיר
    if (operand[0] == '*') return 4;  // רגיסטר עקיף
    return 2;  // ישיר (תווית)
}

// קבלת מספר הרגיסטר
int get_register_number(const char* operand) {
    if (operand[0] == 'r' && isdigit(operand[1])) {
        return operand[1] - '0';
    }
    return -1;  // לא רגיסטר
}

// המרת מספר שלם למחרוזת בינארית
char* int_to_binary(int num, int length) {
    char* binary = (char*)malloc(length + 1);
    if (binary == NULL) {
        fprintf(stderr, "הקצאת זיכרון נכשלה\n");
        exit(EXIT_FAILURE);
    }
    binary[length] = '\0';
    for (int i = length - 1; i >= 0; i--) {
        binary[i] = (num & 1) ? '1' : '0';
        num >>= 1;
    }
    return binary;
}

// הוספת תווית לטבלת התוויות
void add_label(const char* name, int address) {
    if (label_count < MAX_LABELS) {
        strncpy(label_table[label_count].name, name, MAX_LABEL_LENGTH);
        label_table[label_count].name[MAX_LABEL_LENGTH] = '\0';
        label_table[label_count].address = address;
        label_table[label_count].is_entry = 0;
        label_table[label_count].is_extern = 0;
        label_count++;
    } else {
        fprintf(stderr, "גלישת טבלת תוויות\n");
    }
}

// אסמבול הוראה
void assemble_instruction(const char *label, const char *op, const char *operand1, const char *operand2) {
    int opcode = get_opcode(op);
    int src_addressing = 0, dst_addressing = 0;
    int src_reg = -1, dst_reg = -1;
    int immediate_value = 0;
    char src_label[MAX_LABEL_LENGTH + 3] = "";
    char dst_label[MAX_LABEL_LENGTH + 3] = "";
    
    // טיפול בתווית
    if (label && label[0] != '\0' && strncmp(label, "r", 1) != 0) {
        add_label(label, IC);
    }

    // ניתוח אופרנדים
    if (operand1) {
        src_addressing = get_addressing_mode(operand1);
        if (src_addressing == 1) {
            immediate_value = atoi(operand1 + 1);
        } else if (src_addressing == 4) {
            src_reg = get_register_number(operand1 + 1);
        } else if (src_addressing == 8) {
            src_reg = get_register_number(operand1);
        } else {
            snprintf(src_label, sizeof(src_label), "\"%s\"", operand1);
        }
    }
    if (operand2) {
        dst_addressing = get_addressing_mode(operand2);
        if (dst_addressing == 1) {
            immediate_value = atoi(operand2 + 1);
        } else if (dst_addressing == 4) {
            dst_reg = get_register_number(operand2 + 1);
        } else if (dst_addressing == 8) {
            dst_reg = get_register_number(operand2);
        } else {
            snprintf(dst_label, sizeof(dst_label), "\"%s\"", operand2);
        }
    }
    
    // מילה ראשונה
    int first_word = (opcode << 11) | (src_addressing << 7) | (dst_addressing << 3) | 0b100;
    char *first_word_binary = int_to_binary(first_word, 15);
    strcpy(memory[IC].binary, first_word_binary);
    free(first_word_binary);
    memory[IC].address = IC;
    strcpy(memory[IC].label, "");
    IC++;
    
    // מילה שנייה (אם נדרשת)
    if ((src_addressing == 4 || src_addressing == 8) && (dst_addressing == 4 || dst_addressing == 8)) {
        // שני האופרנדים הם רגיסטרים (ישירים או עקיפים)
        int reg_word = 0b100 | (src_reg << 6) | (dst_reg << 3);
        char *reg_word_binary = int_to_binary(reg_word, 15);
        strcpy(memory[IC].binary, reg_word_binary);
        free(reg_word_binary);
        memory[IC].address = IC;
        strcpy(memory[IC].label, "");
        IC++;
    } else {
        // טיפול באופרנד מקור
        if (src_addressing == 1) {
            // מיידי
            int immediate_word = ((immediate_value & 0x3FFF) << 2) | 0b100;
            char *immediate_word_binary = int_to_binary(immediate_word, 15);
            strcpy(memory[IC].binary, immediate_word_binary);
            free(immediate_word_binary);
            memory[IC].address = IC;
            strcpy(memory[IC].label, "");
            IC++;
        } else if (src_addressing == 2) {
            // ישיר (תווית)
            strcpy(memory[IC].binary, "000000000000100");
            memory[IC].address = IC;
            strncpy(memory[IC].label, src_label, MAX_LABEL_LENGTH + 2);
            memory[IC].label[MAX_LABEL_LENGTH + 2] = '\0';
            IC++;
        } else if (src_addressing == 4 || src_addressing == 8) {
            // רגיסטר (עקיף או ישיר)
            int reg_word = 0b100 | (src_reg << 6);
            char *reg_word_binary = int_to_binary(reg_word, 15);
            strcpy(memory[IC].binary, reg_word_binary);
            free(reg_word_binary);
            memory[IC].address = IC;
            strcpy(memory[IC].label, "");
            IC++;
        }
        
        // טיפול באופרנד יעד
        if (dst_addressing != 0) {
            if (dst_addressing == 1) {
                // מיידי
                int immediate_word = ((immediate_value & 0x3FFF) << 2) | 0b100;
                char *immediate_word_binary = int_to_binary(immediate_word, 15);
                strcpy(memory[IC].binary, immediate_word_binary);
                free(immediate_word_binary);
                memory[IC].address = IC;
                strcpy(memory[IC].label, "");
                IC++;
            } else if (dst_addressing == 2) {
                // ישיר (תווית)
                strcpy(memory[IC].binary, "000000000000100");
                memory[IC].address = IC;
                strncpy(memory[IC].label, dst_label, MAX_LABEL_LENGTH + 2);
                memory[IC].label[MAX_LABEL_LENGTH + 2] = '\0';
                IC++;
            } else if (dst_addressing == 4 || dst_addressing == 8) {
                // רגיסטר (עקיף או ישיר)
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

// עיבוד שורה בודדת של קוד אסמבלי
void process_line(char* line) {
    char* label = NULL;
    char* op = NULL;
    char* operand1 = NULL;
    char* operand2 = NULL;
    char* token = strtok(line, " \t");
    
    if (token == NULL) return;

    // בדיקה אם יש תווית
    if (strchr(token, ':') != NULL) {
        label = token;
        label[strlen(label) - 1] = '\0';
        token = strtok(NULL, " \t");
    }

    if (token == NULL) return;

    op = token;
    if (op[0] == '.') {
        // טיפול בהנחיות
        operand1 = strtok(NULL, "");
        
        if (operand1) operand1 = trim(operand1);
        
        if (strcmp(op, ".data") == 0) {
            handle_data_directive(label ? label : "", operand1);
        } else if (strcmp(op, ".string") == 0) {
            handle_string_directive(label ? label : "", operand1);
        } else if (strcmp(op, ".entry") == 0) {
            if (label) {
                fprintf(stderr, "אזהרה: תווית לפני הנחיית .entry מתעלמים ממנה\n");
            }
            handle_entry_directive(operand1);
        } else if (strcmp(op, ".extern") == 0) {
            if (label) {
                fprintf(stderr, "אזהרה: תווית לפני הנחיית .extern מתעלמים ממנה\n");
            }
            handle_extern_directive(operand1);
        }
    } else {
        // טיפול בהוראות
        operand1 = strtok(NULL, ",");
        operand2 = strtok(NULL, "");

        if (operand1) operand1 = trim(operand1);
        if (operand2) operand2 = trim(operand2);

        assemble_instruction(label, op, operand1, operand2);
    }
}

// איפוס האסמבלר
void reset_assembler() {
    IC = 100;
    DC = 0;
    label_count = 0;
    entry_count = 0;
    extern_count = 0;
    memset(memory, 0, sizeof(memory));
}

// מעבר ראשון של האסמבלר
void first_pass(const char* filename) {
    reset_assembler();
    
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        perror("שגיאה בפתיחת הקובץ");
        return;
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
    update_data_labels();
}

// קבלת כתובת של תווית
int get_label_address(const char* name) {
    for (int i = 0; i < label_count; i++) {
        if (strcmp(label_table[i].name, name) == 0) {
            return label_table[i].address;
        }
    }
    return -1;  // תווית לא נמצאה
}

// הדפסת טבלת התוויות
void print_label_table() {
    printf("טבלת תוויות:\n");
    printf("שם\tכתובת\n");
    for (int i = 0; i < label_count; i++) {
        printf("%s\t%d\n", label_table[i].name, label_table[i].address);
    }
}

// טיפול בהנחיית .data
void handle_data_directive(const char *label, const char *params) {
    char *token;
    char *params_copy = strdup(params);
    char *saveptr;

    if (label && label[0] != '\0') {
        add_label(label, IC + DC);
    }

    token = strtok_r(params_copy, ",", &saveptr);
    while (token != NULL) {
        while (isspace(*token)) token++;

        int value = atoi(token);
        value &= 0x7FFF;  // וידוא ערך 15 ביט
        char *word_binary = int_to_binary(value, 15);
        strcpy(memory[IC + DC].binary, word_binary);
        memory[IC + DC].address = IC + DC;
        free(word_binary);
        DC++;

        token = strtok_r(NULL, ",", &saveptr);
    }

    free(params_copy);
}

// עדכון כתובות התוויות של נתונים
void update_data_labels() {
    for (int i = 0; i < label_count; i++) {
        if (label_table[i].address >= 100 && label_table[i].address < IC) {
            label_table[i].address += DC;
        }
    }
    IC += DC;  // עדכון IC לאחר הוספת כל הנתונים
}

// טיפול בהנחיית .string
void handle_string_directive(const char *label, const char *params) {
    if (label && label[0] != '\0') {
        add_label(label, IC + DC);
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
        char *word_binary = int_to_binary((int)*start, 15);
        strcpy(memory[IC + DC].binary, word_binary);
        memory[IC + DC].address = IC + DC;
        free(word_binary);
        DC++;
        start++;
    }

    // הוספת תו סיום
    char *null_binary = int_to_binary(0, 15);
    strcpy(memory[IC + DC].binary, null_binary);
    memory[IC + DC].address = IC + DC;
    free(null_binary);
    DC++;
}

// טיפול בהנחיית .entry
void handle_entry_directive(const char *label) {
    for (int i = 0; i < extern_count; i++) {
        if (strcmp(extern_table[i], label) == 0) {
            printf("שגיאה: התווית '%s' כבר הוצהרה כחיצונית\n", label);
            return;
        }
    }

    for (int i = 0; i < entry_count; i++) {
        if (strcmp(entry_table[i], label) == 0) {
            printf("אזהרה: התווית '%s' כבר הוצהרה כנקודת כניסה\n", label);
            return;
        }
    }
    
    if (entry_count < MAX_ENTRIES) {
        strncpy(entry_table[entry_count], label, MAX_LABEL_LENGTH);
        entry_table[entry_count][MAX_LABEL_LENGTH] = '\0';
        entry_count++;
    } else {
        fprintf(stderr, "שגיאה: יותר מדי הצהרות נקודות כניסה\n");
    }
}

// טיפול בהנחיית .extern
void handle_extern_directive(const char *label) {
    for (int i = 0; i < entry_count; i++) {
        if (strcmp(entry_table[i], label) == 0) {
            printf("שגיאה: התווית '%s' כבר הוצהרה כנקודת כניסה\n", label);
            return;
        }
    }

    for (int i = 0; i < extern_count; i++) {
        if (strcmp(extern_table[i], label) == 0) {
            printf("אזהרה: התווית '%s' כבר הוצהרה כחיצונית\n", label);
            return;
        }
    }
    
    if (extern_count < MAX_EXTERNS) {
        strncpy(extern_table[extern_count], label, MAX_LABEL_LENGTH);
        extern_table[extern_count][MAX_LABEL_LENGTH] = '\0';
        extern_count++;

        add_label(label, 0);
        label_table[label_count - 1].is_extern = 1;
    } else {
        fprintf(stderr, "שגיאה: יותר מדי הצהרות חיצוניות\n");
    }
}

// הדפסת טבלת נקודות כניסה
void print_entry_table() {
    printf("טבלת נקודות כניסה:\n");
    for (int i = 0; i < entry_count; i++) {
        printf("%s\n", entry_table[i]);
    }
}

// הדפסת טבלת הצהרות חיצוניות
void print_extern_table() {
    printf("טבלת הצהרות חיצוניות:\n");
    for (int i = 0; i < extern_count; i++) {
        printf("%s\n", extern_table[i]);
    }
}

// הדפסת הזיכרון
void print_memory() {
    for (int i = 100; i < IC + DC; i++) {
        if (memory[i].label[0] != '\0') {
            if (memory[i].label[0] != '"' && strncmp(memory[i].label, "r", 1) != 0) {
                printf("%04d %s\n", i, memory[i].label);
            } else if (memory[i].label[0] == '"') {
                printf("%04d %s\n", i, memory[i].label);
            }
        } else {
            printf("%04d %s\n", i, memory[i].binary);
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "שימוש: %s <example.as>\n", argv[0]);
        return 1;
    }

    first_pass(argv[1]);
    print_memory();
    print_label_table();
    print_entry_table();
    print_extern_table();

    return 0;
    }