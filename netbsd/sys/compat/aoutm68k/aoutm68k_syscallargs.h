/* $NetBSD: aoutm68k_syscallargs.h,v 1.9 2002/05/03 00:24:22 eeh Exp $ */

/*
 * System call argument lists.
 *
 * DO NOT EDIT-- this file is automatically generated.
 * created from	NetBSD: syscalls.master,v 1.7 2002/05/03 00:20:56 eeh Exp 
 */

#ifndef _AOUTM68K_SYS__SYSCALLARGS_H_
#define	_AOUTM68K_SYS__SYSCALLARGS_H_

#ifdef	syscallarg
#undef	syscallarg
#endif

#define	syscallarg(x)							\
	union {								\
		register_t pad;						\
		struct { x datum; } le;					\
		struct { /* LINTED zero array dimension */		\
			int8_t pad[  /* CONSTCOND */			\
				(sizeof (register_t) < sizeof (x))	\
				? 0					\
				: sizeof (register_t) - sizeof (x)];	\
			x datum;					\
		} be;							\
	}

struct aoutm68k_sys_open_args {
	syscallarg(const char *) path;
	syscallarg(int) flags;
	syscallarg(mode_t) mode;
};

struct aoutm68k_sys_link_args {
	syscallarg(const char *) path;
	syscallarg(const char *) link;
};

struct aoutm68k_sys_unlink_args {
	syscallarg(const char *) path;
};

struct aoutm68k_sys_chdir_args {
	syscallarg(const char *) path;
};

struct aoutm68k_sys_chmod_args {
	syscallarg(const char *) path;
	syscallarg(mode_t) mode;
};

struct aoutm68k_sys_chown_args {
	syscallarg(const char *) path;
	syscallarg(uid_t) uid;
	syscallarg(gid_t) gid;
};

struct aoutm68k_sys_access_args {
	syscallarg(const char *) path;
	syscallarg(int) flags;
};

struct aoutm68k_sys_chflags_args {
	syscallarg(const char *) path;
	syscallarg(u_long) flags;
};

struct aoutm68k_compat_43_sys_stat_args {
	syscallarg(const char *) path;
	syscallarg(struct aoutm68k_stat43 *) ub;
};

struct aoutm68k_compat_43_sys_lstat_args {
	syscallarg(const char *) path;
	syscallarg(struct aoutm68k_stat43 *) ub;
};

struct aoutm68k_sys_ioctl_args {
	syscallarg(int) fd;
	syscallarg(u_long) com;
	syscallarg(void *) data;
};

struct aoutm68k_sys_revoke_args {
	syscallarg(const char *) path;
};

struct aoutm68k_sys_symlink_args {
	syscallarg(const char *) path;
	syscallarg(const char *) link;
};

struct aoutm68k_sys_readlink_args {
	syscallarg(const char *) path;
	syscallarg(char *) buf;
	syscallarg(size_t) count;
};

struct aoutm68k_sys_execve_args {
	syscallarg(const char *) path;
	syscallarg(char *const *) argp;
	syscallarg(char *const *) envp;
};

struct aoutm68k_sys_chroot_args {
	syscallarg(const char *) path;
};

struct aoutm68k_compat_43_sys_fstat_args {
	syscallarg(int) fd;
	syscallarg(struct aoutm68k_stat43 *) sb;
};

struct aoutm68k_sys_rename_args {
	syscallarg(const char *) from;
	syscallarg(const char *) to;
};

struct aoutm68k_compat_43_sys_truncate_args {
	syscallarg(const char *) path;
	syscallarg(long) length;
};

struct aoutm68k_sys_rmdir_args {
	syscallarg(const char *) path;
};

struct aoutm68k_sys_utimes_args {
	syscallarg(const char *) path;
	syscallarg(const struct timeval *) tptr;
};

struct aoutm68k_sys_statfs_args {
	syscallarg(const char *) path;
	syscallarg(struct statfs *) buf;
};

struct aoutm68k_sys_getfh_args {
	syscallarg(const char *) fname;
	syscallarg(fhandle_t *) fhp;
};

struct aoutm68k_compat_12_sys_stat_args {
	syscallarg(const char *) path;
	syscallarg(struct aoutm68k_stat12 *) ub;
};

