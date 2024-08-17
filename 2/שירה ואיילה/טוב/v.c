/*
 * קובץ: assembler.c
 * תיאור: מימוש של אסמבלר לשפת אסמבלי דמיונית
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* הגדרת קבועים */
#define MAX_LINE_LENGTH 100   /* אורך מקסימלי של שורה בקובץ הקלט */
#define MAX_LABEL_LENGTH 31   /* אורך מקסימלי של תווית */
#define MAX_LABELS 1000       /* מספר מקסימלי של תוויות */
#define MEMORY_SIZE 1000      /* גודל הזיכרון הוירטואלי */
#define MAX_ENTRIES 100       /* מספר מקסימלי של הצהרות entry */
#define MAX_EXTERNS 100       /* מספר מקסימלי של הצהרות extern */

/* הגדרת מבני נתונים */

/* מבנה המייצג הוראה בזיכרון */
typedef struct
{
    int address;              /* כתובת ההוראה בזיכרון */
    char binary[16];          /* ייצוג בינארי של ההוראה */
    char label[MAX_LABEL_LENGTH + 1];  /* תווית אם קיימת */
} Instruction;

/* מבנה המייצג תווית בטבלת הסמלים */
typedef struct
{
    char name[MAX_LABEL_LENGTH + 1];  /* שם התווית */
    int address;              /* כתובת התווית */
    int is_entry;             /* האם התווית מוגדרת כ-entry */
    int is_extern;            /* האם התווית מוגדרת כ-extern */
} Label;

/* מבנה המייצג תווית entry */
typedef struct {
    char name[MAX_LABEL_LENGTH + 1];  /* שם התווית */
    int address;              /* כתובת התווית */
} EntryLabel;

/* משתנים גלובליים */
EntryLabel entry_table[MAX_ENTRIES];  /* טבלת תוויות entry */
int entry_count = 0;          /* מספר תוויות ה-entry */

Instruction memory[MEMORY_SIZE];  /* ייצוג הזיכרון הוירטואלי */
Label label_table[MAX_LABELS];    /* טבלת הסמלים */
int label_count = 0;          /* מספר התוויות בטבלת הסמלים */
int IC = 100;                 /* מונה ההוראות */
int DC = 0;                   /* מונה הנתונים */

char extern_table[MAX_EXTERNS][MAX_LABEL_LENGTH + 1];  /* טבלת תוויות extern */
int extern_count = 0;         /* מספר תוויות ה-extern */

/* הצהרות על פונקציות */
char *trim(char *str);
int get_opcode(const char *operation);
int get_addressing_mode(const char *operand);
int get_register_number(const char *operand);
char *int_to_binary(int num, int length);
void add_label(const char *name, int address);
int get_label_address(const char *name);
void handle_data_directive(const char *label, const char *params);
void handle_string_directive(const char *label, const char *params);
void handle_entry_directive(const char *label);
void handle_extern_directive(const char *label);
void assemble_instruction(const char *label, const char *op, const char *operand1, const char *operand2);
void process_line(char *line);
void first_pass(const char *filename);
void print_memory(void);
void print_label_table(void);
void print_entry_table(void);
void print_extern_table(void);
void reset_assembler(void);
char *strtok_r(char *str, const char *delim, char **saveptr);
void update_data_labels();
int is_single_operand_instruction(const char *op);
void update_entry_addresses();
char* encode_immediate_operand(const char* operand);

/* מימוש הפונקציה strtok_r עבור מערכות שאינן תומכות בה */
char *strtok_r(char *str, const char *delim, char **saveptr)
{
    char *token;
    if (str == NULL)
    {
        str = *saveptr;
    }

    /* דילוג על תווי הפרדה בתחילת המחרוזת */
    str += strspn(str, delim);
    if (*str == '\0')
    {
        *saveptr = str;
        return NULL;
    }

    /* מציאת סוף המילה */
    token = str;
    str = strpbrk(token, delim);
    if (str == NULL)
    {
        *saveptr = strchr(token, '\0');
    }
    else
    {
        *str = '\0';
        *saveptr = str + 1;
    }

    return token;
}

/* פונקציה להסרת רווחים מתחילת וסוף מחרוזת */
char *trim(char *str)
{
    char *end;
    /* הסרת רווחים מתחילת המחרוזת */
    while (isspace((unsigned char)*str))
        str++;
    if (*str == 0)
        return str;
    /* הסרת רווחים מסוף המחרוזת */
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end))
        end--;
    end[1] = '\0';
    return str;
}

