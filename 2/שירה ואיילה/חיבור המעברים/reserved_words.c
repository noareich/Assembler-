#include "assembler.h"
char strings[RESERVED_WORD_NUM][10] = {
    ".data", ".string", ".entry", ".extern", "define",
    "macr", "endmacr", 
    "mov", "cmp", "add", "sub", "lea",
    "not", "clr", "inc", "dec", "jmp",
    "bne", "red", "prn", "jsr", "rts", "stop",
    "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7"
};

const char* instruction_set[] = {
    "mov", "cmp", "add", "sub", "lea",
    "not", "clr", "inc", "dec", "jmp",
    "bne", "red", "prn", "jsr", "rts", "stop"
};
int const instruction_set_size = sizeof(instruction_set) / sizeof(instruction_set[0]);

const char* registers[] = {
    "r0", "r1", "r2", "r3", "r4", "r5",
    "r6", "r7"
};
int const registers_size = sizeof(registers) / sizeof(registers[0]);

const char* Guidelines_set[] = {
    ".data", ".string", ".entry", ".extern", "define"
};
int const Guidelines_set_size = sizeof(Guidelines_set) / sizeof(Guidelines_set[0]);