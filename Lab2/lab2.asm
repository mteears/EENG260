
		.global Prog_Start, Prog_Stack, ResetISR, Vector_Table, _c_int00

		.text

Config_SysTick:
		MOV		R0, #0				; Clear R0
		MOVW	R1, #0xE010			; R1 = address of SysTick Control & Status Register
		MOVT	R1, #0xE000			;
		STR		R0, [R1]			; Disable timer (STCTRL = 0)
		ADD		R2, R1, #0x08		; R2 = address of SysTick Current Value Register
		STR		R0, [R2]			; Clear current value (STCURRENT = 0)
		MOVW	R3, #0xFFFF			; R3 = maximum SysTick reload value
		MOVT	R3, #0x00FF			;
		SUB		R2, R1, #0x04		; R2 = address pf SysTick Reload Value Register
		STR		R3, [R2]			; Reload value set to maximum (STRELOAD = 0x00FFFFFF)
		MOV		R3, #5				; Select system clock for timer and enable
		STR		R3, [R1]			; Enable timer (STCTRL = 5)
		BX		LR

Config_PortF:
		MOV		R0, #0				; Clear R0
		MOVW	R1, #0xE608			; R1 = address of GPIO Clock Gating Control
		MOVT	R1, #0x400F			;
		LDR		R2, [R1]			; Load current value of RCGCGPIO
		ORR		R2, R2, #0x20		; Enable Port F (bit 5 of RCGCGPIO = 1)
		STR		R2, [R1]			; Store new value of RCGCGPIO
		ADD		R1, R1, #0x0400		; R1 = address of GPIO Peripheral Ready Register
GPIO_Ready_Test:
		LDR		R2, [R1]			; Load current value of PRGPIO
		AND		R2, R2, #0x20		; Isolate bit 5
		CMP		R2, #0				; Check to see if bit 5 has been set
		BEQ		GPIO_Ready_Test		; Wait until bit 5 is set
		MOVW	R1, #0x5400			; R1 = address of GPIO Direction Select
		MOVT	R1, #0x4002			;
		LDR		R2, [R1]			; Load current value of GPIODIR
		ORR		R2, R2, #0x0E		; Set bits 1,2 & 3, PF1-PF3 set as output
		STR		R2, [R1]			; Store new value of GPIODIR
		ADD		R1, R1, #0x20		; R1 = address of GPIO Alternate Function Select
		LDR		R2, [R1]			; Load current value of GPIOAFSEL
		BIC		R2, R2, #0x0E		; Clear bits 1,2 & 3, PF1-PF3 alt function disabled
		STR		R2, [R1]			; Store new value of GPIOAFSEL
		ADD		R1, R1, #0x0108		; R1 = address of GPIO Analog Mode Select
		LDR		R2, [R1]			; Load current value of GPIOAMSEL
		BIC		R2, R2, #0x0E		; Clear bits 1,2 & 3, PF1-PF3 analog mode disabled
		STR		R2, [R1]			; Store new value of GPIOAMSEL
		ADD		R1, R1, #0x04		; R1 = address of GPIO Port Control
		LDR		R2, [R1]			; Load current value of GPIOPCTL
		MOVW	R3, #0x000F			; R3 = 0xFFFF000F
		MOVT	R3, #0xFFFF			;
		AND		R2, R2, R3			; Clear bits 4 through 15, PF1-PF3 set to GPIO
		STR		R2, [R1]			; Store new value of GPIOPCTL
		SUB		R1, R1, #0x10		; R1 = address of GPIO Digital Enable Register
		LDR		R2, [R1]			; Load current value of GPIODEN
		ORR		R2, R2, #0x0E		; Set bits 1,2 & 3, PF1-PF3 digital function enabled
		STR		R2, [R1]			; Store new value of GPIODEN
		SUB		R1, R1, #0x0120		; R1 = address of GPIO Data Register
		LDR		R2, [R1]			; Load current value of GPIODATA
		BIC		R2, R2, #0x0E		; Clear bits 1,2 & 3, PF1-PF3 data set to 0
		BX		LR

