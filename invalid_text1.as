.entryLIST 
.extern fn1
MAIN: add r3,, LIST
        jsr fn1
LOOP:   prn #48 
        lea STR, r6
        inc r6
        mov *r6, L3          ddd 
        sub r1,r4
        cmp r3, #-646456468465456
        bne END
        add r7,*r6
        clr K
        sub L3,L3
.entry MAIN  
        jmp LOOP
END:    stop
STR:   .string "jjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjj
.extern MAIN
LIST:  .data  6, -9 ,d
       .data  -100 
fn1:     .data  31 
.extern L3
