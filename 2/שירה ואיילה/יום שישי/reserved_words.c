#include "assembler.h"

/*An array of all reserved words in the language*/
char strings[RESERVED_WORD_NUM][10] = {
    /*Guideline names*/
    ".data",
    ".string",
    ".entry",
    ".extern",
    "define",
    /*Used to define a macro*/
    "macr",
    "endmacr",
    /*16 instructions in our assembly language*/
    "mov",
    "cmp",
    "add",
    "sub",
    "lea",
    "not",
    "clr",
    "inc",
    "dec",
    "jmp",
    "bne",
    "red", 
    "prn",
    "jsr", 
    "rts",
    "stop",
    /*These are the 8 registers of the processor*/
    "r0",
    "r1",
    "r2",
    "r3",
    "r4",
    "r5",
    "r6",
    "r7"
};