SysTick_Delay:
		MOVW	R1, #0xE014			; R1 = address of SysTick Reload Value Register
		MOVT	R1, #0xE000
		MOVW	R2, #0x3E7F			; Value for 1 ms delay (16 MHz / 15999)
		STR		R2, [R1]			; Store timer value
		ADD		R1, R1, #0x04		; R1 = address of SysTick Current Value Register
		MOV		R3, #0				;
		STR		R3, [R1]			; Clear current value and start timer
		MOVT	R3, #0x0001			; Set bit 16 in R3
		SUB		R1, R1, #0x08		; R1 = address of SysTick Control & Status Register
Count_Flag_Test:
		LDR		R2, [R1]			; Load current value of STCTRL
		AND		R2, R2, R3			; Isolate bit 16
		CMP		R2, #0				; Check to see if Count flag (bit 16) has been set
		BEQ		Count_Flag_Test		; Wait until Count flag is set
		BX		LR

ms_Delay:
		PUSH	{LR}				; Save return address (for Main_Loop)
Delay_Loop:
		BL		SysTick_Delay		; Branch to 1 ms delay
		SUB		R4, R4, #1			; Decrement loop count
		CMP		R4, #0				; Continue calling 1 ms delay until count reaches 0
		BNE		Delay_Loop			;
		POP		{PC}				; Return to Main_Loop

Prog_Start:
		ADD		SP, SP, #0x100		; Move stack pointer to middle of stack region
		BL		Config_SysTick		; Branch to SysTick configuration subroutine
		BL		Config_PortF		; Branch to GPIO Port F configuration subroutine
		MOVW	R5, #0x53FC			; R5 = address of GPIO Data Register
		MOVT	R5, #0x4002			;
Main_Loop:
		MOV		R6, #0x08			; Bit 3 (green LED)
		STR		R6, [R5]			; Turn LED on
		MOV		R4, #498			; Set loop count for 498 ms (1/2 sec) delay
		BL		ms_Delay			; Branch to delay subroutine
		MOV		R6, #0x00			;
		STR		R6, [R5]			; Turn LED off
		MOV		R4, #498			; Set loop count for 498 ms (1/2 sec) delay
		BL		ms_Delay			; Branch to delay subroutine
		B		Main_Loop			; Repeat Main_Loop

; Reset Handler
_c_int00:
		B		Prog_Start			; Branch to Prog_Start on Reset

; NMI Handler
NmiSR:
		B		$					; Infinite loop

; Fault Handler
FaultISR:
		B		$					; Infinite loop

; Defualt Handler
Def:
		B		$					; Infinite loop

; Define Stack
Prog_Stack		.usect ".stack", 0x200	; Reserve 512 bytes for the stack

; Interrupt Vector Table
		.retain ".intvecs"
		.sect ".intvecs"
Vector_Table:	.word	Prog_Stack	; Initial stack pointer
		.word	_c_int00			; Reset handler
		.word	NmiSR				; NMI handler
		.word	FaultISR			; Hard fault handler
		.word	Def, Def, Def, 0, 0
		.word	0, 0, Def, Def, 0, Def, Def, Def, Def, Def
		.word	Def, Def, Def, Def, Def, Def, Def, Def, Def, Def
		.word	Def, Def, Def, Def, Def, Def, Def, Def, Def, Def
		.word	Def, Def, Def, Def, Def, Def, Def, Def, Def, Def
		.word	Def, Def, Def, Def, Def, Def, Def, Def, 0, 0
		.word	Def, Def, Def, Def, Def, Def, Def, Def, Def, 0
		.word	0, Def, Def, Def, Def, Def, Def, Def, Def, Def
		.word	Def, 0, 0, 0, 0, Def, Def, Def, Def, 0
		.word	0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		.word	0, 0, 0, 0, 0, 0, 0, 0, 0, Def
		.word	Def, Def, Def, Def, Def, Def, Def, Def, Def, Def
		.word	Def, Def, Def, Def, 0, 0, Def, Def, Def, Def
		.word	Def, 0, 0, Def, Def, Def, Def, Def, Def, Def
		.word	Def, Def, Def, Def, Def, Def, Def, Def, Def, Def
		.word	Def, Def, Def, Def, Def, Def
