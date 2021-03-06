/*	$OpenBSD: linux_ioctl.h,v 1.4 1997/12/07 22:59:15 provos Exp $	*/
/*	$NetBSD: linux_ioctl.h,v 1.4 1996/04/05 00:01:36 christos Exp $	*/

/*
 * Copyright (c) 1995 Frank van der Linden
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
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed for the NetBSD Project
 *      by Frank van der Linden
 * 4. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#define _LINUX_IO(x,y)		(((x) << 8) | (y))
#define	LINUX_IOCGROUP(x)	(((x) >> 8) & 0xff)

struct linux_sys_ioctl_args;
int linux_ioctl_audio __P((struct proc *, struct linux_sys_ioctl_args *,
    register_t *));
int linux_machdepioctl __P((struct proc *, void *, register_t *));
int linux_ioctl_termios __P((struct proc *, struct linux_sys_ioctl_args *,
    register_t *));
int linux_ioctl_cdrom __P((struct proc *, struct linux_sys_ioctl_args *,
    register_t *));
int linux_ioctl_socket __P((struct proc *, struct linux_sys_ioctl_args *,
    register_t *));
