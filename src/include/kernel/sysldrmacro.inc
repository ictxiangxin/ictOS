;==============================================================================;
;                           ictOS Macros of sysldr.s                           ;
;                                                                      by: ict ;
;==============================================================================;

%macro LoadDesc 5	; LoadDesc( desc_addr, seg_base_seg, seg_base_offset, seg_limit, seg_attr )
	xor	eax, eax	; eax = 0
	mov	ax, %2	; ax(eax) = seg_base_seg
	shl	eax, 0x4	; ax(eax) *= 16 
	add	eax, %3	; ax(eax) += seg_base_offset
	mov	dword [SegBase], eax	; save eax into TempSegDesc.SegBase
	mov	dword [SegLimit], %4	; save seg_limit into TempSegDesc.SegLimit
	mov	word [SegAttr], %5	; save seg_attr into TempSegDesc.SegAttr
	mov	si, TempSegDesc	; si = TempSegDesc
	mov	di, %1	; di = desc_addr
	call	load_gdt	; load this segment information into right format
%endmacro

%macro LoadInt 4	; LoadInt( int_number, int_offset, int_selector, int_attr )
	push	es	
	mov	dword [IntOffset], %2
	mov	word [IntSelector], %3
	mov	word [IntAttr], %4
	mov	ax, ds
	mov	es, ax
	mov	esi, TempIntDesc
	mov	edi, temp_IDesc
	call	load_idt
	pop	es
	mov	eax, %1
	shl	eax, 0x3
	mov	edi, eax
	mov	esi, temp_IDesc
	mov	ecx, 0x8
	rep	movsb
%endmacro

%macro PortOut 2	; PortOut( port, data )
	mov	al, %2	; al = data
	mov	dx, %1
	out	dx, al	; send data to port
%rep 3	; delay 3 cpu cycle
	nop
%endrep
%endmacro

%macro Initialize 2	; Initialize( Function, msg )
	call	print
	db	color_lightgreen, %2, 0x0
	call	%1
	call	print
	db	color_lightblue, "[ OK ]", 0xa, 0x0
%endmacro
