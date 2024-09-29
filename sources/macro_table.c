#include "assembler.h"

#define MAX_LINE_LENGTH 100
#define INITIAL_CONTENT_SIZE 256
#define INITIAL_NAME_ARRAY_SIZE 10
#define MACRO_START "macr"
#define MACRO_END "endmacr"
#define MACRO_START_LENGTH 4
#define MACRO_END_LENGTH 7



/* Function to read macros from a file and insert them into a table */
Macro* readMacrosFromFile(const char* filename, int* macroCount, int* error) 
{
    FILE* file;
    Macro* macros = NULL;
    char line[MAX_LINE_LENGTH];
    int inMacro = 0;
    char macroName[MAX_MACRO_NAME_LENGTH + 1];
    size_t contentSize = INITIAL_CONTENT_SIZE;
    char* macroContent;
    size_t nameArraySize = INITIAL_NAME_ARRAY_SIZE;
    char** macroNames;
    int macroNamesCount = 0;
    char* remaining;
    size_t lineLen;
    size_t contentLen;
    int i;
    int lineNumber = 0;  /* Add a line counter */

    /* Allocate memory for macro content */
    macroContent = malloc(contentSize);
    if (!macroContent) 
	{
        fprintf(stderr, "Error at line %d: Failed to allocate memory for macro content\n", lineNumber);
        *error = 1; 
        return NULL; 
  	  }
    macroContent[0] = '\0';

    /* Allocate memory for macro names array */
    macroNames = malloc(nameArraySize * sizeof(char*));
    if (!macroNames) 
	{
        fprintf(stderr, "Error at line %d: Failed to allocate memory for macro names\n", lineNumber);
        *error = 1; 
        free(macroContent);
        return NULL;
    }

    /* Open the file for reading */
    file = fopen(filename, "r");
    if (file == NULL)
	 {
        fprintf(stderr, "Error: Failed to open file %s\n", filename);
        *error = 1; 
        return NULL; 
    }

    *macroCount = 0;

    /* Read the file line by line */
    while (fgets(line, sizeof(line), file)) 
    {
        lineNumber++;  /* Increment line counter */
        trimLeadingWhitespace(line);
        
        if (strncmp(line, MACRO_START, MACRO_START_LENGTH) == 0)
        {
            /* Found the start of a macro */
            remaining = line + MACRO_START_LENGTH; 
            while (isspace(*remaining)) remaining++;
            if (sscanf(remaining, "%s", macroName) != 1 || !is_whitespace_line(remaining + strlen(macroName)))
 		{
                fprintf(stderr, "Error at line %d: Invalid macro definition line '%s'\n", lineNumber, line); 
                *error = 1; 
            }

            if (strlen(macroName) > MAX_MACRO_NAME_LENGTH) 
		{
                fprintf(stderr, "Error at line %d: Macro name '%s' exceeds maximum length of 31 characters\n", lineNumber, macroName);
                *error = 1; 
            }

            inMacro = 1;

            /* Check for duplicate macro names */
            for (i = 0; i < macroNamesCount; i++)
	 	{
                if (strcmp(macroNames[i], macroName) == 0) 
		{
                    fprintf(stderr, "Error at line %d: Duplicate macro name '%s'\n", lineNumber, macroName); 
                    *error = 1; 
                }
            }

            /* Increase the size of the macro names array if needed */
            if (macroNamesCount >= nameArraySize)
 		{
                nameArraySize *= 2; 
                macroNames = realloc(macroNames, nameArraySize * sizeof(char*));
                if (!macroNames) 
		{
                    fprintf(stderr, "Error at line %d: Failed to reallocate memory for macro names\n", lineNumber); 
                    *error = 1; 
                    free(macroContent); 
                    fclose(file); 
                    return macros; 
                }
            }

            macroNames[macroNamesCount++] = my_strdup(macroName);
            macroContent[0] = '\0'; 
        }
        else if (inMacro && strncmp(line, MACRO_END, MACRO_END_LENGTH) == 0) 
        {
            /* Found the end of a macro */
            remaining = line + MACRO_END_LENGTH; 
            while (isspace(*remaining)) remaining++; 
            if (*remaining != '\0') 
		{
                fprintf(stderr, "Error at line %d: Invalid endmacro line '%s'\n", lineNumber, line); 
                *error = 1; 
            }

            inMacro = 0;
            macros = realloc(macros, sizeof(Macro) * (*macroCount + 1)); 
            macros[*macroCount].name = my_strdup(macroName); 
            macros[*macroCount].content = my_strdup(macroContent); 
            (*macroCount)++; 
        }
        else if (inMacro)
		 {
            /* Inside a macro, add the line to the content if it is not empty */
            if (!is_empty_macro_line(line)) 
		{
                lineLen = strlen(line); 
                contentLen = strlen(macroContent); 

                if (contentLen + lineLen + 1 >= contentSize)
		 {
                    contentSize *= 2; 
                    macroContent = realloc(macroContent, contentSize); 
                    if (!macroContent) {
                        fprintf(stderr, "Error at line %d: Failed to reallocate memory for macro content\n", lineNumber); 
                        *error = 1; 
                        for (i = 0; i < macroNamesCount; i++) 
		{
                            free(macroNames[i]);
                        }
                        free(macroNames); 
                        fclose(file); 
                        return macros; 
                    }
                }

                strcat(macroContent, line);
            }
        }
    }

    /* Clean up */
    for (i = 0; i < macroNamesCount; i++) 
	{
        free(macroNames[i]);
    }
    free(macroNames);
    free(macroContent);
    fclose(file);
    return macros;
}
