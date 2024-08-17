#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <limits.h>

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
 * בודק אם אורך השורה תקין
 */
bool check_line_length(const char* line) {
    return strlen(line) <= MAX_LINE_LENGTH;
}

/**
 * בודק אם השורה ריקה או מכילה רק רווחים
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
 * בודק אם השורה היא הערה תקינה
 */
bool is_valid_comment(const char* line) {
    return line[0] == ';';
}

/**
 * בודק אם התווית תקינה
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
 * בודק אם המילה היא מילה שמורה
 */
bool is_reserved_word(const char* word) {
    for (int i = 0; i < num_reserved_words; i++) {
        if (strcmp(word, reserved_words[i]) == 0)
            return true;
    }
    return false;
}

/**
 * בודק אם התווית כבר קיימת
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
 * בודק אם ההוראה תקינה
 */
bool is_valid_instruction(const char* instruction) {
    return is_reserved_word(instruction);
}

/**
 * בודק אם המספר המיידי תקין
 */
bool is_valid_immediate(const char* operand) {
    char* endptr;
    long value = strtol(operand + 1, &endptr, 10);
    return *endptr == '\0' && value >= -2078 && value <= 2047;
}

/**
 * בודק אם מספר האופרנדים תקין להוראה
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
 * בודק אם האופרנד תקין
 */
bool is_valid_operand(const char* operand) {
    if (operand[0] == '#') {
        char* endptr;
        long value = strtol(operand + 1, &endptr, 10);
        return *endptr == '\0' && value >= -2078 && value <= 2047;
    }
    
    if (operand[0] == '*') {
        if (strlen(operand) == 2 && operand[1] == 'r') {
            return true;
        }
        return strlen(operand) == 3 && operand[1] == 'r' && operand[2] >= '0' && operand[2] <= '7';
    }
    
    if (operand[0] == 'r' && strlen(operand) == 2) {
        return operand[1] >= '0' && operand[1] <= '7';
    }
    
    return is_valid_label(operand);
}

/**
 * בודק אם שיטת המיעון תקינה להוראה ולאופרנד
 */
bool is_valid_addressing_mode(const char* instruction, const char* operand, bool is_source) {
    bool is_immediate = (operand[0] == '#');
    bool is_direct_register = (operand[0] == 'r' && strlen(operand) == 2 && operand[1] >= '0' && operand[1] <= '7');
    bool is_indirect_register = (operand[0] == '*' && strlen(operand) == 3 && operand[1] == 'r' && operand[2] >= '0' && operand[2] <= '7');
    bool is_direct = (!is_immediate && !is_direct_register && !is_indirect_register);

    if (strcmp(instruction, "lea") == 0) {
        if (is_source)
            return is_direct;
        else
            return !is_immediate;
    }
    
    if (strcmp(instruction, "cmp") == 0)
        return true;
    
    if (strcmp(instruction, "mov") == 0 || strcmp(instruction, "add") == 0 || strcmp(instruction, "sub") == 0) {
        if (is_source)
            return true;
        else
            return !is_immediate;
    }
    
    if (strcmp(instruction, "clr") == 0 || strcmp(instruction, "not") == 0 ||
        strcmp(instruction, "inc") == 0 || strcmp(instruction, "dec") == 0 ||
        strcmp(instruction, "red") == 0)
        return !is_immediate;
    
    if (strcmp(instruction, "jmp") == 0 || strcmp(instruction, "bne") == 0 ||
        strcmp(instruction, "jsr") == 0)
        return is_direct || is_indirect_register;
    
    if (strcmp(instruction, "prn") == 0)
        return true;

    return false;
}

/**
 * מוסיף תווית למערכת
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
 * בודק אם התווית קיימת
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
 * בודק אם התווית של entry תקינה (רק אותיות גדולות)
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
 * בודק אם תווית ה-entry מוגדרת בקובץ
 */
bool is_entry_label_defined(const AssemblyState* state, const char* label) {
    for (int i = 0; i < state->label_count; i++) {
        if (strcmp(state->labels[i].name, label) == 0) {
            return true;
        }
    }
    return false;
}

