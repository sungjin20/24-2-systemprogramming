	.file	"decomment.c"
	.text
	.globl	print_char
	.type	print_char, @function
print_char:
.LFB0:
	.cfi_startproc
	endbr64
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movq	%rdi, -8(%rbp)
	movq	%rsi, -16(%rbp)
	movq	%rdx, -24(%rbp)
	movl	$39, %edi
	call	putchar@PLT
.L8:
	call	getchar@PLT
	movq	-8(%rbp), %rdx
	movl	%eax, (%rdx)
	movq	-8(%rbp), %rax
	movl	(%rax), %eax
	cmpl	$-1, %eax
	jne	.L2
	movl	$1, %eax
	jmp	.L3
.L2:
	movq	-8(%rbp), %rax
	movl	(%rax), %eax
	movl	%eax, %edx
	movq	-16(%rbp), %rax
	movb	%dl, (%rax)
	movq	-16(%rbp), %rax
	movzbl	(%rax), %eax
	movsbl	%al, %eax
	movl	%eax, %edi
	call	putchar@PLT
	movq	-16(%rbp), %rax
	movzbl	(%rax), %eax
	cmpb	$92, %al
	jne	.L4
	call	getchar@PLT
	movq	-8(%rbp), %rdx
	movl	%eax, (%rdx)
	movq	-8(%rbp), %rax
	movl	(%rax), %eax
	movl	%eax, %edx
	movq	-16(%rbp), %rax
	movb	%dl, (%rax)
	movq	-16(%rbp), %rax
	movzbl	(%rax), %eax
	movsbl	%al, %eax
	movl	%eax, %edi
	call	putchar@PLT
	jmp	.L5
.L4:
	movq	-16(%rbp), %rax
	movzbl	(%rax), %eax
	cmpb	$10, %al
	jne	.L6
	movq	-24(%rbp), %rax
	movl	(%rax), %eax
	leal	1(%rax), %edx
	movq	-24(%rbp), %rax
	movl	%edx, (%rax)
.L6:
	movq	-16(%rbp), %rax
	movzbl	(%rax), %eax
	cmpb	$39, %al
	je	.L10
.L5:
	jmp	.L8
.L10:
	nop
	movl	$0, %eax
.L3:
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE0:
	.size	print_char, .-print_char
	.globl	print_str
	.type	print_str, @function
print_str:
.LFB1:
	.cfi_startproc
	endbr64
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movq	%rdi, -8(%rbp)
	movq	%rsi, -16(%rbp)
	movq	%rdx, -24(%rbp)
	movl	$34, %edi
	call	putchar@PLT
.L18:
	call	getchar@PLT
	movq	-8(%rbp), %rdx
	movl	%eax, (%rdx)
	movq	-8(%rbp), %rax
	movl	(%rax), %eax
	cmpl	$-1, %eax
	jne	.L12
	movl	$1, %eax
	jmp	.L13
.L12:
	movq	-8(%rbp), %rax
	movl	(%rax), %eax
	movl	%eax, %edx
	movq	-16(%rbp), %rax
	movb	%dl, (%rax)
	movq	-16(%rbp), %rax
	movzbl	(%rax), %eax
	movsbl	%al, %eax
	movl	%eax, %edi
	call	putchar@PLT
	movq	-16(%rbp), %rax
	movzbl	(%rax), %eax
	cmpb	$92, %al
	jne	.L14
	call	getchar@PLT
	movq	-8(%rbp), %rdx
	movl	%eax, (%rdx)
	movq	-8(%rbp), %rax
	movl	(%rax), %eax
	movl	%eax, %edx
	movq	-16(%rbp), %rax
	movb	%dl, (%rax)
	movq	-16(%rbp), %rax
	movzbl	(%rax), %eax
	movsbl	%al, %eax
	movl	%eax, %edi
	call	putchar@PLT
	jmp	.L15
.L14:
	movq	-16(%rbp), %rax
	movzbl	(%rax), %eax
	cmpb	$10, %al
	jne	.L16
	movq	-24(%rbp), %rax
	movl	(%rax), %eax
	leal	1(%rax), %edx
	movq	-24(%rbp), %rax
	movl	%edx, (%rax)
.L16:
	movq	-16(%rbp), %rax
	movzbl	(%rax), %eax
	cmpb	$34, %al
	je	.L20
.L15:
	jmp	.L18
