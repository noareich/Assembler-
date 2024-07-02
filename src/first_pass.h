#ifndef FILE_READER_H
#define FILE_READER_H

//קובץ הכותרת מכיל את ההצהרות על המבנה (struct) והפונקציות כך שניתן יהיה להשתמש בהן בקבצים אחרים:// 


// הגדרת מבנה נתונים לשמירת שורות הקובץ
typedef struct Line {
    char* content; // תוכן השורה
    struct Line* next; // מצביע לשורה הבאה
} Line;//Line - מבנה נתונים לשמירת שורות הקובץ ברשימה מקושרת
int write_to_new_file(const char* filename, Line* head);
// הצהרות על הפונקציות המוגדרות בקובץ file_reader.c
Line* createLine(const char* content);
void freeLines(Line* head);
Line* read_preprocessed_file(const char* filename);

#endif
