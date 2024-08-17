/*פונקציה שמצעת את פרישת המקרואים*/
#include "assembler.h"

void expandMacrosInFile(const char* inputFilename, const char* outputFilename, Macro* macros, int macroCount, int* error) {
    FILE* inputFile;  /* קובץ קלט לקריאה */
    FILE* outputFile; /* קובץ פלט לכתיבה */
    char line[256]; /* מערך לאחסון שורה מהקובץ */
    int lineNumber = 0; /* אתחול מספר השורות */
    int replaced; /* דגל שמציין אם בוצעה החלפה */
    char macroName[256]; /* מערך לאחסון שם המקרו */
    char* remaining; /* הצבעה לשארית השורה אחרי "macr" */
    int i, j; /* משתנים לאיטרציה על מקרואים */
    char* macroLine; /* שכפול תוכן המקרו */
    char* macroLinePtr; /* מצביע לתחילת תוכן המקרו */
    char* endOfLine; /* מציאת סוף השורה */
    char* remaining_end; /* הצבעה לשארית השורה אחרי "endmacr" */

    inputFile = fopen(inputFilename, "r"); /* פתיחת קובץ הקלט לקריאה */
    if (inputFile == NULL) {
        perror("Failed to open input file"); /* הדפסת הודעת שגיאה אם קובץ הקלט לא נפתח */
        *error = 1; /* סימון שיש שגיאה */
        return; 
    }

    outputFile = fopen(outputFilename, "w"); /* פתיחת קובץ הפלט לכתיבה */
    if (outputFile == NULL) {
        perror("Failed to create output file"); /* הדפסת הודעת שגיאה אם קובץ הפלט לא נפתח */
        fclose(inputFile); /* סגירת קובץ הקלט */
        *error = 1; /* סימון שיש שגיאה */
        return; 
    }
    while (fgets(line, sizeof(line), inputFile)) /* קריאת שורה מהקובץ */
    {
        trimLeadingWhitespace(line); /* הסרת רווחים מובילים מהשורה */
        lineNumber++; /* עדכון מספר השורות */
        if (strlen(line) > MAX_LINE_LENGTH) {
            fprintf(stderr, "Error: Line %d exceeds maximum length of %d characters\n", lineNumber, MAX_LINE_LENGTH); /* הודעת שגיאה אם השורה ארוכה מדי */
            *error = 1; /* סימון שיש שגיאה */
        }

        if (is_comment(line) || is_empty_line(line)) {
            fputs(line, outputFile); /* כתיבת שורה ריקה או שורת הערה לקובץ הפלט */
            continue; /* מעבר לשורה הבאה */
        }

        replaced = 0; /* דגל שמציין אם בוצעה החלפה */

        if (strncmp(line, "macr", 4) == 0) { /* בדיקה אם השורה מתחילה במקרו */
            remaining = line + 4; /* הצבעה לשארית השורה אחרי "macr" */
            while (isspace(*remaining)) remaining++; /* דילוג על רווחים */
            if (sscanf(remaining, "%s", macroName) != 1 || !is_whitespace_line(remaining + strlen(macroName))) {
                fprintf(stderr, "Error: Invalid macro definition line '%s' at line %d\n", line, lineNumber); /* הודעת שגיאה אם השורה לא תקינה */
                *error = 1; /* סימון שיש שגיאה */
            }

            for (j = 0; j < macroCount; j++) {
                if (strcmp(macroName, macros[j].name) == 0) {
                    macroLine = strdup(macros[j].content); /* שכפול תוכן המקרו */
                    macroLinePtr = macroLine; /* מצביע לתחילת תוכן המקרו */
                    while (*macroLinePtr) {
                        endOfLine = strchr(macroLinePtr, '\n'); /* מציאת סוף השורה */
                        if (endOfLine) *endOfLine = '\0'; /* סיום השורה בנקודה זו */
                        if (!is_empty_macro_line(macroLinePtr)) {
                            fputs(macroLinePtr, outputFile); /* כתיבת תוכן המקרו לקובץ הפלט */
                            if (*(endOfLine + 1) != '\0') { /* הוספת שורת חדשה רק אם לא מדובר בשורה האחרונה */
                                fputs("\n", outputFile);
                            }
                        }
                        if (!endOfLine) break; /* אם לא נמצאה סוף שורה, יציאה מהלולאה */
                        macroLinePtr = endOfLine + 1; /* מעבר לשורה הבאה בתוכן המקרו */
                    }
                    free(macroLine); /* שחרור הזיכרון של המקרו */
                    replaced = 1; /* סימון שהחלפנו את המקרו */
                }
            }

            while (fgets(line, sizeof(line), inputFile)) /* קריאת השורות עד לסוף המקרו */
            {
                lineNumber++; /* עדכון מספר השורות */
                trimLeadingWhitespace(line); /* הסרת רווחים מובילים מהשורה */
                if (strncmp(line, "endmacr", 7) == 0) {
                    break; /* יציאה מהלולאה כאשר מוצאים "endmacr" */
                }
            }
            remaining_end = line + 7; /* הצבעה לשארית השורה אחרי "endmacr" */
            while (isspace(*remaining_end)) remaining_end++; /* דילוג על רווחים */
            if (*remaining_end != '\0') {
                fprintf(stderr, "Error: Invalid endmacro line '%s' at line %d\n", line, lineNumber); /* הודעת שגיאה אם השורה לא תקינה */
                *error = 1; /* סימון שיש שגיאה */
            }
        } else {
            for (i = 0; i < macroCount; i++) {
                if (strncmp(line, macros[i].name, strlen(macros[i].name)) == 0) {
                    macroLine = strdup(macros[i].content); /* שכפול תוכן המקרו */
                    macroLinePtr = macroLine; /* מצביע לתחילת תוכן המקרו */
                    while (*macroLinePtr) {
                        endOfLine = strchr(macroLinePtr, '\n'); /* מציאת סוף השורה */
                        if (endOfLine) *endOfLine = '\0'; /* סיום השורה בנקודה זו */
                        if (!is_empty_macro_line(macroLinePtr)) {
                            fputs(macroLinePtr, outputFile); /* כתיבת תוכן המקרו לקובץ הפלט */
                            if (*(endOfLine + 1) != '\0') { /* הוספת שורת חדשה רק אם לא מדובר בשורה האחרונה */
                                fputs("\n", outputFile);
                            }
                        }
                        if (!endOfLine) break; /* אם לא נמצאה סוף שורה, יציאה מהלולאה */
                        macroLinePtr = endOfLine + 1; /* מעבר לשורה הבאה בתוכן המקרו */
                    }
                    free(macroLine); /* שחרור הזיכרון של המקרו */
                    fputs(line + strlen(macros[i].name), outputFile); /* כתיבת השורה המקורית אחרי שם המקרו */
                    replaced = 1; /* סימון שהחלפנו את המקרו */
                }
            }
        }

        if (!replaced) {
            fputs(line, outputFile); /* כתיבת השורה המקורית אם לא בוצעה החלפה */
        }
    }

    fclose(inputFile); /* סגירת קובץ הקלט */
    fclose(outputFile); /* סגירת קובץ הפלט */

}