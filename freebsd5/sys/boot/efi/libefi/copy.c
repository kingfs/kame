/*-
 * Copyright (c) 1998 Michael Smith <msmith@freebsd.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef lint
static const char rcsid[] =
  "$FreeBSD: src/sys/boot/efi/libefi/copy.c,v 1.4 2002/05/19 04:42:18 marcel Exp $";
#endif /* not lint */

/*
 * MD primitives supporting placement of module data 
 */
#include <stand.h>

#include <efi.h>
#include <efilib.h>
#include <machine/ia64_cpu.h>
#include <machine/vmparam.h>

int
efi_copyin(void *src, vm_offset_t dest, size_t len)
{
	EFI_PHYSICAL_ADDRESS p = IA64_RR_MASK(dest);
#if 0
	BS->AllocatePages(AllocateAddress, EfiRuntimeServicesData,
			  len >> 12, &p);
#endif
	bcopy(src, (void*) p, len);
	return (len);
}

int
efi_copyout(vm_offset_t src, void *dest, size_t len)
{
	bcopy((void*) IA64_RR_MASK(src), dest, len);
	return (len);
}

int
efi_readin(int fd, vm_offset_t dest, size_t len)
{
	EFI_PHYSICAL_ADDRESS p = IA64_RR_MASK(dest);
#if 0
	BS->AllocatePages(AllocateAddress, EfiRuntimeServicesData,
			  len >> 12, &p);
#endif
	return (read(fd, (void*) p, len));
}
