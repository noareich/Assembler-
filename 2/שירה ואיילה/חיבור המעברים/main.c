#include "assembler.h"

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Error: Not enough arguments, please insert file names.\n");
        return 1;
    }

    for (int i = 1; i < argc; ++i)
    {
        char *filename = argv[i];
        char filenameWithExtension[260];
        char outputFilename[256];
        int macroCount = 0;
        int error = 0;
        Macro *macros;

        addExtension(filename, ".as", filenameWithExtension);

        FILE *file_content = fopen(filenameWithExtension, "r");
        if (!file_content)
        {
            fprintf(stderr, "Failed to read file %s\n", filenameWithExtension);
            continue;
        }
        fclose(file_content);

        addExtension(filename, ".am", outputFilename);

        printf("Reading macros from file: %s\n", filenameWithExtension);
        macros = readMacrosFromFile(filenameWithExtension, &macroCount, &error);
        if (error == 1)
        {
            fprintf(stderr, "Error reading macros from file %s\n", filenameWithExtension);
            freeMacros(macros, macroCount);
            continue;
        }

        printf("Expanding macros in file: %s\n", filenameWithExtension);
        expandMacrosInFile(filenameWithExtension, outputFilename, macros, macroCount, &error);
        if (error == 1)
        {
            fprintf(stderr, "Error expanding macros, deleting output file %s\n", outputFilename);
            freeMacros(macros, macroCount);
            continue;
        }
        freeMacros(macros, macroCount);
        printf("Macros expanded successfully in file: %s\n", outputFilename);

        // חלק של המעבר הראשון והשני
        printf("Initializing assembler state for file: %s\n", outputFilename);
        AssemblerState *state = init_assembler_state();
        if (!state)
        {
            fprintf(stderr, "Failed to initialize assembler state.\n");
            return 1;
        }

        printf("Running first pass on file: %s\n", outputFilename);
        first_pass(state, outputFilename);

        printf("Updating entry addresses...\n");
        update_entry_addresses(state);

        // יצירת שם הקובץ לפלט של המעבר השני עם סיומת ".ob"
        char obFilename[260];
        addExtension(filename, ".ob", obFilename);

        printf("Running second pass on input file: %s, output file: %s\n", outputFilename, obFilename);
        int result = second_pass(state, outputFilename, obFilename);

        if (result != 0)
        {
            fprintf(stderr, "Second pass failed with error code %d\n", result);
            free_assembler_state(state);
            return result;
        }

        printf("Output file '%s' created successfully.\n", obFilename);
        FILE *outputFile = fopen(obFilename, "r");
        if (outputFile)
        {
            char buffer[256];
            while (fgets(buffer, sizeof(buffer), outputFile))
            {
                printf("%s", buffer);
            }
            fclose(outputFile);
        }
        else
        {
            perror("Error opening output file for reading");
        }

        free_assembler_state(state);
        printf("Assembler process finished successfully for file: %s\n", filename);
    }

    return 0;
}