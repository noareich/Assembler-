#include "assembler.h"
#ifndef CHECK_H
#define CHECK_H

#define MAX_LINE_LENGTH 81
#define MAX_LABEL_LENGTH 31
#define MAX_LABELS 1000




// Function prototypes for check functions
bool check_line_length(const char* line);
bool is_empty_or_whitespace(const char* line);
bool is_valid_comment(const char* line);
bool is_valid_label(const char* label);
bool is_reserved_word(const char* word);
bool is_duplicate_label(const char* label, const AssemblerState* state);
bool is_valid_instruction(const char* instruction);
bool is_valid_immediate(const char* operand);
bool check_operand_count(const char* instruction, int operand_count);
bool is_valid_operand(const char* operand);
bool is_valid_addressing_mode(const char* instruction, const char* operand, bool is_source);
bool is_valid_entry_label(const char* label);
bool is_entry_label_defined(const AssemblerState* state, const char* label);
bool is_extern_label_not_defined(const AssemblerState* state, const char* label);

#endif // CHECK_H