/* פונקציה להמרת שם הפעולה לקוד האופרציה המתאים */
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
    return -1; /* פעולה לא חוקית */
}

/* פונקציה לזיהוי שיטת המיעון של אופרנד */
int get_addressing_mode(const char *operand)
{
    if (operand[0] == '#')
        return 1; /* מיידי */
    if (operand[0] == 'r')
        return 8; /* רגיסטר ישיר */
    if (operand[0] == '*')
        return 4; /* רגיסטר עקיף */
    return 2;     /* ישיר (תווית) */
}

/* פונקציה לקבלת מספר הרגיסטר */
int get_register_number(const char *operand)
{
    if (operand[0] == 'r' && isdigit(operand[1]))
    {
        return operand[1] - '0';
    }
    return -1; /* לא רגיסטר */
}

/* פונקציה להמרת מספר שלם לייצוג בינארי */
char* int_to_binary(int value, int bits) {
    char* binary = (char*)malloc(bits + 1);
    if (binary == NULL) {
        fprintf(stderr, "שגיאה: הקצאת זיכרון נכשלה.\n");
        exit(1);
    }
    
    binary[bits] = '\0'; /* סיום המחרוזת */
    for (int i = bits - 1; i >= 0; i--) {
        binary[i] = (value & 1) ? '1' : '0';
        value >>= 1;
    }
    
    return binary;
}

/* פונקציה להוספת תווית לטבלת הסמלים */
void add_label(const char *name, int address)
{
     if (label_count < MAX_LABELS)
    {
        /* בדיקה אם התווית היא חיצונית */
        for (int i = 0; i < extern_count; i++)
        {
            if (strcmp(extern_table[i], name) == 0)
            {
                /* זו תווית חיצונית, לא נוסיף אותה לטבלת התוויות הרגילה */
                return;
            }
        }

        strncpy(label_table[label_count].name, name, MAX_LABEL_LENGTH);
        label_table[label_count].name[MAX_LABEL_LENGTH] = '\0';
        label_table[label_count].address = address;
        label_table[label_count].is_entry = 0;
        label_table[label_count].is_extern = 0;
        label_count++;
    }
    else
    {
        fprintf(stderr, "גלישת טבלת התוויות\n");
    }
}

/* פונקציה לעדכון כתובות של תוויות נתונים */
void update_data_labels()
{
 
    /* עדכון ה-IC עם הערך של DC ואיפוס DC */
    if (DC > 0)
    {
        IC += DC;
        DC = 0;
     }

    /* עדכון כתובות התוויות */
    for (int i = 0; i < label_count; i++)
    {
         if (label_table[i].address >= 100 && label_table[i].address < IC)
        {
            label_table[i].address += DC;
         }
    }
}

