		.global Prog_Start, _c_int00, ResetISR, myStack, Vecs, _main
		.sect ".mycode"

									; program creates a table of 10 values in memorty and stores values 0 - 15
									;First	.equ	0x20000400 ;this will be the first address in memory to which we will write the first value
									;Count	.equ	0x10 this is our loop count
									;Init_Sum	.equ	0x0000	;initialize sum to zero				

;Label		pnemonic	 operands
Prog_Start:	MOVW		R1,#0x0400
			MOVT		R1,#0x2000
			MOVB		R0,#0x00
			MOVB		R2,#0x10
Loop:		STRB		R0,[R1]
			ADD			R1,#1
			ADD			R0,#1
			SUB			R2,R2,#1
			BNE			Loop
			MOVW		R1,#0x0400
			MOVT		R1,#0x2000
			MOVB		R2,#0x10
			MOVW		R3,#0x0000
Sum:		LDR			R0,[R1]
			ADD			R3,R3,R0
			ADD			R1,#1
			SUB			R2,R2,#1
			BNE			Sum
			STR			R3,[R1]
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