.L20:
	nop
	movl	$0, %eax
.L13:
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE1:
	.size	print_str, .-print_str
	.globl	decomment_sl
	.type	decomment_sl, @function
decomment_sl:
.LFB2:
	.cfi_startproc
	endbr64
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$16, %rsp
	movq	%rdi, -8(%rbp)
	movq	%rsi, -16(%rbp)
	movl	$32, %edi
	call	putchar@PLT
.L25:
	call	getchar@PLT
	movq	-8(%rbp), %rdx
	movl	%eax, (%rdx)
	movq	-8(%rbp), %rax
	movl	(%rax), %eax
	cmpl	$-1, %eax
	je	.L26
	movq	-8(%rbp), %rax
	movl	(%rax), %eax
	movl	%eax, %edx
	movq	-16(%rbp), %rax
	movb	%dl, (%rax)
	movq	-16(%rbp), %rax
	movzbl	(%rax), %eax
	cmpb	$10, %al
	jne	.L25
	movq	-16(%rbp), %rax
	movzbl	(%rax), %eax
	movsbl	%al, %eax
	movl	%eax, %edi
	call	putchar@PLT
	jmp	.L23
.L26:
	nop
.L23:
	nop
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE2:
	.size	decomment_sl, .-decomment_sl
	.section	.rodata
	.align 8
.LC0:
	.string	"Error: line %d: unterminated comment\n"
	.text
	.globl	decomment_ml
	.type	decomment_ml, @function
decomment_ml:
.LFB3:
	.cfi_startproc
	endbr64
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$48, %rsp
	movq	%rdi, -24(%rbp)
	movq	%rsi, -32(%rbp)
	movq	%rdx, -40(%rbp)
	movq	%rcx, -48(%rbp)
	movl	$32, %edi
	call	putchar@PLT
	movq	-40(%rbp), %rax
	movl	(%rax), %edx
	movq	-48(%rbp), %rax
	movl	%edx, (%rax)
	movl	$0, -4(%rbp)
.L35:
	call	getchar@PLT
	movq	-24(%rbp), %rdx
	movl	%eax, (%rdx)
	movq	-24(%rbp), %rax
	movl	(%rax), %eax
	cmpl	$-1, %eax
	jne	.L28
	movq	-48(%rbp), %rax
	movl	(%rax), %edx
	movq	stderr(%rip), %rax
	leaq	.LC0(%rip), %rcx
	movq	%rcx, %rsi
	movq	%rax, %rdi
	movl	$0, %eax
	call	fprintf@PLT
	movl	$1, %eax
	jmp	.L29
.L28:
	movq	-24(%rbp), %rax
	movl	(%rax), %eax
	movl	%eax, %edx
	movq	-32(%rbp), %rax
	movb	%dl, (%rax)
	jmp	.L30
.L31:
	call	getchar@PLT
	movq	-24(%rbp), %rdx
	movl	%eax, (%rdx)
	movq	-24(%rbp), %rax
	movl	(%rax), %eax
	movl	%eax, %edx
	movq	-32(%rbp), %rax
	movb	%dl, (%rax)
	movq	-32(%rbp), %rax
	movzbl	(%rax), %eax
	cmpb	$47, %al
	jne	.L30
	movl	$1, -4(%rbp)
.L30:
	movq	-32(%rbp), %rax
	movzbl	(%rax), %eax
	cmpb	$42, %al
	je	.L31
	cmpl	$0, -4(%rbp)
	jne	.L37
	movq	-32(%rbp), %rax
	movzbl	(%rax), %eax
	cmpb	$10, %al
	jne	.L35
	movq	-32(%rbp), %rax
	movzbl	(%rax), %eax
	movsbl	%al, %eax
	movl	%eax, %edi
	call	putchar@PLT
	movq	-40(%rbp), %rax
	movl	(%rax), %eax
	leal	1(%rax), %edx
	movq	-40(%rbp), %rax
	movl	%edx, (%rax)
	jmp	.L35
.L37:
	nop
	movl	$0, %eax
.L29:
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE3:
	.size	decomment_ml, .-decomment_ml
	.globl	main
	.type	main, @function
main:
.LFB4:
	.cfi_startproc
	endbr64
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movq	%fs:40, %rax
	movq	%rax, -8(%rbp)
	xorl	%eax, %eax
	movl	$1, -20(%rbp)
	movl	$-1, -16(%rbp)