/**
 * בודק אם תווית ה-extern לא מוגדרת בקובץ
 */
bool is_extern_label_not_defined(const AssemblyState* state, const char* label) {
    for (int i = 0; i < state->label_count; i++) {
        if (strcmp(state->labels[i].name, label) == 0) {
            return false;
        }
    }
    return true;
}

/**
 * מנתח שורה בודדת של קוד האסמבלי
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

    if (strlen(trimmed_line) > MAX_LINE_LENGTH) {
        printf("שגיאה בשורה %d: אורך השורה חורג מהמותר\n", line_number);
        return;
    }

    if (trimmed_line[0] == '\0' || trimmed_line[0] == ';') {
        return;
    }

    char* token = strtok(trimmed_line, " \t");
    if (token == NULL) return;

    // בדיקת הוראות .entry ו-.extern
    if (strcmp(token, ".entry") == 0) {
        token = strtok(NULL, " \t");
        if (token && is_valid_entry_label(token)) {
            if (!is_entry_label_defined(state, token)) {
                printf("שגיאה בשורה %d: תווית .entry אינה מוגדרת בקובץ\n", line_number);
            }
        } else {
            printf("שגיאה בשורה %d: תווית .entry לא תקינה\n", line_number);
        }
        return;
    }

    if (strcmp(token, ".extern") == 0) {
        token = strtok(NULL, " \t");
        if (token && is_valid_label(token)) {
            if (!is_extern_label_not_defined(state, token)) {
                printf("שגיאה בשורה %d: תווית .extern מוגדרת בקובץ\n", line_number);
            }
        } else {
            printf("שגיאה בשורה %d: תווית .extern לא תקינה\n", line_number);
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
            printf("שגיאה בשורה %d: תווית לא תקינה\n", line_number);
            return;
        }
        if (is_reserved_word(label)) {
            printf("שגיאה בשורה %d: שימוש במילה שמורה כתווית\n", line_number);
            return;
        }
        if (is_duplicate_label(label, state)) {
            printf("שגיאה בשורה %d: הגדרה כפולה של תווית\n", line_number);
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
        printf("שגיאה בשורה %d: הוראה לא תקינה\n", line_number);
        return;
    }

    if (strcmp(instruction, "stop") == 0) {
        if (state->stop_encountered) {
            printf("שגיאה בשורה %d: הוראת STOP כבר הופיעה קודם לכן\n", line_number);
            return;
        }
        state->stop_encountered = true;
    }

    char* rest_of_line = strtok(NULL, "\n");
    if (rest_of_line != NULL) {
        while (isspace(*rest_of_line)) rest_of_line++;
        
        if (*rest_of_line == ',') {
            printf("שגיאה בשורה %d: תו לא חוקי אחרי ההוראה\n", line_number);
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
                printf("שגיאה בשורה %d: אופרנד לא תקין\n", line_number);
                return;
            }

            operand = strtok(NULL, ",");
        }
    }

    if (!check_operand_count(instruction, operand_count)) {
        printf("שגיאה בשורה %d: מספר אופרנדים שגוי להוראה\n", line_number);
        return;
    }

    if (operand_count > 0 && !is_valid_addressing_mode(instruction, operands[0], true)) {
        printf("שגיאה בשורה %d: שיטת מיעון לא חוקית לאופרנד מקור\n", line_number);
        return;
    }
    if (operand_count > 1 && !is_valid_addressing_mode(instruction, operands[1], false)) {
        printf("שגיאה בשורה %d: שיטת מיעון לא חוקית לאופרנד יעד\n", line_number);
        return;
    }

    for (int i = 0; i < operand_count; i++) {
        if (operands[i][0] == '#' && !is_valid_immediate(operands[i])) {
            printf("שגיאה בשורה %d: מספר לא תקין בשיטת מיעון מיידי\n", line_number);
            return;
        }
    }

    state->line_count++;
}

/**
 * פונקציית main - מנתחת את קובץ הקלט
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
        printf("שגיאה: הוראת STOP לא נמצאה בקוד\n");
    }

    return 0;
}