struct aoutm68k_compat_12_sys_fstat_args {
	syscallarg(int) fd;
	syscallarg(struct aoutm68k_stat12 *) sb;
};

struct aoutm68k_compat_12_sys_lstat_args {
	syscallarg(const char *) path;
	syscallarg(struct aoutm68k_stat12 *) ub;
};

struct aoutm68k_sys_pathconf_args {
	syscallarg(const char *) path;
	syscallarg(int) name;
};

struct aoutm68k_sys_truncate_args {
	syscallarg(const char *) path;
	syscallarg(int) pad;
	syscallarg(off_t) length;
};

struct aoutm68k_sys_undelete_args {
	syscallarg(const char *) path;
};

struct aoutm68k_sys___posix_rename_args {
	syscallarg(const char *) from;
	syscallarg(const char *) to;
};

struct aoutm68k_sys_lchmod_args {
	syscallarg(const char *) path;
	syscallarg(mode_t) mode;
};

struct aoutm68k_sys_lchown_args {
	syscallarg(const char *) path;
	syscallarg(uid_t) uid;
	syscallarg(gid_t) gid;
};

struct aoutm68k_sys_lutimes_args {
	syscallarg(const char *) path;
	syscallarg(const struct timeval *) tptr;
};

struct aoutm68k_sys___stat13_args {
	syscallarg(const char *) path;
	syscallarg(struct aoutm68k_stat *) ub;
};

struct aoutm68k_sys___fstat13_args {
	syscallarg(int) fd;
	syscallarg(struct aoutm68k_stat *) sb;
};

struct aoutm68k_sys___lstat13_args {
	syscallarg(const char *) path;
	syscallarg(struct aoutm68k_stat *) ub;
};

struct aoutm68k_sys___posix_chown_args {
	syscallarg(const char *) path;
	syscallarg(uid_t) uid;
	syscallarg(gid_t) gid;
};

struct aoutm68k_sys_fhstat_args {
	syscallarg(const fhandle_t *) fhp;
	syscallarg(struct aoutm68k_stat *) sb;
};

/*
 * System call prototypes.
 */

