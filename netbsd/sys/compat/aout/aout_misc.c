/*	$NetBSD: aout_misc.c,v 1.13 2003/06/29 22:29:11 fvdl Exp $	*/

/*-
 * Copyright (c) 1998 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Christos Zoulas.
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
 *        This product includes software developed by the NetBSD
 *        Foundation, Inc. and its contributors.
 * 4. Neither the name of The NetBSD Foundation nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/cdefs.h>
__KERNEL_RCSID(0, "$NetBSD: aout_misc.c,v 1.13 2003/06/29 22:29:11 fvdl Exp $");

#if defined(_KERNEL_OPT)
#include "opt_ktrace.h"
#include "opt_nfsserver.h"
#include "opt_compat_netbsd.h"
#include "opt_sysv.h"
#include "opt_compat_43.h"

#include "fs_lfs.h"
#include "fs_nfs.h"
#endif

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/mount.h>
#include <sys/fcntl.h>
#include <sys/proc.h>

#include <sys/syscall.h>
#include <sys/sa.h>
#include <sys/syscallargs.h>

#include <compat/common/compat_util.h>
#include <compat/aout/aout_syscall.h>
#include <compat/aout/aout_syscallargs.h>

int
aout_sys_open(l, v, retval)
	struct lwp *l;
	void *v;
	register_t *retval;
{
	struct aout_sys_open_args *uap = v;
	struct proc *p = l->l_proc;
	caddr_t sg = stackgap_init(p, 0);

#if 0
	if (SCARG(uap, flags) & O_CREAT)
		CHECK_ALT_CREAT(p, &sg, SCARG(uap, path));
	else 
#endif
		CHECK_ALT_EXIST(p, &sg, SCARG(uap, path));

	return sys_open(l, v, retval);
}


int
aout_sys_link(l, v, retval)
	struct lwp *l;
	void *v;
	register_t *retval;
{
	struct aout_sys_link_args *uap = v;
	struct proc *p = l->l_proc;
	caddr_t sg = stackgap_init(p, 0);

	CHECK_ALT_EXIST(p, &sg, SCARG(uap, path));
#if 0
	CHECK_ALT_CREAT(p, &sg, SCARG(uap, link));
#endif

	return sys_link(l, v, retval);
}


int
aout_sys_unlink(l, v, retval)
	struct lwp *l;
	void *v;
	register_t *retval;
{
	struct aout_sys_unlink_args *uap = v;
	struct proc *p = l->l_proc;
	caddr_t sg = stackgap_init(p, 0);

	CHECK_ALT_EXIST(p, &sg, SCARG(uap, path));

	return sys_unlink(l, v, retval);
}


int
aout_sys_chdir(l, v, retval)
	struct lwp *l;
	void *v;
	register_t *retval;
{
	struct aout_sys_chdir_args *uap = v;
	struct proc *p = l->l_proc;
	caddr_t sg = stackgap_init(p, 0);

	CHECK_ALT_EXIST(p, &sg, SCARG(uap, path));

	return sys_chdir(l, v, retval);
}


int
aout_sys_chmod(l, v, retval)
	struct lwp *l;
	void *v;
	register_t *retval;
{
	struct aout_sys_chmod_args *uap = v;
	struct proc *p = l->l_proc;
	caddr_t sg = stackgap_init(p, 0);

	CHECK_ALT_EXIST(p, &sg, SCARG(uap, path));

	return sys_chmod(l, v, retval);
}


int
aout_sys_chown(l, v, retval)
	struct lwp *l;
	void *v;
	register_t *retval;
{
	struct aout_sys_chown_args *uap = v;
	struct proc *p = l->l_proc;
	caddr_t sg = stackgap_init(p, 0);

	CHECK_ALT_EXIST(p, &sg, SCARG(uap, path));

	return sys_chown(l, v, retval);
}


int
aout_sys_unmount(l, v, retval)
	struct lwp *l;
	void *v;
	register_t *retval;
{
	struct aout_sys_unmount_args *uap = v;
	struct proc *p = l->l_proc;
	caddr_t sg = stackgap_init(p, 0);

	CHECK_ALT_EXIST(p, &sg, SCARG(uap, path));

	return sys_unmount(l, v, retval);
}


int
aout_sys_access(l, v, retval)
	struct lwp *l;
	void *v;
	register_t *retval;
{
	struct aout_sys_access_args *uap = v;
	struct proc *p = l->l_proc;
	caddr_t sg = stackgap_init(p, 0);

	CHECK_ALT_EXIST(p, &sg, SCARG(uap, path));

	return sys_access(l, v, retval);
}


int
aout_sys_chflags(l, v, retval)
	struct lwp *l;
	void *v;
	register_t *retval;
{
	struct aout_sys_chflags_args *uap = v;
	struct proc *p = l->l_proc;
	caddr_t sg = stackgap_init(p, 0);

	CHECK_ALT_EXIST(p, &sg, SCARG(uap, path));

	return sys_chflags(l, v, retval);
}


int
aout_compat_43_sys_stat(l, v, retval)
	struct lwp *l;
	void *v;
	register_t *retval;
{
	struct aout_compat_43_sys_stat_args *uap = v;
	struct proc *p = l->l_proc;
	caddr_t sg = stackgap_init(p, 0);

	CHECK_ALT_EXIST(p, &sg, SCARG(uap, path));

	return compat_43_sys_stat(l, v, retval);
}


int
aout_compat_43_sys_lstat(l, v, retval)
	struct lwp *l;
	void *v;
	register_t *retval;
{
	struct aout_compat_43_sys_lstat_args *uap = v;
	struct proc *p = l->l_proc;
	caddr_t sg = stackgap_init(p, 0);

	CHECK_ALT_EXIST(p, &sg, SCARG(uap, path));

	return compat_43_sys_lstat(l, v, retval);
}


int
aout_sys_revoke(l, v, retval)
	struct lwp *l;
	void *v;
	register_t *retval;
{
	struct aout_sys_revoke_args *uap = v;
	struct proc *p = l->l_proc;
	caddr_t sg = stackgap_init(p, 0);

	CHECK_ALT_EXIST(p, &sg, SCARG(uap, path));

	return sys_revoke(l, v, retval);
}


int
aout_sys_symlink(l, v, retval)
	struct lwp *l;
	void *v;
	register_t *retval;
{
	struct aout_sys_symlink_args *uap = v;
	struct proc *p = l->l_proc;
	caddr_t sg = stackgap_init(p, 0);

	CHECK_ALT_EXIST(p, &sg, SCARG(uap, path));
#if 0
	CHECK_ALT_CREAT(p, &sg, SCARG(uap, link));
#endif

	return sys_symlink(l, v, retval);
}


int
aout_sys_readlink(l, v, retval)
	struct lwp *l;
	void *v;
	register_t *retval;
{
	struct aout_sys_readlink_args *uap = v;
	struct proc *p = l->l_proc;
	caddr_t sg = stackgap_init(p, 0);

	CHECK_ALT_SYMLINK(p, &sg, SCARG(uap, path));

	return sys_readlink(l, v, retval);
}


int
aout_sys_execve(l, v, retval)
	struct lwp *l;
	void *v;
	register_t *retval;
{
	struct aout_sys_execve_args *uap = v;
	struct proc *p = l->l_proc;
	caddr_t sg = stackgap_init(p, 0);

	CHECK_ALT_EXIST(p, &sg, SCARG(uap, path));

	return sys_execve(l, v, retval);
}


int
aout_sys_chroot(l, v, retval)
	struct lwp *l;
	void *v;
	register_t *retval;
{
	struct aout_sys_chroot_args *uap = v;
	struct proc *p = l->l_proc;
	caddr_t sg = stackgap_init(p, 0);

	CHECK_ALT_EXIST(p, &sg, SCARG(uap, path));

	return sys_chroot(l, v, retval);
}


int
aout_sys_rename(l, v, retval)
	struct lwp *l;
	void *v;
	register_t *retval;
{
	struct aout_sys_rename_args *uap = v;
	struct proc *p = l->l_proc;
	caddr_t sg = stackgap_init(p, 0);

	CHECK_ALT_EXIST(p, &sg, SCARG(uap, from));
#if 0
	CHECK_ALT_CREAT(p, &sg, SCARG(uap, to));
#endif

	return sys_rename(l, v, retval);
}


int
aout_compat_43_sys_truncate(l, v, retval)
	struct lwp *l;
	void *v;
	register_t *retval;
{
	struct aout_compat_43_sys_truncate_args *uap = v;
	struct proc *p = l->l_proc;
	caddr_t sg = stackgap_init(p, 0);

	CHECK_ALT_EXIST(p, &sg, SCARG(uap, path));

	return compat_43_sys_truncate(l, v, retval);
}


int
aout_sys_rmdir(l, v, retval)
	struct lwp *l;
	void *v;
	register_t *retval;
{
	struct aout_sys_rmdir_args *uap = v;
	struct proc *p = l->l_proc;
	caddr_t sg = stackgap_init(p, 0);

	CHECK_ALT_EXIST(p, &sg, SCARG(uap, path));

	return sys_rmdir(l, v, retval);
}


int
aout_sys_utimes(l, v, retval)
	struct lwp *l;
	void *v;
	register_t *retval;
{
	struct aout_sys_utimes_args *uap = v;
	struct proc *p = l->l_proc;
	caddr_t sg = stackgap_init(p, 0);

	CHECK_ALT_EXIST(p, &sg, SCARG(uap, path));

	return sys_utimes(l, v, retval);
}


int
aout_sys_statfs(l, v, retval)
	struct lwp *l;
	void *v;
	register_t *retval;
{
	struct aout_sys_statfs_args *uap = v;
	struct proc *p = l->l_proc;
	caddr_t sg = stackgap_init(p, 0);

	CHECK_ALT_EXIST(p, &sg, SCARG(uap, path));

	return sys_statfs(l, v, retval);
}


#if defined(NFS) || defined(NFSSERVER)
int
aout_sys_getfh(l, v, retval)
	struct lwp *l;
	void *v;
	register_t *retval;
{
	struct aout_sys_getfh_args *uap = v;
	struct proc *p = l->l_proc;
	caddr_t sg = stackgap_init(p, 0);

	CHECK_ALT_EXIST(p, &sg, SCARG(uap, fname));

	return sys_getfh(l, v, retval);
}
#endif


int
aout_compat_12_sys_stat(l, v, retval)
	struct lwp *l;
	void *v;
	register_t *retval;
{
	struct aout_compat_12_sys_stat_args *uap = v;
	struct proc *p = l->l_proc;
	caddr_t sg = stackgap_init(p, 0);

	CHECK_ALT_EXIST(p, &sg, SCARG(uap, path));

	return compat_12_sys_stat(l, v, retval);
}


int
aout_compat_12_sys_lstat(l, v, retval)
	struct lwp *l;
	void *v;
	register_t *retval;
{
	struct aout_compat_12_sys_lstat_args *uap = v;
	struct proc *p = l->l_proc;
	caddr_t sg = stackgap_init(p, 0);

	CHECK_ALT_EXIST(p, &sg, SCARG(uap, path));

	return compat_12_sys_lstat(l, v, retval);
}


int
aout_sys_pathconf(l, v, retval)
	struct lwp *l;
	void *v;
	register_t *retval;
{
	struct aout_sys_pathconf_args *uap = v;
	struct proc *p = l->l_proc;
	caddr_t sg = stackgap_init(p, 0);

	CHECK_ALT_EXIST(p, &sg, SCARG(uap, path));

	return sys_pathconf(l, v, retval);
}


int
aout_sys_truncate(l, v, retval)
	struct lwp *l;
	void *v;
	register_t *retval;
{
	struct aout_sys_truncate_args *uap = v;
	struct proc *p = l->l_proc;
	caddr_t sg = stackgap_init(p, 0);

	CHECK_ALT_EXIST(p, &sg, SCARG(uap, path));

	return sys_truncate(l, v, retval);
}


int
aout_sys_undelete(l, v, retval)
	struct lwp *l;
	void *v;
	register_t *retval;
{
	struct aout_sys_undelete_args *uap = v;
	struct proc *p = l->l_proc;
	caddr_t sg = stackgap_init(p, 0);

	CHECK_ALT_EXIST(p, &sg, SCARG(uap, path));

	return sys_undelete(l, v, retval);
}


int
aout_sys___posix_rename(l, v, retval)
	struct lwp *l;
	void *v;
	register_t *retval;
{
	struct aout_sys___posix_rename_args *uap = v;
	struct proc *p = l->l_proc;
	caddr_t sg = stackgap_init(p, 0);

	CHECK_ALT_EXIST(p, &sg, SCARG(uap, from));
#if 0
	CHECK_ALT_CREAT(p, &sg, SCARG(uap, to));
#endif

	return sys___posix_rename(l, v, retval);
}


int
aout_sys_lchmod(l, v, retval)
	struct lwp *l;
	void *v;
	register_t *retval;
{
	struct aout_sys_lchmod_args *uap = v;
	struct proc *p = l->l_proc;
	caddr_t sg = stackgap_init(p, 0);

	CHECK_ALT_SYMLINK(p, &sg, SCARG(uap, path));

	return sys_lchmod(l, v, retval);
}


int
aout_sys_lchown(l, v, retval)
	struct lwp *l;
	void *v;
	register_t *retval;
{
	struct aout_sys_lchown_args *uap = v;
	struct proc *p = l->l_proc;
	caddr_t sg = stackgap_init(p, 0);

	CHECK_ALT_SYMLINK(p, &sg, SCARG(uap, path));

	return sys_lchown(l, v, retval);
}


int
aout_sys_lutimes(l, v, retval)
	struct lwp *l;
	void *v;
	register_t *retval;
{
	struct aout_sys_lutimes_args *uap = v;
	struct proc *p = l->l_proc;
	caddr_t sg = stackgap_init(p, 0);

	CHECK_ALT_SYMLINK(p, &sg, SCARG(uap, path));

	return sys_lutimes(l, v, retval);
}


int
aout_sys___posix_chown(l, v, retval)
	struct lwp *l;
	void *v;
	register_t *retval;
{
	struct aout_sys___posix_chown_args *uap = v;
	struct proc *p = l->l_proc;
	caddr_t sg = stackgap_init(p, 0);

	CHECK_ALT_EXIST(p, &sg, SCARG(uap, path));

	return sys___posix_chown(l, v, retval);
}
