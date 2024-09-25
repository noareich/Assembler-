#include "assembler.h"
#ifndef CHECK_H
#define CHECK_H

/*
 * This header file contains declarations for various checking and validation functions
 * used in the assembler implementation. These functions are responsible for ensuring
 * the integrity and correctness of the assembly code during both the first and second passes.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

/* General Functions */

/**
 * @brief Checks if a line exceeds the maximum allowed length.
 *
 * @param line The line to be checked.
 * @return 1 if the line length is valid, 0 otherwise.
 */
int check_line_length(const char* line);

/**
 * @brief Checks if a line is empty or contains only whitespace.
 *
 * @param line The line to be checked.
 * @return 1 if the line is empty or contains only whitespace, 0 otherwise.
 */
int is_empty_or_whitespace(const char* line);

/**
 * @brief Checks if a line is a valid comment.
 *
 * @param line The line to be checked.
 * @return 1 if the line is a valid comment, 0 otherwise.
 */
int is_valid_comment(const char* line);

/* First Pass Validation Functions */

/**
 * @brief Checks if a word is in a given set of words.
 *
 * @param word The word to check.
 * @param set An array of words to check against.
 * @param setSize The size of the set array.
 * @return 1 if the word is in the set, 0 otherwise.
 */
int isInSet(const char *word, const char *set[], int setSize);

/**
 * @brief Checks if a word is a valid label.
 *
 * @param word The word to check.
 * @param macros An array of Macro structures.
 * @param macroCount A pointer to the number of macros.
 * @return 1 if the word is a valid label, 0 otherwise.
 */
int isValidLabel(const char *word, Macro *macros, int *macroCount);

/**
 * @brief Checks the integrity of data operands.
 *
 * @param operands The operands to check.
 * @return 1 if the data operands are valid, 0 otherwise.
 */
int data_intergity_check(char *operands);

/**
 * @brief Checks if a string contains invalid characters.
 *
 * @param str The string to check.
 * @return 1 if the string contains invalid characters, 0 otherwise.
 */
int has_invalid_characters(const char *str);

/**
 * @brief Checks the integrity of string operands.
 *
 * @param operands The operands to check.
 * @return 1 if the string operands are valid, 0 otherwise.
 */
int string_intergity_check(char *operands);

/**
 * @brief Checks the integrity of entry directives.
 *
 * @param line The line containing the entry directive.
 * @param state The current assembler state.
 * @param macros An array of Macro structures.
 * @param macroCount A pointer to the number of macros.
 * @return 1 if the entry directive is valid, 0 otherwise.
 */
int entry_intergity_check(char *line, AssemblerState* state, Macro *macros, int *macroCount);

/**
 * @brief Checks the integrity of extern directives.
 *
 * @param line The line containing the extern directive.
 * @param state The current assembler state.
 * @return 1 if the extern directive is valid, 0 otherwise.
 */
int extern_intergity_check(char *line, AssemblerState* state);

/**
 * @brief Checks if a word is a reserved word in the assembly language.
 *
 * @param word The word to check.
 * @return 1 if the word is a reserved word, 0 otherwise.
 */
int is_reserved_word(const char* word);

/**
 * @brief Checks if a label is already defined in the assembler state.
 *
 * @param label The label to check.
 * @param state The current assembler state.
 * @return 1 if the label is a duplicate, 0 otherwise.
 */
int is_duplicate_label(const char* label, const AssemblerState* state);

/**
 * @brief Checks if an instruction is valid.
 *
 * @param instruction The instruction to check.
 * @return 1 if the instruction is valid, 0 otherwise.
 */
int is_valid_instruction(const char* instruction);

/**
 * @brief Checks if an operand is a valid immediate value.
 *
 * @param operand The operand to check.
 * @return 1 if the operand is a valid immediate value, 0 otherwise.
 */
int is_valid_immediate(const char* operand);

/**
 * @brief Checks if the number of operands is correct for a given instruction.
 *
 * @param instruction The instruction to check.
 * @param operand_count The number of operands.
 * @return 1 if the operand count is correct, 0 otherwise.
 */
int check_operand_count(const char* instruction, int operand_count);

/**
 * @brief Checks if an operand is valid.
 *
 * @param operand The operand to check.
 * @param macros An array of Macro structures.
 * @param macroCount A pointer to the number of macros.
 * @return 1 if the operand is valid, 0 otherwise.
 */
int is_valid_operand(const char* operand, Macro *macros, int *macroCount);

/**
 * @brief Checks if the addressing mode is valid for a given instruction and operand.
 *
 * @param instruction The instruction.
 * @param operand The operand.
 * @param is_source 1 if the operand is a source operand, 0 if it's a destination operand.
 * @return 1 if the addressing mode is valid, 0 otherwise.
 */
int is_valid_addressing_mode(const char* instruction, const char* operand, int is_source);

/* Second Pass Validation Functions */

/**
 * @brief Checks if a label exists in the assembler state.
 *
 * @param state The current assembler state.
 * @param name The name of the label to check.
 * @return 1 if the label exists, 0 otherwise.
 */
int label_exists(const AssemblerState* state, const char* name);

/**
 * @brief Checks if a label is valid for use as an entry label.
 *
 * @param label The label to check.
 * @return 1 if the label is valid for entry, 0 otherwise.
 */
int is_valid_entry_label(const char* label);

/**
 * @brief Checks if an entry label is defined in the assembler state.
 *
 * @param state The current assembler state.
 * @param label The label to check.
 * @return 1 if the entry label is defined, 0 otherwise.
 */
int is_entry_label_defined(const AssemblerState* state, const char* label);

/**
 * @brief Checks if an extern label is erroneously defined as an entry label.
 *
 * @param state The current assembler state.
 * @param label The label to check.
 * @return 1 if the extern label is defined as an entry, 0 otherwise.
 */

int is_extern_label_defined_as_entry(const AssemblerState* state, const char* label);

int data_intergity_check(char *line);

#endif 
