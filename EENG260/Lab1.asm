		.global Prog_Start, _c_int00, ResetISR, myStack, Vecs, _main
		.sect ".mycode"

									; program creates a table of 10 values in memorty and stores values 0 - 15
									;First	.equ	0x20000400 ;this will be the first address in memory to which we will write the first value
									;Count	.equ	0x10 this is our loop count
									;Init_Sum	.equ	0x0000	;initialize sum to zero

;Label		pnemonic	 operands
Prog_Start:	MOVW		R1,#0x0400	;puts 0x0400 in Register 1. [15:00] 	(0x----0400)
			MOVT		R1,#0x2000	;puts 0x2000 in Register 1. [31:16]		(0x20000400) {address}
			MOVB		R0,#0x00	;puts 0x0000 in Register 0. (0x0000)
			MOVB		R2,#0x10	;puts 0x0010 in Register 2. (0x0010)
Loop:		STRB		R0,[R1]		;puts LSB from R0 into address at R1	(0x0000 -> 0x20000400) {first time}
			ADD			R1,#1		;adds 1 to R1.  (
			ADD			R0,#1		;adds 1 to R0.  (0x0001) {first time}
			SUB			R2,R2,#1	;Subtracts 1 from R1, places result back in R2.
			BNE			Loop		;Returns to Loop if the zero flag is set.  (ie.  the previous operation finally equaled 0).
			MOVW		R1,#0x0400	;Same as line 10.  ;puts 0x0400 in Register 1. [15:00] 	(0x----0400)
			MOVT		R1,#0x2000	;puts 0x2000 in Register 1. [31:16]		(0x20000400) {address}
			MOVB		R2,#0x10	;puts 0x0010 in Register 2. (0x0010)
			MOVW		R3,#0x0000	;puts 0x0000 in Register 3. (0x0000)
Sum:		LDR			R0,[R1]		;Loads R0 from the address in R1. (R0 <- 0x20000400) {first time should be 0}
			ADD			R3,R3,R0	;Adds value in R0 to R3, saves into R3
			ADD			R1,#1		;Adds 1 to R1
			SUB			R2,R2,#1	;Subtracts 1 from R2 and saves to R2
			BNE			Sum			;Jumps to Sum: if R2 != 0
			STR			R3,[R1]		;Stores value of R3 in Address in R1
			B			Prog_Start
			.text

_c_int00:	B			Prog_Start			;reset handler
NMISR:		B			$					;NMI

FaultISR:	B			$

myStack		.usect		".stack",	0x400

			.retain		".intvecs"			;abbreviated interrupt vector table
			.sect		".intvecs"
Vecs:		.word myStack					;initial stack pointer
			.word _c_int00					;the reset handler
			.word NMISR
			.word FaultISR
			.word 0
