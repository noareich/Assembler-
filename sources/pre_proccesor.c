#include "assembler.h"

#define MAX_LINE_SIZE 256
#define MACRO_START "macr"
#define MACRO_END "endmacr"
#define MACRO_START_LENGTH 4
#define MACRO_END_LENGTH 7

 

/* Function to expand macros in the input file */
void expandMacrosInFile(const char* inputFilename, char* outputFilename, Macro* macros, int macroCount, int* error) {
    FILE* inputFile;
    FILE* outputFile;
    char line[MAX_LINE_SIZE];
    int lineNumber = 0;
    int replaced;
    char macroName[MAX_LINE_SIZE];
    char* remaining;
    int i, j;
    char* macroLine;
    char* macroLinePtr;
    char* endOfLine;
    char* remaining_end;

    /* Open input file for reading */

    inputFile = fopen(inputFilename, "r");
    if (inputFile == NULL) 
    {
        fprintf(stderr, "Error at line %d: Failed to open input file\n", lineNumber);
        perror("Error details");
        *error = 1;
        return; 
    }

    /* Open output file for writing */
    outputFile = fopen(outputFilename, "w");
    if (outputFile == NULL)
 {
        fprintf(stderr, "Error at line %d: Failed to create output file\n", lineNumber);
        perror("Error details");
        fclose(inputFile);
        *error = 1;
        return; 
    }

    /* Process each line in the input file */
    while (fgets(line, sizeof(line), inputFile))
    {
        trimLeadingWhitespace(line);
        lineNumber++;
        
       

        /* Write comments and empty lines directly to output file */
        if (is_comment(line) || is_empty_line(line)) 
	{
            fputs(line, outputFile);
            continue;
        }

        replaced = 0;

        /* Check if line starts with macro definition */
        if (strncmp(line, MACRO_START, MACRO_START_LENGTH) == 0) 
	{
            remaining = line + MACRO_START_LENGTH;
            while (isspace(*remaining)) remaining++;
            if (sscanf(remaining, "%s", macroName) != 1 || !is_whitespace_line(remaining + strlen(macroName)))
	 {
                fprintf(stderr, "Error at line %d: Invalid macro definition line '%s'\n", lineNumber, line);
                *error = 1;
            }

            /* Replace macro with its content */
            for (j = 0; j < macroCount; j++) {
                if (strcmp(macroName, macros[j].name) == 0) 
		{
                    macroLine = my_strdup(macros[j].content);
                    macroLinePtr = macroLine;
                    while (*macroLinePtr) {
                        endOfLine = strchr(macroLinePtr, '\n');
                        if (endOfLine) *endOfLine = '\0';
                        if (!is_empty_macro_line(macroLinePtr)) 
			{
                            fputs(macroLinePtr, outputFile);

 
                                fputs("\n", outputFile);
                            
                        }
                        if (!endOfLine) break;
                        macroLinePtr = endOfLine + 1;
                    }
                    free(macroLine);
                    replaced = 1;
                }
            }

            /* Skip lines until end of macro definition */
            while (fgets(line, sizeof(line), inputFile))
            {
                lineNumber++;
                trimLeadingWhitespace(line);
                if (strncmp(line, MACRO_END, MACRO_END_LENGTH) == 0)
		 {
                    break;
                }
            }
            remaining_end = line + MACRO_END_LENGTH;
            while (isspace(*remaining_end)) remaining_end++;
            if (*remaining_end != '\0')
		 {
                fprintf(stderr, "Error at line %d: Invalid endmacro line '%s'\n", lineNumber, line);
                *error = 1;
            }
        }
	 else
	 {
            /* Check if line contains a macro call and replace it */
            for (i = 0; i < macroCount; i++) 
	{
                if (strncmp(line, macros[i].name, strlen(macros[i].name)) == 0)
	 {
                    macroLine = my_strdup(macros[i].content);
                    macroLinePtr = macroLine;
                    while (*macroLinePtr) {
                        endOfLine = strchr(macroLinePtr, '\n');
                        if (endOfLine) *endOfLine = '\0';
                        if (!is_empty_macro_line(macroLinePtr))
			 {
                            fputs(macroLinePtr, outputFile);
                                 fputs("\n", outputFile);
                            
                        }
                        if (!endOfLine) break;
                        macroLinePtr = endOfLine + 1;
                    }
                    free(macroLine);
                    fputs(line + strlen(macros[i].name), outputFile);
                    replaced = 1;
                }
            }
        }

        /* Write original line if no replacement occurred */
        if (!replaced) {
            fputs(line, outputFile);
        }
    }

    /* Close input and output files */
    fclose(inputFile);
    fclose(outputFile);
}
