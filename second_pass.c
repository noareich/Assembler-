#include "assembler.h"
#include "check.h"

#define MEMORY_START 100
#define BINARY_LENGTH 16
#define ADDRESS_BINARY_LENGTH 13
#define OCTAL_STRING_LENGTH 8
#define EXTERNAL_BINARY "000000000000001"
#define RELOCATABLE_BINARY_SUFFIX "010"
#define MAX_FILENAME_LENGTH 260
#define ADDRESS_FORMAT "%04d"



/* Function to perform the second pass of the assembler */
void second_pass(AssemblerState *state, const char *input_filename, const char *output_filename, int *error)
{


    FILE *inputFile;
    FILE *outputFile;
    
    char binaryCode[BINARY_LENGTH] = {0};
    char addressBinary[ADDRESS_BINARY_LENGTH] = {0};
    int addressValue;
    int binaryDecimal;
    char octalStr[OCTAL_STRING_LENGTH] = {0};

    int result1 ;
    int result2 ;
    char entFilename[MAX_FILENAME_LENGTH] = "";
    char extFilename[MAX_FILENAME_LENGTH] = "";
    int i, j;
    int is_extern;
    int line_number = 0;  /* Add a line number counter */
    addressValue = -1;
     binaryDecimal = 0;
    result1 = 0;
    result2 = 0;
if (state == NULL || input_filename == NULL || output_filename == NULL || error == NULL) {
    fprintf(stderr, "Error: Invalid input parameters to second_pass\n");
    return;
}
    /* Open input file */

    inputFile = fopen(input_filename, "r");
    if (!inputFile)
    {
        perror("Error opening input file");
        return;
    }

    /* Open output file */
    outputFile = fopen(output_filename, "w");
    if (!outputFile)
    {
        perror("Error creating output file");
        fclose(inputFile);
        return;
    }

    /* Write IC and DC values at the beginning of the output file */
    fprintf(outputFile, "%d %d\n", state->IC - MEMORY_START, state->DC);

    /* Process each memory location */
    for (i = MEMORY_START; i < state->IC + state->DC; i++)
    {
        line_number++;  /* Increment line number */

        if (state->memory[i].label[0] != '\0')
        {
            is_extern = 0;
            for (j = 0; j < state->extern_count; j++)
            {
                if (strcmp(state->extern_table[j].name, state->memory[i].label) == 0)
                {
                    is_extern = 1;
                    break;
                }
            }

            if (is_extern)
            {
strncpy(binaryCode, EXTERNAL_BINARY, BINARY_LENGTH - 1);
binaryCode[BINARY_LENGTH - 1] = '\0';
                binaryDecimal = binaryToDecimal(binaryCode);
                decimalToOctal(binaryDecimal, octalStr);
                fprintf(outputFile, ADDRESS_FORMAT " %s\n", i, octalStr);
            }
            else
            {
                addressValue = get_label_address(state, state->memory[i].label);
                if (addressValue != -1)
                {
                    decimalToBinary12(addressValue, addressBinary);

strncpy(binaryCode, addressBinary, BINARY_LENGTH - 1);
binaryCode[BINARY_LENGTH - 1] = '\0';
strncat(binaryCode, RELOCATABLE_BINARY_SUFFIX, BINARY_LENGTH - strlen(binaryCode) - 1);

                    binaryDecimal = binaryToDecimal(binaryCode);
                    decimalToOctal(binaryDecimal, octalStr);
                    fprintf(outputFile, ADDRESS_FORMAT " %s\n", i, octalStr);
                }
                else
                {
                    fprintf(stderr, "Error at line %d: undefined label %s\n", line_number, state->memory[i].label);
                    *error = 1;
                }
            }
        }
        else
        {
            binaryDecimal = binaryToDecimal(state->memory[i].binary);
            decimalToOctal(binaryDecimal, octalStr);
            fprintf(outputFile, ADDRESS_FORMAT " %s\n", i, octalStr);
        }
    }

    /* Close input and output files */
    fclose(inputFile);
    fclose(outputFile);

    /* Create entry and extern files */
    result1 = createEntryFile(state, input_filename, entFilename);
    result2 = createExternFile(state, input_filename, extFilename);
   
    if(result1 == 1 || result2 == 1)
    {
        remove(entFilename);
        remove(extFilename);
        *error = 1;
    }
}

/* Function to create the entry file */
int createEntryFile(AssemblerState *state, const char *filename, char *entFilename)
{
    int error = 0;
    FILE *entFile;
    int i;

    if (state->entry_count > 0)
    {
        createExtension(filename, entFilename, ".ent");
        entFile = fopen(entFilename, "w");
        if (!entFile)
        {
            perror("Error creating entry file");
            return 1;
        }

        for (i = 0; i < state->entry_count; i++)
        {
            if (is_entry_label_defined(state, state->entry_table[i].name) == 0)
            {
                fprintf(entFile, "%s " ADDRESS_FORMAT "\n", state->entry_table[i].name, state->entry_table[i].address);
            }
            else
            {
                fprintf(stderr, "Error in entry file: label entry %s not found in symbol table\n", state->entry_table[i].name);
                error = 1;
            }
        }
        fclose(entFile);
    }
    
    return error;
}

/* Function to create the extern file */
int createExternFile(AssemblerState *state, const char *filename, char *extFilename)
{
    int error = 0;
    FILE *extFile;
    int i, j;

    if (state->extern_count > 0)
    {
        createExtension(filename, extFilename, ".ext");
        extFile = fopen(extFilename, "w");
        if (!extFile)
        {
            perror("Error creating extern file");
            return 1;
        }

        for (i = 0; i < state->extern_count; i++)
        {
            /* Check if the extern label is also defined as an entry label */
            if (is_extern_label_defined_as_entry(state, state->extern_table[i].name) == 0)
            {
                fprintf(stderr, "Error in extern file: extern label %s is also defined as an entry label\n", state->extern_table[i].name);
                error = 1;
            }
            for (j = MEMORY_START; j < state->IC + state->DC; j++)
            {
                if (strcmp(state->memory[j].label, state->extern_table[i].name) == 0)
                {
                    fprintf(extFile, "%s " ADDRESS_FORMAT "\n", state->extern_table[i].name, j);
                }
            }
        }
        fclose(extFile);
    }
    
    return error;
}
