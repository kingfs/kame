/*
 * Written by J.T. Conklin <jtc@netbsd.org>.
 * Public domain.
 */

#include <machine/asm.h>

#if defined(LIBC_SCCS)
	RCSID("$NetBSD: index.S,v 1.1 1998/02/22 07:20:52 mycroft Exp $")
#endif

#ifdef STRCHR
ENTRY(strchr)
#else
ENTRY(index)
#endif
	pushl	%ebx
	movl	8(%esp),%eax
	movb	12(%esp),%cl
	.align 2,0x90
L1:
	movb	(%eax),%bl
	cmpb	%bl,%cl			/* found char??? */
	je 	L2
	incl	%eax
	testb	%bl,%bl			/* null terminator??? */
	jnz	L1
	xorl	%eax,%eax
L2:
	popl	%ebx
	ret