/* פונקציה להרכבת הוראה ושמירתה בזיכרון */
void assemble_instruction(const char *label, const char *op, const char *operand1, const char *operand2) {
            label ? label : "NULL", op, operand1 ? operand1 : "NULL", operand2 ? operand2 : "NULL";

    int opcode = get_opcode(op);
 
    int src_addressing = 0, dst_addressing = 0;
    int src_reg = -1, dst_reg = -1;
    int immediate_value = 0;
    char src_label[MAX_LABEL_LENGTH + 3] = "";
    char dst_label[MAX_LABEL_LENGTH + 3] = "";

    /* הוספת תווית אם קיימת */
    if (label && label[0] != '\0' && strncmp(label, "r", 1) != 0) {
         add_label(label, IC);
    }

    /* טיפול בהוראות עם אופרנד יחיד */
    if (is_single_operand_instruction(op)) {
        if (operand2 != NULL) {
             return;
        }
         operand2 = operand1;
        operand1 = NULL;
    }

    /* ניתוח האופרנדים */
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
    
    /* יצירת המילה הראשונה של ההוראה */
    int first_word = (opcode << 11) | (src_addressing << 7) | (dst_addressing << 3) | 0b100;
    char *first_word_binary = int_to_binary(first_word, 15);
     strcpy(memory[IC].binary, first_word_binary);
    free(first_word_binary);
    memory[IC].address = IC;
    strcpy(memory[IC].label, "");
    IC++;
    
    /* טיפול במילה השנייה (אם נדרש) */
    if ((src_addressing == 4 || src_addressing == 8) && (dst_addressing == 4 || dst_addressing == 8)) {
         int reg_word = 0b100 | (src_reg << 6) | (dst_reg << 3);
        char *reg_word_binary = int_to_binary(reg_word, 15);
         strcpy(memory[IC].binary, reg_word_binary);
        free(reg_word_binary);
        memory[IC].address = IC;
        strcpy(memory[IC].label, "");
        IC++;
    } else {
        /* טיפול באופרנד המקור */
        if (src_addressing == 1) {
             char* encoded_immediate = encode_immediate_operand(operand1);
            if (encoded_immediate != NULL) {
                strcpy(memory[IC].binary, encoded_immediate);
                free(encoded_immediate);
                memory[IC].address = IC;
                strcpy(memory[IC].label, "");
                IC++;
            } else {
                fprintf(stderr, "שגיאה: אופרנד מיידי לא חוקי %s\n", operand1);
            }
        } else if (src_addressing == 2) {
             strcpy(memory[IC].binary, "000000000000100");
            memory[IC].address = IC;
            snprintf(memory[IC].label, sizeof(memory[IC].label), "%s", operand1);
            IC++;
        } else if (src_addressing == 4 || src_addressing == 8) {
             int reg_word = 0b100 | (src_reg << 6);
            char *reg_word_binary = int_to_binary(reg_word, 15);
             strcpy(memory[IC].binary, reg_word_binary);
            free(reg_word_binary);
            memory[IC].address = IC;
            strcpy(memory[IC].label, "");
            IC++;
        }
        
        /* טיפול באופרנד היעד */
        if (dst_addressing != 0) {
            if (dst_addressing == 1) {
                 char* encoded_immediate = encode_immediate_operand(operand2);
                if (encoded_immediate != NULL) {
                    strcpy(memory[IC].binary, encoded_immediate);
                    free(encoded_immediate);
                    memory[IC].address = IC;
                    strcpy(memory[IC].label, "");
                    IC++;
                } else {
                    fprintf(stderr, "שגיאה: אופרנד מיידי לא חוקי %s\n", operand2);
                }
            } else if (dst_addressing == 2) {
                 strcpy(memory[IC].binary, "000000000000100");
                memory[IC].address = IC;
                snprintf(memory[IC].label, sizeof(memory[IC].label), "%s", operand2);
                IC++;
            } else if (dst_addressing == 4 || dst_addressing == 8) {
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

/* פונקציה לעיבוד שורה בודדת מקובץ הקלט */
void process_line(char *line)
{
    char *label = NULL;
    char *op = NULL;
    char *operand1 = NULL;
    char *operand2 = NULL;
    char *token = strtok(line, " \t");

    if (token == NULL)
        return;

    /* בדיקה אם יש תווית בתחילת השורה */
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
        /* טיפול בהנחיות */
        operand1 = strtok(NULL, "");

        if (operand1)
            operand1 = trim(operand1);

        if (strcmp(op, ".data") == 0)
        {
            handle_data_directive(label ? label : "", operand1);
        }
        else if (strcmp(op, ".string") == 0)
        {
            handle_string_directive(label ? label : "", operand1);
        }
        else if (strcmp(op, ".entry") == 0)
        {
            if (label)
            {
                fprintf(stderr, "אזהרה: התווית לפני הנחיית .entry נתעלמה\n");
            }
            handle_entry_directive(operand1);
        }
        else if (strcmp(op, ".extern") == 0)
        {
            if (label)
            {
                fprintf(stderr, "אזהרה: התווית לפני הנחיית .extern נתעלמה\n");
            }
            handle_extern_directive(operand1);
        }
    }
    else
    {
        /* טיפול בהוראות */
        operand1 = strtok(NULL, ",");
        operand2 = strtok(NULL, "");

        if (operand1)
            operand1 = trim(operand1);
        if (operand2)
            operand2 = trim(operand2);

        assemble_instruction(label, op, operand1, operand2);
    }
}

/* פונקציה לאיפוס המצב ההתחלתי של האסמבלר */
void reset_assembler()
{
    IC = 100;
    DC = 0;
    label_count = 0;
    entry_count = 0;
    extern_count = 0;
    memset(memory, 0, sizeof(memory));
}

/* פונקציה לביצוע המעבר הראשון על קובץ הקלט */
void first_pass(const char *filename)
{
    reset_assembler();

    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        perror("שגיאה בפתיחת הקובץ");
        return;
    }

    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), file))
    {
        char *trimmed_line = trim(line);
        if (trimmed_line[0] == '\0' || trimmed_line[0] == ';')
        {
            continue;
        }
        process_line(trimmed_line);
    }
    update_data_labels();

    fclose(file);
}

