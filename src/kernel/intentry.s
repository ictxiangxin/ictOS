;==============================================================================;
;                 ICT perfect 2.00 Interrupt Programme Entry                   ;
;                                                                      by: ict ;
;==============================================================================;

%include	"intentry.inc"

%macro SaveCall 1
	pushad
	push	ds
	push	es
	push	gs
	push	fs
	mov	ax, ss
	mov	ds, ax
	mov	es, ax
	call	%1
	pop	fs
	pop	gs
	pop	es
	pop	ds
	popad
	iretd
%endmacro

global	keyboard_interrupt
global	hd_interrupt
global	clock_interrupt

[bits 32]
clock_interrupt:
	SaveCall	int_clock

hd_interrupt:
	SaveCall	int_hd

keyboard_interrupt:
	SaveCall	int_keyboard