int	sys_exit(struct proc *, void *, register_t *);
int	sys_fork(struct proc *, void *, register_t *);
int	sys_read(struct proc *, void *, register_t *);
int	sys_write(struct proc *, void *, register_t *);
#ifdef COMPAT_AOUT_ALTPATH
int	aoutm68k_sys_open(struct proc *, void *, register_t *);
#else
int	sys_open(struct proc *, void *, register_t *);
#endif
int	sys_close(struct proc *, void *, register_t *);
int	sys_wait4(struct proc *, void *, register_t *);
#ifdef COMPAT_43
int	compat_43_sys_creat(struct proc *, void *, register_t *);
#else
#endif
#ifdef COMPAT_AOUT_ALTPATH
int	aoutm68k_sys_link(struct proc *, void *, register_t *);
int	aoutm68k_sys_unlink(struct proc *, void *, register_t *);
#else
int	sys_link(struct proc *, void *, register_t *);
int	sys_unlink(struct proc *, void *, register_t *);
#endif
#ifdef COMPAT_AOUT_ALTPATH
int	aoutm68k_sys_chdir(struct proc *, void *, register_t *);
#else
int	sys_chdir(struct proc *, void *, register_t *);
#endif
int	sys_fchdir(struct proc *, void *, register_t *);
int	sys_mknod(struct proc *, void *, register_t *);
#ifdef COMPAT_AOUT_ALTPATH
int	aoutm68k_sys_chmod(struct proc *, void *, register_t *);
int	aoutm68k_sys_chown(struct proc *, void *, register_t *);
#else
int	sys_chmod(struct proc *, void *, register_t *);
int	sys_chown(struct proc *, void *, register_t *);
#endif
int	sys_obreak(struct proc *, void *, register_t *);
int	sys_getfsstat(struct proc *, void *, register_t *);
#ifdef COMPAT_43
int	compat_43_sys_lseek(struct proc *, void *, register_t *);
#else
#endif
int	sys_getpid(struct proc *, void *, register_t *);
int	sys_mount(struct proc *, void *, register_t *);
int	sys_unmount(struct proc *, void *, register_t *);
int	sys_setuid(struct proc *, void *, register_t *);
int	sys_getuid(struct proc *, void *, register_t *);
int	sys_geteuid(struct proc *, void *, register_t *);
int	sys_ptrace(struct proc *, void *, register_t *);
int	sys_recvmsg(struct proc *, void *, register_t *);
int	sys_sendmsg(struct proc *, void *, register_t *);
int	sys_recvfrom(struct proc *, void *, register_t *);
int	sys_accept(struct proc *, void *, register_t *);
int	sys_getpeername(struct proc *, void *, register_t *);
int	sys_getsockname(struct proc *, void *, register_t *);
#ifdef COMPAT_AOUT_ALTPATH
int	aoutm68k_sys_access(struct proc *, void *, register_t *);
int	aoutm68k_sys_chflags(struct proc *, void *, register_t *);
#else
int	sys_access(struct proc *, void *, register_t *);
int	sys_chflags(struct proc *, void *, register_t *);
#endif
int	sys_fchflags(struct proc *, void *, register_t *);
int	sys_sync(struct proc *, void *, register_t *);
int	sys_kill(struct proc *, void *, register_t *);
#ifdef COMPAT_43
int	aoutm68k_compat_43_sys_stat(struct proc *, void *, register_t *);
#else
#endif
int	sys_getppid(struct proc *, void *, register_t *);
#ifdef COMPAT_43
int	aoutm68k_compat_43_sys_lstat(struct proc *, void *, register_t *);
#else
#endif
int	sys_dup(struct proc *, void *, register_t *);
int	sys_pipe(struct proc *, void *, register_t *);
int	sys_getegid(struct proc *, void *, register_t *);
int	sys_profil(struct proc *, void *, register_t *);
#if defined(KTRACE) || !defined(_KERNEL)
int	sys_ktrace(struct proc *, void *, register_t *);
#else
#endif
#ifdef COMPAT_13
int	compat_13_sys_sigaction(struct proc *, void *, register_t *);
#else
#endif
int	sys_getgid(struct proc *, void *, register_t *);
#ifdef COMPAT_13
int	compat_13_sys_sigprocmask(struct proc *, void *, register_t *);
#else
#endif
int	sys___getlogin(struct proc *, void *, register_t *);
int	sys_setlogin(struct proc *, void *, register_t *);
int	sys_acct(struct proc *, void *, register_t *);
#ifdef COMPAT_13
int	compat_13_sys_sigpending(struct proc *, void *, register_t *);
int	compat_13_sys_sigaltstack(struct proc *, void *, register_t *);
#else
#endif
int	aoutm68k_sys_ioctl(struct proc *, void *, register_t *);
#ifdef COMPAT_12
int	compat_12_sys_reboot(struct proc *, void *, register_t *);
#else
#endif
#ifdef COMPAT_AOUT_ALTPATH
int	aoutm68k_sys_revoke(struct proc *, void *, register_t *);
int	aoutm68k_sys_symlink(struct proc *, void *, register_t *);
int	aoutm68k_sys_readlink(struct proc *, void *, register_t *);
int	aoutm68k_sys_execve(struct proc *, void *, register_t *);
#else
int	sys_revoke(struct proc *, void *, register_t *);
int	sys_symlink(struct proc *, void *, register_t *);
int	sys_readlink(struct proc *, void *, register_t *);
int	sys_execve(struct proc *, void *, register_t *);
#endif
int	sys_umask(struct proc *, void *, register_t *);
#ifdef COMPAT_AOUT_ALTPATH
int	aoutm68k_sys_chroot(struct proc *, void *, register_t *);
#else
int	sys_chroot(struct proc *, void *, register_t *);
#endif
#ifdef COMPAT_43
int	aoutm68k_compat_43_sys_fstat(struct proc *, void *, register_t *);
int	compat_43_sys_getkerninfo(struct proc *, void *, register_t *);
int	compat_43_sys_getpagesize(struct proc *, void *, register_t *);
#else
#endif
#ifdef COMPAT_12
int	compat_12_sys_msync(struct proc *, void *, register_t *);
#else
#endif
int	sys_vfork(struct proc *, void *, register_t *);
int	sys_sbrk(struct proc *, void *, register_t *);
int	sys_sstk(struct proc *, void *, register_t *);
#ifdef COMPAT_43
int	compat_43_sys_mmap(struct proc *, void *, register_t *);
#else
#endif
int	sys_ovadvise(struct proc *, void *, register_t *);
int	sys_munmap(struct proc *, void *, register_t *);
int	sys_mprotect(struct proc *, void *, register_t *);
int	sys_madvise(struct proc *, void *, register_t *);
int	sys_mincore(struct proc *, void *, register_t *);
int	sys_getgroups(struct proc *, void *, register_t *);
int	sys_setgroups(struct proc *, void *, register_t *);
int	sys_getpgrp(struct proc *, void *, register_t *);
int	sys_setpgid(struct proc *, void *, register_t *);
int	sys_setitimer(struct proc *, void *, register_t *);
#ifdef COMPAT_43
int	compat_43_sys_wait(struct proc *, void *, register_t *);
#else
#endif
#ifdef COMPAT_12
int	compat_12_sys_swapon(struct proc *, void *, register_t *);
#else
#endif
int	sys_getitimer(struct proc *, void *, register_t *);
#ifdef COMPAT_43
int	compat_43_sys_gethostname(struct proc *, void *, register_t *);
int	compat_43_sys_sethostname(struct proc *, void *, register_t *);
int	compat_43_sys_getdtablesize(struct proc *, void *, register_t *);
#else
#endif
int	sys_dup2(struct proc *, void *, register_t *);
int	sys_fcntl(struct proc *, void *, register_t *);
int	sys_select(struct proc *, void *, register_t *);
int	sys_fsync(struct proc *, void *, register_t *);
int	sys_setpriority(struct proc *, void *, register_t *);
int	sys_socket(struct proc *, void *, register_t *);
int	sys_connect(struct proc *, void *, register_t *);
#ifdef COMPAT_43
int	compat_43_sys_accept(struct proc *, void *, register_t *);
#else
#endif
int	sys_getpriority(struct proc *, void *, register_t *);
#ifdef COMPAT_43
int	compat_43_sys_send(struct proc *, void *, register_t *);
int	compat_43_sys_recv(struct proc *, void *, register_t *);
#else
#endif
#ifdef COMPAT_13
int	compat_13_sys_sigreturn(struct proc *, void *, register_t *);
#else
#endif
int	sys_bind(struct proc *, void *, register_t *);
int	sys_setsockopt(struct proc *, void *, register_t *);
int	sys_listen(struct proc *, void *, register_t *);
#ifdef COMPAT_43
int	compat_43_sys_sigvec(struct proc *, void *, register_t *);
int	compat_43_sys_sigblock(struct proc *, void *, register_t *);
int	compat_43_sys_sigsetmask(struct proc *, void *, register_t *);
#else
#endif
#ifdef COMPAT_13
int	compat_13_sys_sigsuspend(struct proc *, void *, register_t *);
#else
#endif
#ifdef COMPAT_43
int	compat_43_sys_sigstack(struct proc *, void *, register_t *);
int	compat_43_sys_recvmsg(struct proc *, void *, register_t *);
int	compat_43_sys_sendmsg(struct proc *, void *, register_t *);
#else
#endif
int	sys_gettimeofday(struct proc *, void *, register_t *);
int	sys_getrusage(struct proc *, void *, register_t *);
int	sys_getsockopt(struct proc *, void *, register_t *);
int	sys_readv(struct proc *, void *, register_t *);
int	sys_writev(struct proc *, void *, register_t *);
int	sys_settimeofday(struct proc *, void *, register_t *);
int	sys_fchown(struct proc *, void *, register_t *);
int	sys_fchmod(struct proc *, void *, register_t *);
#ifdef COMPAT_43
int	compat_43_sys_recvfrom(struct proc *, void *, register_t *);
#else
#endif
int	sys_setreuid(struct proc *, void *, register_t *);
int	sys_setregid(struct proc *, void *, register_t *);
#ifdef COMPAT_AOUT_ALTPATH
int	aoutm68k_sys_rename(struct proc *, void *, register_t *);
#else
int	sys_rename(struct proc *, void *, register_t *);
#endif
#ifdef COMPAT_43
#ifdef COMPAT_AOUT_ALTPATH
int	aoutm68k_compat_43_sys_truncate(struct proc *, void *, register_t *);
#else
int	compat_43_sys_truncate(struct proc *, void *, register_t *);
#endif
int	compat_43_sys_ftruncate(struct proc *, void *, register_t *);
#else
#ifdef COMPAT_AOUT_ALTPATH
#else
#endif
#endif
int	sys_flock(struct proc *, void *, register_t *);
int	sys_mkfifo(struct proc *, void *, register_t *);
int	sys_sendto(struct proc *, void *, register_t *);
int	sys_shutdown(struct proc *, void *, register_t *);
int	sys_socketpair(struct proc *, void *, register_t *);
int	sys_mkdir(struct proc *, void *, register_t *);
#ifdef COMPAT_AOUT_ALTPATH
int	aoutm68k_sys_rmdir(struct proc *, void *, register_t *);
int	aoutm68k_sys_utimes(struct proc *, void *, register_t *);
#else
int	sys_rmdir(struct proc *, void *, register_t *);
int	sys_utimes(struct proc *, void *, register_t *);
#endif
int	sys_adjtime(struct proc *, void *, register_t *);
#ifdef COMPAT_43
int	compat_43_sys_getpeername(struct proc *, void *, register_t *);
int	compat_43_sys_gethostid(struct proc *, void *, register_t *);
int	compat_43_sys_sethostid(struct proc *, void *, register_t *);
int	compat_43_sys_getrlimit(struct proc *, void *, register_t *);
int	compat_43_sys_setrlimit(struct proc *, void *, register_t *);
int	compat_43_sys_killpg(struct proc *, void *, register_t *);
#else
#endif
int	sys_setsid(struct proc *, void *, register_t *);
int	sys_quotactl(struct proc *, void *, register_t *);
#ifdef COMPAT_43
int	compat_43_sys_quota(struct proc *, void *, register_t *);
int	compat_43_sys_getsockname(struct proc *, void *, register_t *);
#else
#endif
#if defined(NFS) || defined(NFSSERVER) || !defined(_KERNEL)
int	sys_nfssvc(struct proc *, void *, register_t *);
#else
#endif
#ifdef COMPAT_43
int	compat_43_sys_getdirentries(struct proc *, void *, register_t *);
#else
#endif
#ifdef COMPAT_AOUT_ALTPATH
int	aoutm68k_sys_statfs(struct proc *, void *, register_t *);
#else
int	sys_statfs(struct proc *, void *, register_t *);
#endif
int	sys_fstatfs(struct proc *, void *, register_t *);
#if defined(NFS) || defined(NFSSERVER) || !defined(_KERNEL)
#ifdef COMPAT_AOUT_ALTPATH
int	aoutm68k_sys_getfh(struct proc *, void *, register_t *);
#else
int	sys_getfh(struct proc *, void *, register_t *);
#endif
#else
#endif
#ifdef COMPAT_09
int	compat_09_sys_getdomainname(struct proc *, void *, register_t *);
int	compat_09_sys_setdomainname(struct proc *, void *, register_t *);
int	compat_09_sys_uname(struct proc *, void *, register_t *);
#else
#endif
int	sys_sysarch(struct proc *, void *, register_t *);
#if (defined(SYSVSEM) || !defined(_KERNEL)) && !defined(_LP64) && defined(COMPAT_10)
int	compat_10_sys_semsys(struct proc *, void *, register_t *);
#else
#endif
#if (defined(SYSVMSG) || !defined(_KERNEL)) && !defined(_LP64) && defined(COMPAT_10)
int	compat_10_sys_msgsys(struct proc *, void *, register_t *);
#else
#endif
#if (defined(SYSVSHM) || !defined(_KERNEL)) && !defined(_LP64) && defined(COMPAT_10)
int	compat_10_sys_shmsys(struct proc *, void *, register_t *);
#else
#endif
int	sys_pread(struct proc *, void *, register_t *);
int	sys_pwrite(struct proc *, void *, register_t *);
int	sys_ntp_gettime(struct proc *, void *, register_t *);
#if defined(NTP) || !defined(_KERNEL)
int	sys_ntp_adjtime(struct proc *, void *, register_t *);
#else
#endif
int	sys_setgid(struct proc *, void *, register_t *);
int	sys_setegid(struct proc *, void *, register_t *);
int	sys_seteuid(struct proc *, void *, register_t *);
#if defined(LFS) || !defined(_KERNEL)
int	sys_lfs_bmapv(struct proc *, void *, register_t *);
int	sys_lfs_markv(struct proc *, void *, register_t *);
int	sys_lfs_segclean(struct proc *, void *, register_t *);
int	sys_lfs_segwait(struct proc *, void *, register_t *);
#else
#endif
#ifdef COMPAT_12
int	aoutm68k_compat_12_sys_stat(struct proc *, void *, register_t *);
int	aoutm68k_compat_12_sys_fstat(struct proc *, void *, register_t *);
int	aoutm68k_compat_12_sys_lstat(struct proc *, void *, register_t *);
#else
#endif
#ifdef COMPAT_AOUT_ALTPATH
int	aoutm68k_sys_pathconf(struct proc *, void *, register_t *);
#else
int	sys_pathconf(struct proc *, void *, register_t *);
#endif
int	sys_fpathconf(struct proc *, void *, register_t *);
int	sys_getrlimit(struct proc *, void *, register_t *);
int	sys_setrlimit(struct proc *, void *, register_t *);
#ifdef COMPAT_12
int	compat_12_sys_getdirentries(struct proc *, void *, register_t *);
#else
#endif
int	sys_mmap(struct proc *, void *, register_t *);
int	sys_lseek(struct proc *, void *, register_t *);
#ifdef COMPAT_AOUT_ALTPATH
int	aoutm68k_sys_truncate(struct proc *, void *, register_t *);
#else
int	sys_truncate(struct proc *, void *, register_t *);
#endif
int	sys_ftruncate(struct proc *, void *, register_t *);
int	sys___sysctl(struct proc *, void *, register_t *);
int	sys_mlock(struct proc *, void *, register_t *);
int	sys_munlock(struct proc *, void *, register_t *);
#ifdef COMPAT_AOUT_ALTPATH
int	aoutm68k_sys_undelete(struct proc *, void *, register_t *);
#else
int	sys_undelete(struct proc *, void *, register_t *);
#endif
int	sys_futimes(struct proc *, void *, register_t *);
int	sys_getpgid(struct proc *, void *, register_t *);
int	sys_reboot(struct proc *, void *, register_t *);
int	sys_poll(struct proc *, void *, register_t *);
#if defined(LKM) || !defined(_KERNEL)
int	sys_lkmnosys(struct proc *, void *, register_t *);
int	sys_lkmnosys(struct proc *, void *, register_t *);
int	sys_lkmnosys(struct proc *, void *, register_t *);
int	sys_lkmnosys(struct proc *, void *, register_t *);
int	sys_lkmnosys(struct proc *, void *, register_t *);
int	sys_lkmnosys(struct proc *, void *, register_t *);
int	sys_lkmnosys(struct proc *, void *, register_t *);
int	sys_lkmnosys(struct proc *, void *, register_t *);
int	sys_lkmnosys(struct proc *, void *, register_t *);
int	sys_lkmnosys(struct proc *, void *, register_t *);
#else	/* !LKM */
#endif	/* !LKM */
#if defined(SYSVSEM) || !defined(_KERNEL)
#ifdef COMPAT_14
int	compat_14_sys___semctl(struct proc *, void *, register_t *);
#else
#endif
int	sys_semget(struct proc *, void *, register_t *);
int	sys_semop(struct proc *, void *, register_t *);
int	sys_semconfig(struct proc *, void *, register_t *);
#else
#endif
#if defined(SYSVMSG) || !defined(_KERNEL)
#ifdef COMPAT_14
int	compat_14_sys_msgctl(struct proc *, void *, register_t *);
#else
#endif
int	sys_msgget(struct proc *, void *, register_t *);
int	sys_msgsnd(struct proc *, void *, register_t *);
int	sys_msgrcv(struct proc *, void *, register_t *);
#else
#endif
#if defined(SYSVSHM) || !defined(_KERNEL)
int	sys_shmat(struct proc *, void *, register_t *);
#ifdef COMPAT_14
int	compat_14_sys_shmctl(struct proc *, void *, register_t *);
#else
#endif
int	sys_shmdt(struct proc *, void *, register_t *);
int	sys_shmget(struct proc *, void *, register_t *);
#else
#endif
int	sys_clock_gettime(struct proc *, void *, register_t *);
int	sys_clock_settime(struct proc *, void *, register_t *);
int	sys_clock_getres(struct proc *, void *, register_t *);
int	sys_nanosleep(struct proc *, void *, register_t *);
int	sys_fdatasync(struct proc *, void *, register_t *);
int	sys_mlockall(struct proc *, void *, register_t *);
int	sys_munlockall(struct proc *, void *, register_t *);
#ifdef COMPAT_AOUT_ALTPATH
int	aoutm68k_sys___posix_rename(struct proc *, void *, register_t *);
#else
int	sys___posix_rename(struct proc *, void *, register_t *);
#endif
int	sys_swapctl(struct proc *, void *, register_t *);
int	sys_getdents(struct proc *, void *, register_t *);
int	sys_minherit(struct proc *, void *, register_t *);
#ifdef COMPAT_AOUT_ALTPATH
int	aoutm68k_sys_lchmod(struct proc *, void *, register_t *);
int	aoutm68k_sys_lchown(struct proc *, void *, register_t *);
int	aoutm68k_sys_lutimes(struct proc *, void *, register_t *);
#else
int	sys_lchmod(struct proc *, void *, register_t *);
int	sys_lchown(struct proc *, void *, register_t *);
int	sys_lutimes(struct proc *, void *, register_t *);
#endif
int	sys___msync13(struct proc *, void *, register_t *);
int	aoutm68k_sys___stat13(struct proc *, void *, register_t *);
int	aoutm68k_sys___fstat13(struct proc *, void *, register_t *);
int	aoutm68k_sys___lstat13(struct proc *, void *, register_t *);
int	sys___sigaltstack14(struct proc *, void *, register_t *);
int	sys___vfork14(struct proc *, void *, register_t *);
#ifdef COMPAT_AOUT_ALTPATH
int	aoutm68k_sys___posix_chown(struct proc *, void *, register_t *);
#else
int	sys___posix_chown(struct proc *, void *, register_t *);
#endif
int	sys___posix_fchown(struct proc *, void *, register_t *);
int	sys___posix_lchown(struct proc *, void *, register_t *);
int	sys_getsid(struct proc *, void *, register_t *);
#if defined(KTRACE) || !defined(_KERNEL)
int	sys_fktrace(struct proc *, void *, register_t *);
#else
#endif
int	sys_preadv(struct proc *, void *, register_t *);
int	sys_pwritev(struct proc *, void *, register_t *);
int	sys___sigaction14(struct proc *, void *, register_t *);
int	sys___sigpending14(struct proc *, void *, register_t *);
int	sys___sigprocmask14(struct proc *, void *, register_t *);
int	sys___sigsuspend14(struct proc *, void *, register_t *);
int	sys___sigreturn14(struct proc *, void *, register_t *);
int	sys___getcwd(struct proc *, void *, register_t *);
int	sys_fchroot(struct proc *, void *, register_t *);
int	sys_fhopen(struct proc *, void *, register_t *);
int	aoutm68k_sys_fhstat(struct proc *, void *, register_t *);
int	sys_fhstatfs(struct proc *, void *, register_t *);
#if defined(SYSVSEM) || !defined(_KERNEL)
int	sys_____semctl13(struct proc *, void *, register_t *);
#else
#endif
#if defined(SYSVMSG) || !defined(_KERNEL)
int	sys___msgctl13(struct proc *, void *, register_t *);
#else
#endif
#if defined(SYSVSHM) || !defined(_KERNEL)
int	sys___shmctl13(struct proc *, void *, register_t *);
#else
#endif
int	sys_lchflags(struct proc *, void *, register_t *);
int	sys_issetugid(struct proc *, void *, register_t *);
#endif /* _AOUTM68K_SYS__SYSCALLARGS_H_ */
