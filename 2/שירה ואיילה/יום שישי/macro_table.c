#include "assembler.h"

// Function to read macros from a file and insert them into a table
Macro* readMacrosFromFile(const char* filename, int* macroCount, int* error) {
    FILE* file; /* File pointer */
    Macro* macros = NULL; /* Pointer to macro table */
    char line[100]; /* Array to store a line from the file */
    int inMacro = 0; /* Flag indicating if we are inside a macro */
    char macroName[32]; /* Array to store the macro name */
    size_t contentSize = 256; /* Initial size for macro content */
    char* macroContent = malloc(contentSize); /* Dynamic allocation for macro content */
    size_t nameArraySize = 10; /* Initial size for macro names array */
    char** macroNames = malloc(nameArraySize * sizeof(char*)); /* Dynamic allocation for macro names array */
    int macroNamesCount = 0; /* Counter for macro names */
    char* remaining; /* Pointer to the remaining part of the line */
    size_t lineLen; /* Length of the line */
    size_t contentLen; /* Length of the current content */
    int i; /* Variable for loop counting */

    if (!macroContent) {
        perror("Failed to allocate memory for macro content"); /* Print error message if memory allocation fails */
        *error = 1; 
        return NULL; 
    }
    macroContent[0] = '\0'; /* Initialize macro content to empty */

    if (!macroNames) {
        perror("Failed to allocate memory for macro names"); /* Print error message if memory allocation fails */
        *error = 1; 
        free(macroContent);
        return NULL;
    }

    file = fopen(filename, "r"); /* Open the file for reading */
    if (file == NULL) {
        perror("Failed to open file"); /* Print error message if file cannot be opened */
        *error = 1; 
        return NULL; 
    }

    *macroCount = 0; /* Initialize macro count */

    while (fgets(line, sizeof(line), file)) 
    {
        trimLeadingWhitespace(line); /* Remove leading whitespace from the line */
        
        if (strncmp(line, "macr", 4) == 0) /* Found the start of a macro */
        {
            /* Check if the macro name is valid and if there are extra characters after the macro name */
            remaining = line + 4; 
            while (isspace(*remaining)) remaining++;
            if (sscanf(remaining, "%s", macroName) != 1 || !is_whitespace_line(remaining + strlen(macroName))) {
                fprintf(stderr, "Error: Invalid macro definition line '%s'\n", line); 
                *error = 1; 
            }

            /* Check if the macro name is too long */
            if (strlen(macroName) > MAX_MACRO_NAME_LENGTH) {
                fprintf(stderr, "Error: Macro name '%s' exceeds maximum length of 31 characters\n", macroName);
                *error = 1; 
            }

            inMacro = 1; /* Indicate that we are inside a macro */

            /* Check for duplicate macro names */
            for (i = 0; i < macroNamesCount; i++) {
                if (strcmp(macroNames[i], macroName) == 0) {
                    fprintf(stderr, "Error: Duplicate macro name '%s'\n", macroName); 
                    *error = 1; 
                }
            }

            /* If needed, increase the size of the dynamic array of macro names */
            if (macroNamesCount >= nameArraySize) {
                nameArraySize *= 2; 
                macroNames = realloc(macroNames, nameArraySize * sizeof(char*));
                if (!macroNames) {
                    perror("Failed to reallocate memory for macro names"); 
                    *error = 1; 
                    free(macroContent); 
                    fclose(file); 
                    return macros; 
                }
            }

            /* Copy the macro name to the macro names array */
            macroNames[macroNamesCount++] = strdup(macroName);
            macroContent[0] = '\0'; 
        }
        else if (inMacro && strncmp(line, "endmacr", 7) == 0) 
        {
            /* Check if there are extra characters beyond whitespace after the end of the macro */
            remaining = line + 7; 
            while (isspace(*remaining)) remaining++; 
            if (*remaining != '\0') {
                fprintf(stderr, "Error: Invalid endmacro line '%s'\n", line); 
                *error = 1; 
            }

            /* Expand memory for the new macro, copy the macro name and content to the macro table, and update the macro count */
            inMacro = 0; /* Indicate that we are outside the macro */
            macros = realloc(macros, sizeof(Macro) * (*macroCount + 1)); 
            macros[*macroCount].name = strdup(macroName); 
            macros[*macroCount].content = strdup(macroContent); 
            (*macroCount)++; 
        }
        else if (inMacro) {
            /* Inside a macro, add the line to the content if it is not empty */
            if (!is_empty_macro_line(line)) {
                lineLen = strlen(line); 
                contentLen = strlen(macroContent); 

                if (contentLen + lineLen + 1 >= contentSize) {
                    contentSize *= 2; 
                    macroContent = realloc(macroContent, contentSize); 
                    if (!macroContent) {
                        perror("Failed to reallocate memory for macro content"); 
                        *error = 1; 
                        for (i = 0; i < macroNamesCount; i++) {
                            free(macroNames[i]);
                        }
                        free(macroNames); 
                        fclose(file); 
                        return macros; 
                    }
                }

                strcat(macroContent, line); /* Append the line to the macro content */
            }
        }
    }

    for (i = 0; i < macroNamesCount; i++) {
        free(macroNames[i]);
    }
    free(macroNames); /* Free memory allocated for macro names */
    free(macroContent); /* Free memory allocated for macro content */
    fclose(file); /* Close the file */
    return macros; /* Return the macro table */
}