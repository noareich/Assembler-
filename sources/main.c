#include "assembler.h"
#include "check.h"

#define MAX_FILENAME_LENGTH 260
#define MIN_ARGUMENTS 2


/* Main function: Entry point of the assembler program */
int main(int argc, char *argv[])
{
    char *filename;
    char filenameWithExtension[MAX_FILENAME_LENGTH];
    char outputFilename[MAX_FILENAME_LENGTH];
    char obFilename[MAX_FILENAME_LENGTH];
    char entFilename[MAX_FILENAME_LENGTH];
    char extFilename[MAX_FILENAME_LENGTH];
    int macroCount;
    int error;
    Macro *macros;
    AssemblerState *state;
    int i;
    FILE *file_content;

    /* Check if enough arguments are provided */
    if (argc < MIN_ARGUMENTS)
    {
        fprintf(stderr, "Error: Not enough arguments, please insert file names.\n");
        return 1;
    }

    /* Process each input file */
    for (i = 1; i < argc; ++i)
    {
        filename = argv[i];
        macroCount = 0;
        error = 0;

        /* Add .as extension to the input filename */
        addExtension(filename, ".as", filenameWithExtension);

        /* Check if the input file exists */
        file_content = fopen(filenameWithExtension, "r");
        if (!file_content)
        {
            fprintf(stderr, "Error: Failed to read file %s. Make sure the file exists and you have the necessary permissions.\n", filenameWithExtension);
            continue;
        }
        fclose(file_content);

        /* Add .am extension to the output filename */
        addExtension(filename, ".am", outputFilename);

        /* Read macros from the input file */
        printf("Reading macros from file: %s\n", filenameWithExtension);
        macros = readMacrosFromFile(filenameWithExtension, &macroCount, &error);
        if (error == 1)
        {
            fprintf(stderr, "Error: Failed to read macros from file %s. Check the file for syntax errors in macro definitions.\n", filenameWithExtension);
            freeMacros(macros, macroCount);
            continue;
        }

        /* Expand macros in the input file */
        printf("Expanding macros in file: %s\n", filenameWithExtension);
        expandMacrosInFile(filenameWithExtension, outputFilename, macros, macroCount, &error);
        if (error == 1)
        {
            fprintf(stderr, "Error: Failed to expand macros in file %s. Check the file for invalid macro usage.\n", filenameWithExtension);
            freeMacros(macros, macroCount);
            remove(outputFilename);
            continue;
        }
        freeMacros(macros, macroCount);
        printf("Macros expanded successfully in file: %s\n", outputFilename);

        /* Initialize assembler state */
        printf("Initializing assembler state for file: %s\n", outputFilename);
        state = init_assembler_state();
        if (!state)
        {
            fprintf(stderr, "Error: Failed to initialize assembler state. This might be due to memory allocation issues.\n");
            return 1;
        }

        /* Run first pass */
        printf("Running first pass on file: %s\n", outputFilename);
        first_pass(state, outputFilename, macros, &macroCount, &error);
        if (error == 1)
        {
            fprintf(stderr, "Error: First pass failed for file %s. Check the file for syntax errors or invalid instructions.\n", outputFilename);
            free_assembler_state(state);
            continue;
        }

        /* Update entry addresses */
        printf("Updating entry addresses...\n");
        update_entry_addresses(state);

        /* Create output filenames */
        addExtension(filename, ".ob", obFilename);
        entFilename[0] = '\0';
        extFilename[0] = '\0';

        /* Run second pass */
        printf("Running second pass on input file: %s, output file: %s\n", outputFilename, obFilename);
        second_pass(state, outputFilename, obFilename, &error);
        if (error == 1)
        {
            fprintf(stderr, "Error: Second pass failed for file %s. This might be due to unresolved symbols or other assembly errors.\n", outputFilename);
            remove(obFilename);
            remove(entFilename);
            remove(extFilename);
            free_assembler_state(state);
            continue;
        }

        /* Clean up and finish */
        free_assembler_state(state);
        state = NULL;
        printf("Assembler process finished successfully for file: %s\n", filename);
    }

    return 0;
}
