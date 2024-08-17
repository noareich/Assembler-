#include "assembler.h"

/*פונקציה כדי שיעבוד לי בויזואל סטודיו ה strtok_r*/
char *strtok_r(char *str, const char *delim, char **saveptr) {
    char *token;

    if (str == NULL) {
        str = *saveptr;
    }

    // קפיצה על כל תווי ההפרדה בהתחלה
    str += strspn(str, delim);
    if (*str == '\0') {
        return NULL;
    }

    // חיפוש סוף ה-token
    token = str;
    str = strpbrk(token, delim);
    if (str == NULL) {
        *saveptr = strchr(token, '\0');
    } else {
        *str = '\0';
        *saveptr = str + 1;
    }

    return token;
}
/*פונקציה שיוצרת סיומת חדשה לקובץ פלט*/
void addExtension(char* filename, const char* extension, char* result) {
    strcpy(result, filename);
    strcat(result, extension);
}
/*מסיר תווים לבנים מתחילת שורה*/
void trimLeadingWhitespace(char *str) {
    char *start = str;
    int is_comment = 0;
    //int *erorr=0;

    // הסרת תווים לבנים בתחילת השורה
    while (isspace((unsigned char)*start)) {
        start++;
    }

    // בדוק אם השורה היא שורת הערה
    if (*start == ';') {
        is_comment = 1;
    }

    if (is_comment) {
        // אם זו שורת הערה והיו תווים לבנים לפני הסימן ";", הצג הודעת שגיאה
        if (str != start) {
            printf("Error: Leading whitespace before comment on line: %s\n", str);
            //*erorr=1;
        }
    } else {
        // אם לא מדובר בשורת הערה, העבר את התוכן הנותר לשם המקורי
        if (start != str) {
            memmove(str, start, strlen(start) + 1);
        }
    }
}
bool has_leading_whitespace(const char *line) {
    // Loop through the line until we encounter a non-whitespace character or end of string
    while (*line != '\0' && isspace((unsigned char)*line)) {
        line++;
    }

    // If we encountered a non-whitespace character, then the line had leading whitespace
    return *line != '\0';
}
/***************************************************************
 *פונקציות שקשורות לבניית טבלת המקוראים - קובץ macro_table.c*
/***************************************************************/

// פונקציה לשחרור הזיכרון שהוקצה לטבלה
void freeMacros(Macro* macros, int macroCount) {
    for (int i = 0; i < macroCount; i++) {
        free(macros[i].name);
        free(macros[i].content);
    }
    free(macros);
}
// פונקציה שבודקת אם השורה היא הערה
int is_comment(const char* line) {
        char* trim_line = strdup(line);
    trim_line = strtok(trim_line, " \t\n");
    if (trim_line != NULL && trim_line[0] == ';') {
        free(trim_line);
        return 1; /* השורה היא הערה */
    }
    free(trim_line);
    return 0; /* השורה אינה הערה */
}
// פונקציה שבודקת אם השורה ריקה
int is_empty_line(const char* line) {
        char* trim_line = strdup(line);
    trim_line = strtok(trim_line, " \t\n");
    if (trim_line == NULL) {
        free(trim_line);
        return 1; /* השורה היא ריקה */
    }
    free(trim_line);
    return 0; /* השורה אינה ריקה */
}
// פונקציה שבודקת אם השורה מכילה רק תווים לבנים
int is_whitespace_line(const char* line) {
    while (*line) {
        if (!isspace(*line)) {
            return 0; // מצאנו תו שאינו רווח
        }
        line++;
    }
    return 1; // כל השורה מכילה רק תווים לבנים
}
// פונקציה שבודקת אם השורה בתוך מקרו ריקה או מכילה רק תווים לבנים
int is_empty_macro_line(const char* line) {
    return is_whitespace_line(line);
}
/**************************************
 **פונקציות שכרגע אני לא משתמשת בהן**
/**************************************/
//פונקציה שבודקת אם המילה היא מילה שמורה
int reserved_words(char* str) {
    int i;
    for (i = 0; i < RESERVED_WORD_NUM; i++) {
        if (strcmp(strings[i], str) == 0)
            return 1; /* המחרוזת קיימת במערך */
    }
    return 0; /* המחרוזת לא קיימת במערך */
}
// פונקציה שבודקת אם המילה היא שם של רשומה
int is_register(char* str) {
    int register_index = 23;  // אינדקס התחלת הרשומות
    int i;
    for (i = register_index; i < RESERVED_WORD_NUM; i++) {
        if (strcmp(strings[i], str) == 0)
            return 1; /* המחרוזה היא שם רשומה */
   }
    return 0; /* המחרוזה לא שם רשומה */
}