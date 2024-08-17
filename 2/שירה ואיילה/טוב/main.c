#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// הכללת הכותרות של שני המעברים
#include "first_pass.h"
#include "second_pass.h"

#define MAX_FILENAME 100

int main(int argc, char *argv[]) {
    char input_filename[MAX_FILENAME];
    char output_filename[MAX_FILENAME];
    
    if (argc != 2) {
        fprintf(stderr, "שימוש: %s <example.as>\n", argv[0]);
        return 1;
    }
    
    strncpy(input_filename, argv[1], MAX_FILENAME);
    input_filename[MAX_FILENAME - 1] = '\0';
    
    // ביצוע המעבר הראשון
    if (first_pass(input_filename) != 0) {
        fprintf(stderr, "שגיאה במעבר הראשון\n");
        return 1;
    }
    
    // יצירת שם קובץ הפלט למעבר השני
    snprintf(output_filename, MAX_FILENAME, "%s.ob", input_filename);
    
    // ביצוע המעבר השני
    if (second_pass(input_filename, output_filename) != 0) {
        fprintf(stderr, "שגיאה במעבר השני\n");
        return 1;
    }
    
    printf("האסמבלי הושלם בהצלחה. קובץ הפלט: %s\n", output_filename);
    
    return 0;
}