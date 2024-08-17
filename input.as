MAIN: mov r3, LENGTH
LOOP: jmp L1
      prn #48
      sub r1, r4
      cmp r3, #0
      bne END
L1:   inc K
      bne LOOP
END:  stop

STR:  .string "abcdef"
LENGTH: .data 6, -9, 15
K:    .data 22

.entry MAIN
.entry LOOP
.extern L3