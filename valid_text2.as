.entry b1
.extern f1

jmp  LOOP
 .extern f2
  macr m

MAIN: mov f1 ,r4
 .string "HELLO" 

  endmacr
.entry b2
  LOOP: add #2 ,r4
b1: .data 1 , 3 , -3 , 5

;comment
b2: cmp f2,r2
.string "LOVE"
