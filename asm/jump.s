default rel
global jump

section .text
bits 64

jump:
	cli
	lgdt [rcx]

	; stack
	mov rsp, rdx

	; data segment
	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax

	; code segment
	mov rax, 0x08
	push 0x08
	push r8

	; arguments
	mov rcx, r9

	retfq