/* פונקציה לקבלת הכתובת של תווית */
int get_label_address(const char *name)
{
    for (int i = 0; i < label_count; i++)
    {
        if (strcmp(label_table[i].name, name) == 0)
        {
            return label_table[i].address;
        }
    }
    return -1; /* התווית לא נמצאה */
}

/* פונקציה להדפסת טבלת התוויות */
void print_label_table()
{
    printf(" label tabel:\n");
    printf("name \address\n");
    for (int i = 0; i < label_count; i++)
    {
        if (!label_table[i].is_extern)
        {
            printf("%s\t%d\n", label_table[i].name, label_table[i].address);
        }
    }
}

/* פונקציה לטיפול בהנחיית .data */
void handle_data_directive(const char *label, const char *params)
{
    char *token;
    char *params_copy = strdup(params);
    char *saveptr;

    if (label && label[0] != '\0')
    {
        add_label(label, IC + DC);
    }

    token = strtok_r(params_copy, ",", &saveptr);
    while (token != NULL)
    {
        while (isspace(*token))
            token++;

        int value = atoi(token);
        value &= 0x7FFF; /* הבטחת ערך 15 ביט */
        char *word_binary = int_to_binary(value, 15);
        strcpy(memory[IC + DC].binary, word_binary);
        memory[IC + DC].address = IC + DC;
        free(word_binary);
        DC++;

        token = strtok_r(NULL, ",", &saveptr);
    }

    free(params_copy);
}

/* פונקציה לטיפול בהנחיית .string */
void handle_string_directive(const char *label, const char *params)
{
    if (label && label[0] != '\0')
    {
        add_label(label, IC + DC);
    }

    char *start = strchr(params, '"');
    char *end = strrchr(params, '"');
    if (start == NULL || end == NULL || start == end)
    {
        fprintf(stderr, "מחרוזת לא חוקית\n");
        return;
    }

    start++;
    *end = '\0';

    while (*start != '\0')
    {
        char *word_binary = int_to_binary((int)*start, 15);
        strcpy(memory[IC + DC].binary, word_binary);
        memory[IC + DC].address = IC + DC;
        free(word_binary);
        DC++;
        start++;
    }

    /* הוספת תו סיום */
    char *null_binary = int_to_binary(0, 15);
    strcpy(memory[IC + DC].binary, null_binary);
    memory[IC + DC].address = IC + DC;
    free(null_binary);
    DC++;
}

/* פונקציה לטיפול בהנחיית .entry */
void handle_entry_directive(const char *label)
{
    /* בדיקה אם התווית כבר הוכרזה כ-extern */
    for (int i = 0; i < extern_count; i++)
    {
        if (strcmp(extern_table[i], label) == 0)
        {
            printf("שגיאה: התווית '%s' כבר הוכרזה כ-extern\n", label);
            return;
        }
    }

    /* בדיקה אם התווית כבר הוכרזה כ-entry */
    for (int i = 0; i < entry_count; i++)
    {
        if (strcmp(entry_table[i].name, label) == 0)
        {
            printf("אזהרה: התווית '%s' כבר הוכרזה כ-entry\n", label);
            return;
        }
    }

    if (entry_count < MAX_ENTRIES)
    {
        strncpy(entry_table[entry_count].name, label, MAX_LABEL_LENGTH);
        entry_table[entry_count].name[MAX_LABEL_LENGTH] = '\0';
        entry_table[entry_count].address = -1; /* נאתחל ל-1- ונעדכן בשלב מאוחר יותר */
        entry_count++;
    }
    else
    {
        fprintf(stderr, "שגיאה: יותר מדי הצהרות entry\n");
    }
}

/* פונקציה לעדכון כתובות של תוויות entry */
void update_entry_addresses()
{
    for (int i = 0; i < entry_count; i++)
    {
        int address = get_label_address(entry_table[i].name);
        if (address == -1)
        {
            fprintf(stderr, "שגיאה: תווית Entry '%s' לא נמצאה בטבלת הסמלים\n", entry_table[i].name);
        }
        else
        {
            entry_table[i].address = address;
        }
    }
}

/* פונקציה להדפסת טבלת ה-entry */
void print_entry_table()
{
    printf("label Entry:\n");
    printf("name \address\n");
    for (int i = 0; i < entry_count; i++)
    {
        printf("%s\t%d\n", entry_table[i].name, entry_table[i].address);
    }
}