.L52:
	movl	$0, -12(%rbp)
	call	getchar@PLT
	movl	%eax, -24(%rbp)
	movl	-24(%rbp), %eax
	cmpl	$-1, %eax
	je	.L55
	movl	-24(%rbp), %eax
	movb	%al, -25(%rbp)
	movzbl	-25(%rbp), %eax
	cmpb	$39, %al
	jne	.L41
	leaq	-20(%rbp), %rdx
	leaq	-25(%rbp), %rcx
	leaq	-24(%rbp), %rax
	movq	%rcx, %rsi
	movq	%rax, %rdi
	call	print_char
	movl	%eax, -12(%rbp)
	jmp	.L42
.L41:
	movzbl	-25(%rbp), %eax
	cmpb	$34, %al
	jne	.L43
	leaq	-20(%rbp), %rdx
	leaq	-25(%rbp), %rcx
	leaq	-24(%rbp), %rax
	movq	%rcx, %rsi
	movq	%rax, %rdi
	call	print_str
	movl	%eax, -12(%rbp)
	jmp	.L42
.L43:
	movzbl	-25(%rbp), %eax
	cmpb	$47, %al
	jne	.L44
	call	getchar@PLT
	movl	%eax, -24(%rbp)
	movl	-24(%rbp), %eax
	movb	%al, -25(%rbp)
	movzbl	-25(%rbp), %eax
	cmpb	$47, %al
	jne	.L45
	leaq	-25(%rbp), %rdx
	leaq	-24(%rbp), %rax
	movq	%rdx, %rsi
	movq	%rax, %rdi
	call	decomment_sl
	jmp	.L42
.L45:
	movzbl	-25(%rbp), %eax
	cmpb	$42, %al
	jne	.L46
	leaq	-16(%rbp), %rcx
	leaq	-20(%rbp), %rdx
	leaq	-25(%rbp), %rsi
	leaq	-24(%rbp), %rax
	movq	%rax, %rdi
	call	decomment_ml
	testl	%eax, %eax
	je	.L42
	movl	$1, %eax
	jmp	.L53
.L46:
	movl	$47, %edi
	call	putchar@PLT
	movzbl	-25(%rbp), %eax
	cmpb	$39, %al
	jne	.L48
	leaq	-20(%rbp), %rdx
	leaq	-25(%rbp), %rcx
	leaq	-24(%rbp), %rax
	movq	%rcx, %rsi
	movq	%rax, %rdi
	call	print_char
	movl	%eax, -12(%rbp)
	jmp	.L42
.L48:
	movzbl	-25(%rbp), %eax
	cmpb	$34, %al
	jne	.L49
	leaq	-20(%rbp), %rdx
	leaq	-25(%rbp), %rcx
	leaq	-24(%rbp), %rax
	movq	%rcx, %rsi
	movq	%rax, %rdi
	call	print_str
	movl	%eax, -12(%rbp)
	jmp	.L42
.L49:
	movzbl	-25(%rbp), %eax
	movsbl	%al, %eax
	movl	%eax, %edi
	call	putchar@PLT
	jmp	.L42
.L44:
	movzbl	-25(%rbp), %eax
	movsbl	%al, %eax
	movl	%eax, %edi
	call	putchar@PLT
.L42:
	movzbl	-25(%rbp), %eax
	cmpb	$10, %al
	jne	.L50
	movl	-20(%rbp), %eax
	addl	$1, %eax
	movl	%eax, -20(%rbp)
.L50:
	cmpl	$0, -12(%rbp)
	jne	.L56
	jmp	.L52
.L55:
	nop
	jmp	.L40
.L56:
	nop
.L40:
	movl	$0, %eax
.L53:
	movq	-8(%rbp), %rdx
	subq	%fs:40, %rdx
	je	.L54
	call	__stack_chk_fail@PLT
.L54:
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE4:
	.size	main, .-main
	.ident	"GCC: (Ubuntu 11.4.0-1ubuntu1~22.04) 11.4.0"
	.section	.note.GNU-stack,"",@progbits
	.section	.note.gnu.property,"a"
	.align 8
	.long	1f - 0f
	.long	4f - 1f
	.long	5
0:
	.string	"GNU"
1:
	.align 8
	.long	0xc0000002
	.long	3f - 2f
2:
	.long	0x3
3:
	.align 8
4:
