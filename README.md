# Two-Pass Assembler

This project implements a two-pass assembler for a custom assembly language. The assembler is written in C and is designed to run on Ubuntu systems.

# Project Overview

The assembler processes assembly language source files and generates machine code output. It operates in three main stages:

1. Pre-processing (Macro expansion)
2. First Pass
3. Second Pass

# Pre-processing
- Expands macros defined in the source file
- Generates an intermediate file with expanded macros

# First Pass
- Builds the symbol table
- Calculates memory addresses for labels
- Performs initial error checking

# Second Pass
- Generates the final machine code
- Resolves symbol addresses
- Produces output files (.ob, .ent, .ext)

# Features

- Supports various addressing modes
- Handles data and string directives
- Processes entry and extern declarations
- Performs comprehensive error checking

 
The assembler generates the following output files:

.ob: Object file containing the machine code
.ent: Entry labels file (if any entry labels are defined)
.ext: External labels file (if any external labels are used)

Error Handling
The assembler performs extensive error checking during both passes. If errors are encountered, they are reported to stderr, and no output files are generated.
Limitations

The assembler follows the ANSI C90 standard
Designed specifically for the custom assembly language defined in the project specifications
May have limitations on input file size and number of symbols

# developed by Noa Reich and Leli Vexler.
