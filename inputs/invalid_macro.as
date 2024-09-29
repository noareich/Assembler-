MAIN: add r3, LIST
prn #48
macr , m1 
cmp r3, #-6
bne END
endmacr gc
lea STR, r6

sub rl, r4
m1
dec K
jmp LOOP
stop
macr m2
cmp r3, #-6
bne END
endmacr

macr m2
cmp r3, #-6

endmacr