/* פונקציה לטיפול בהנחיית .extern */
void handle_extern_directive(const char *label)
{
    /* בדיקה אם התווית כבר הוכרזה כ-entry */
    for (int i = 0; i < entry_count; i++)
    {
        if (strcmp(entry_table[i].name, label) == 0)
        {
            printf("שגיאה: התווית '%s' כבר הוכרזה כ-entry\n", label);
            return;
        }
    }

    /* בדיקה אם התווית כבר הוכרזה כ-extern */
    for (int i = 0; i < extern_count; i++)
    {
        if (strcmp(extern_table[i], label) == 0)
        {
            printf("אזהרה: התווית '%s' כבר הוכרזה כ-extern\n", label);
            return;
        }
    }

    if (extern_count < MAX_EXTERNS)
    {
        strncpy(extern_table[extern_count], label, MAX_LABEL_LENGTH);
        extern_table[extern_count][MAX_LABEL_LENGTH] = '\0';
        extern_count++;
    }
    else
    {
        fprintf(stderr, "שגיאה: יותר מדי הצהרות extern\n");
    }
}

/* פונקציה להדפסת טבלת ה-extern */
/* פונקציה להדפסת טבלת ה-extern */
void print_extern_table()
{
    printf("label Extern:\n");
    printf("name \taddress\n");
    for (int i = 0; i < extern_count; i++)
    {
        printf("%s\t000000000000001\n", extern_table[i]);
    }
}

/* פונקציה להדפסת תוכן הזיכרון */
void print_memory()
{
    for (int i = 100; i < IC + DC; i++)
    {
        if (memory[i].label[0] != '\0')
        {
            int is_extern = 0;
            for (int j = 0; j < extern_count; j++)
            {
                if (strcmp(extern_table[j], memory[i].label) == 0)
                {
                    is_extern = 1;
                    break;
                }
            }

            if (is_extern)
            {
                /* זו תווית extern, נדפיס אותה עם מרכאות */
                printf("%04d \"%s\"\n", i, memory[i].label);
            }
            else
            {
                /* זו תווית רגילה או entry, נדפיס אותה עם מרכאות */
                printf("%04d \"%s\"\n", i, memory[i].label);
            }
        }
        else
        {
            printf("%04d %s\n", i, memory[i].binary);
        }
    }
}

/* פונקציה לבדיקה אם ההוראה היא עם אופרנד יחיד */
int is_single_operand_instruction(const char *op)
{
    const char *single_operand_instructions[] = {
        "clr", "not", "inc", "dec", "jmp", "bne", "red", "prn", "jsr"};
    int num_instructions = sizeof(single_operand_instructions) / sizeof(single_operand_instructions[0]);

    for (int i = 0; i < num_instructions; i++)
    {
        if (strcmp(op, single_operand_instructions[i]) == 0)
        {
            return 1;
        }
    }
    return 0;
}

/* פונקציה לקידוד אופרנד מיידי */
char* encode_immediate_operand(const char* operand) {
    if (operand == NULL || operand[0] != '#') {
        return NULL;  /* לא אופרנד מיידי */
    }

    /* דילוג על התו '#' */
    const char* number_str = operand + 1;
    
    /* בדיקה אם השאר הוא מספר תקין */
    char* endptr;
    long number = strtol(number_str, &endptr, 10);
    
    /* אם endptr הוא בתחילת המחרוזת, או לא הגיע לסוף,
       אז ההמרה נכשלה או יש תווים שאינם מספריים */
    if (endptr == number_str || *endptr != '\0') {
        return NULL;  /* מספר לא חוקי */
    }

    /* וידוא שהמספר נכנס ב-12 ביטים (מסומן) */
    if (number < -2048 || number > 2047) {
        return NULL;  /* מספר מחוץ לטווח */
    }

    /* הקצאת זיכרון לתוצאה (12 ביטים + 3 ביטים + תו סיום) */
    char* result = (char*)malloc(16 * sizeof(char));
    if (result == NULL) {
        return NULL;  /* כשלון בהקצאת זיכרון */
    }

    /* המרה ל-12 ביט בינארי */
    unsigned short unsigned_number = (unsigned short)(number & 0xFFF);
    for (int i = 11; i >= 0; i--) {
        result[11 - i] = (unsigned_number & (1 << i)) ? '1' : '0';
    }

    /* הוספת "100" */
    strcat(result, "100");
    result[15] = '\0';  /* וידוא סיום מחרוזת */

    return result;
}

/* הפונקציה הראשית */
int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "שימוש: %s <example.as>\n", argv[0]);
        return 1;
    }

    first_pass(argv[1]);
    update_entry_addresses();
    print_memory();
    print_label_table();
    print_entry_table();
    print_extern_table();

    return 0;
}