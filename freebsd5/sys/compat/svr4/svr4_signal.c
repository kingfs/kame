/*
 * Copyright (c) 1998 Mark Newton
 * Copyright (c) 1994 Christos Zoulas
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
 * 3. The name of the author may not be used to endorse or promote products
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
 * 
 * $FreeBSD: src/sys/compat/svr4/svr4_signal.c,v 1.21.2.1 2002/12/19 09:40:07 alfred Exp $
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/filedesc.h>
#include <sys/lock.h>
#include <sys/mutex.h>
#include <sys/proc.h>
#include <sys/signal.h>
#include <sys/signalvar.h>
#include <sys/sysproto.h>

#include <machine/cpu.h>

#include <compat/svr4/svr4.h>
#include <compat/svr4/svr4_types.h>
#include <compat/svr4/svr4_signal.h>
#include <compat/svr4/svr4_proto.h>
#include <compat/svr4/svr4_util.h>
#include <compat/svr4/svr4_ucontext.h>

#define	svr4_sigmask(n)		(1 << (((n) - 1) & 31))
#define	svr4_sigword(n)		(((n) - 1) >> 5)
#define svr4_sigemptyset(s)	memset((s), 0, sizeof(*(s)))
#define	svr4_sigismember(s, n)	((s)->bits[svr4_sigword(n)] & svr4_sigmask(n))
#define	svr4_sigaddset(s, n)	((s)->bits[svr4_sigword(n)] |= svr4_sigmask(n))

void svr4_to_bsd_sigaction(const struct svr4_sigaction *, struct sigaction *);
void bsd_to_svr4_sigaction(const struct sigaction *, struct svr4_sigaction *);
void svr4_sigfillset(svr4_sigset_t *);

int bsd_to_svr4_sig[SVR4_SIGTBLSZ] = {
	SVR4_SIGHUP,
	SVR4_SIGINT,
	SVR4_SIGQUIT,
	SVR4_SIGILL,
	SVR4_SIGTRAP,
	SVR4_SIGABRT,
	SVR4_SIGEMT,
	SVR4_SIGFPE,
	SVR4_SIGKILL,
	SVR4_SIGBUS,
	SVR4_SIGSEGV,
	SVR4_SIGSYS,
	SVR4_SIGPIPE,
	SVR4_SIGALRM,
	SVR4_SIGTERM,
	SVR4_SIGURG,
	SVR4_SIGSTOP,
	SVR4_SIGTSTP,
	SVR4_SIGCONT,
	SVR4_SIGCHLD,
	SVR4_SIGTTIN,
	SVR4_SIGTTOU,
	SVR4_SIGIO,
	SVR4_SIGXCPU,
	SVR4_SIGXFSZ,
	SVR4_SIGVTALRM,
	SVR4_SIGPROF,
	SVR4_SIGWINCH,
	0,			/* SIGINFO */
	SVR4_SIGUSR1,
	SVR4_SIGUSR2,
};

int svr4_to_bsd_sig[SVR4_SIGTBLSZ] = {
	SIGHUP,
	SIGINT,
	SIGQUIT,
	SIGILL,
	SIGTRAP,
	SIGABRT,
	SIGEMT,
	SIGFPE,
	SIGKILL,
	SIGBUS,
	SIGSEGV,
	SIGSYS,
	SIGPIPE,
	SIGALRM,
	SIGTERM,
	SIGUSR1,
	SIGUSR2,
	SIGCHLD,
	0,		/* XXX NetBSD uses SIGPWR here, but we don't seem to have one */
	SIGWINCH,
	SIGURG,
	SIGIO,
	SIGSTOP,
	SIGTSTP,
	SIGCONT,
	SIGTTIN,
	SIGTTOU,
	SIGVTALRM,
	SIGPROF,
	SIGXCPU,
	SIGXFSZ,
};

void
svr4_sigfillset(s)
	svr4_sigset_t *s;
{
	int i;

	svr4_sigemptyset(s);
	for (i = 0; i < SVR4_NSIG; i++) 
		if (svr4_to_bsd_sig[i] != 0)
			svr4_sigaddset(s, i);
}

void
svr4_to_bsd_sigset(sss, bss)
	const svr4_sigset_t *sss;
	sigset_t *bss;
{
	int i, newsig;

	SIGEMPTYSET(*bss);
	for (i = 0; i < SVR4_NSIG; i++)
		if (svr4_sigismember(sss, i + 1)) {
			newsig = svr4_to_bsd_sig[i];
			if (newsig)
				SIGADDSET(*bss, newsig);
		}
}

void
bsd_to_svr4_sigset(bss, sss)
	const sigset_t *bss;
	svr4_sigset_t *sss;
{
	int i, newsig;

	svr4_sigemptyset(sss);
	sss->bits[0] = bss->__bits[0] & ~((1U << SVR4_SIGTBLSZ) - 1);
	sss->bits[1] = bss->__bits[1];
	sss->bits[2] = bss->__bits[2];
	sss->bits[3] = bss->__bits[3];
	for (i = 1; i <= SVR4_SIGTBLSZ; i++) {
		if (SIGISMEMBER(*bss, i)) {
			newsig = bsd_to_svr4_sig[_SIG_IDX(i)];
			if (newsig)
				svr4_sigaddset(sss, newsig);
		}
	}
}

/*
 * XXX: Only a subset of the flags is currently implemented.
 */
void
svr4_to_bsd_sigaction(ssa, bsa)
	const struct svr4_sigaction *ssa;
	struct sigaction *bsa;
{

	bsa->sa_handler = (sig_t) ssa->ssa_handler;
	svr4_to_bsd_sigset(&ssa->ssa_mask, &bsa->sa_mask);
	bsa->sa_flags = 0;
	if ((ssa->ssa_flags & SVR4_SA_ONSTACK) != 0)
		bsa->sa_flags |= SA_ONSTACK;
	if ((ssa->ssa_flags & SVR4_SA_RESETHAND) != 0)
		bsa->sa_flags |= SA_RESETHAND;
	if ((ssa->ssa_flags & SVR4_SA_RESTART) != 0)
		bsa->sa_flags |= SA_RESTART;
	if ((ssa->ssa_flags & SVR4_SA_SIGINFO) != 0)
		DPRINTF(("svr4_to_bsd_sigaction: SA_SIGINFO ignored\n"));
	if ((ssa->ssa_flags & SVR4_SA_NOCLDSTOP) != 0)
		bsa->sa_flags |= SA_NOCLDSTOP;
	if ((ssa->ssa_flags & SVR4_SA_NODEFER) != 0)
		bsa->sa_flags |= SA_NODEFER;
	if ((ssa->ssa_flags & SVR4_SA_NOCLDWAIT) != 0)
		bsa->sa_flags |= SA_NOCLDWAIT;
	if ((ssa->ssa_flags & ~SVR4_SA_ALLBITS) != 0)
		DPRINTF(("svr4_to_bsd_sigaction: extra bits ignored\n"));
}

void
bsd_to_svr4_sigaction(bsa, ssa)
	const struct sigaction *bsa;
	struct svr4_sigaction *ssa;
{

	ssa->ssa_handler = (svr4_sig_t) bsa->sa_handler;
	bsd_to_svr4_sigset(&bsa->sa_mask, &ssa->ssa_mask);
	ssa->ssa_flags = 0;
	if ((bsa->sa_flags & SA_ONSTACK) != 0)
		ssa->ssa_flags |= SVR4_SA_ONSTACK;
	if ((bsa->sa_flags & SA_RESETHAND) != 0)
		ssa->ssa_flags |= SVR4_SA_RESETHAND;
	if ((bsa->sa_flags & SA_RESTART) != 0)
		ssa->ssa_flags |= SVR4_SA_RESTART;
	if ((bsa->sa_flags & SA_NODEFER) != 0)
		ssa->ssa_flags |= SVR4_SA_NODEFER;
	if ((bsa->sa_flags & SA_NOCLDSTOP) != 0)
		ssa->ssa_flags |= SVR4_SA_NOCLDSTOP;
}

void
svr4_to_bsd_sigaltstack(sss, bss)
	const struct svr4_sigaltstack *sss;
	struct sigaltstack *bss;
{

	bss->ss_sp = sss->ss_sp;
	bss->ss_size = sss->ss_size;
	bss->ss_flags = 0;
	if ((sss->ss_flags & SVR4_SS_DISABLE) != 0)
		bss->ss_flags |= SS_DISABLE;
	if ((sss->ss_flags & SVR4_SS_ONSTACK) != 0)
		bss->ss_flags |= SS_ONSTACK;
	if ((sss->ss_flags & ~SVR4_SS_ALLBITS) != 0)
	  /*XXX*/ uprintf("svr4_to_bsd_sigaltstack: extra bits ignored\n");
}

void
bsd_to_svr4_sigaltstack(bss, sss)
	const struct sigaltstack *bss;
	struct svr4_sigaltstack *sss;
{

	sss->ss_sp = bss->ss_sp;
	sss->ss_size = bss->ss_size;
	sss->ss_flags = 0;
	if ((bss->ss_flags & SS_DISABLE) != 0)
		sss->ss_flags |= SVR4_SS_DISABLE;
	if ((bss->ss_flags & SS_ONSTACK) != 0)
		sss->ss_flags |= SVR4_SS_ONSTACK;
}

int
svr4_sys_sigaction(td, uap)
	register struct thread *td;
	struct svr4_sys_sigaction_args *uap;
{
	struct svr4_sigaction *nisa, *oisa, tmpisa;
	struct sigaction *nbsa, *obsa, tmpbsa;
	struct sigaction_args sa;
	caddr_t sg;
	int error;

	DPRINTF(("@@@ svr4_sys_sigaction(%d, %d, %d)\n", td->td_proc->p_pid,
			uap->signum,
			SVR4_SVR42BSD_SIG(uap->signum)));
	
	sg = stackgap_init();
	nisa = uap->nsa;
	oisa = uap->osa;

	if (oisa != NULL)
		obsa = stackgap_alloc(&sg, sizeof(struct sigaction));
	else
		obsa = NULL;

	if (nisa != NULL) {
		nbsa = stackgap_alloc(&sg, sizeof(struct sigaction));
		if ((error = copyin(nisa, &tmpisa, sizeof(tmpisa))) != 0)
			return error;
		svr4_to_bsd_sigaction(&tmpisa, &tmpbsa);
		if ((error = copyout(&tmpbsa, nbsa, sizeof(tmpbsa))) != 0)
			return error;
	} else
		nbsa = NULL;

#if defined(DEBUG_SVR4)
	{
		int i;
		for (i = 0; i < 4; i++) 
			DPRINTF(("\tssa_mask[%d] = %lx\n", i,
						nisa->ssa_mask.bits[i]));
		DPRINTF(("\tssa_handler = %p\n", nisa->ssa_handler));
	}
#endif

	sa.sig = SVR4_SVR42BSD_SIG(uap->signum);
	sa.act = nbsa;
	sa.oact = obsa;

	if ((error = sigaction(td, &sa)) != 0)
		return error;

	if (oisa != NULL) {
		if ((error = copyin(obsa, &tmpbsa, sizeof(tmpbsa))) != 0)
			return error;
		bsd_to_svr4_sigaction(&tmpbsa, &tmpisa);
		if ((error = copyout(&tmpisa, oisa, sizeof(tmpisa))) != 0)
			return error;
	}

	return 0;
}

int 
svr4_sys_sigaltstack(td, uap)
	register struct thread *td;
	struct svr4_sys_sigaltstack_args *uap;
{
	struct svr4_sigaltstack *nsss, *osss, tmpsss;
	struct sigaltstack *nbss, *obss, tmpbss;
	struct sigaltstack_args sa;
	caddr_t sg;
	int error, *retval;

	retval = td->td_retval;
	sg = stackgap_init();
	nsss = uap->nss;
	osss = uap->oss;

	if (osss != NULL)
		obss = stackgap_alloc(&sg, sizeof(struct sigaltstack));
	else
		obss = NULL;

	if (nsss != NULL) {
		nbss = stackgap_alloc(&sg, sizeof(struct sigaltstack));
		if ((error = copyin(nsss, &tmpsss, sizeof(tmpsss))) != 0)
			return error;
		svr4_to_bsd_sigaltstack(&tmpsss, &tmpbss);
		if ((error = copyout(&tmpbss, nbss, sizeof(tmpbss))) != 0)
			return error;
	} else
		nbss = NULL;

	sa.ss = nbss;
	sa.oss = obss;

	if ((error = sigaltstack(td, &sa)) != 0)
		return error;

	if (obss != NULL) {
		if ((error = copyin(obss, &tmpbss, sizeof(tmpbss))) != 0)
			return error;
		bsd_to_svr4_sigaltstack(&tmpbss, &tmpsss);
		if ((error = copyout(&tmpsss, osss, sizeof(tmpsss))) != 0)
			return error;
	}

	return 0;
}

/*
 * Stolen from the ibcs2 one
 */
int
svr4_sys_signal(td, uap)
	register struct thread *td;
	struct svr4_sys_signal_args *uap;
{
	int signum;
	int error, *retval = td->td_retval;
	caddr_t sg = stackgap_init();

	DPRINTF(("@@@ svr4_sys_signal(%d)\n", td->td_proc->p_pid));

	signum = SVR4_SVR42BSD_SIG(SVR4_SIGNO(uap->signum));
	if (signum <= 0 || signum > SVR4_NSIG)
		return (EINVAL);

	switch (SVR4_SIGCALL(uap->signum)) {
	case SVR4_SIGDEFER_MASK:
		if (uap->handler == SVR4_SIG_HOLD)
			goto sighold;
		/* FALLTHROUGH */

	case SVR4_SIGNAL_MASK:
		{
			struct sigaction_args sa_args;
			struct sigaction *nbsa, *obsa, sa;

			nbsa = stackgap_alloc(&sg, sizeof(struct sigaction));
			obsa = stackgap_alloc(&sg, sizeof(struct sigaction));
			sa_args.sig = signum;
			sa_args.act = nbsa;
			sa_args.oact = obsa;

			sa.sa_handler = (sig_t) uap->handler;
			SIGEMPTYSET(sa.sa_mask);
			sa.sa_flags = 0;

			if (signum != SIGALRM)
				sa.sa_flags = SA_RESTART;

			if ((error = copyout(&sa, nbsa, sizeof(sa))) != 0)
				return error;
			if ((error = sigaction(td, &sa_args)) != 0) {
				DPRINTF(("signal: sigaction failed: %d\n",
					 error));
				*retval = (int)SVR4_SIG_ERR;
				return error;
			}
			if ((error = copyin(obsa, &sa, sizeof(sa))) != 0)
				return error;
			*retval = (int)sa.sa_handler;
			return 0;
		}

	case SVR4_SIGHOLD_MASK:
sighold:
		{
			struct sigprocmask_args sa;
			sigset_t *set;

			set = stackgap_alloc(&sg, sizeof(sigset_t));
			SIGEMPTYSET(*set);
			SIGADDSET(*set, signum);
			sa.how = SIG_BLOCK;
			sa.set = set;
			sa.oset = NULL;
			return sigprocmask(td, &sa);
		}

	case SVR4_SIGRELSE_MASK:
		{
			struct sigprocmask_args sa;
			sigset_t *set;

			set = stackgap_alloc(&sg, sizeof(sigset_t));
			SIGEMPTYSET(*set);
			SIGADDSET(*set, signum);
			sa.how = SIG_UNBLOCK;
			sa.set = set;
			sa.oset = NULL;
			return sigprocmask(td, &sa);
		}

	case SVR4_SIGIGNORE_MASK:
		{
			struct sigaction_args sa_args;
			struct sigaction *bsa, sa;

			bsa = stackgap_alloc(&sg, sizeof(struct sigaction));
			sa_args.sig = signum;
			sa_args.act = bsa;
			sa_args.oact = NULL;

			sa.sa_handler = SIG_IGN;
			SIGEMPTYSET(sa.sa_mask);
			sa.sa_flags = 0;
			if ((error = copyout(&sa, bsa, sizeof(sa))) != 0)
				return error;
			if ((error = sigaction(td, &sa_args)) != 0) {
				DPRINTF(("sigignore: sigaction failed\n"));
				return error;
			}
			return 0;
		}

	case SVR4_SIGPAUSE_MASK:
		{
			struct sigsuspend_args sa;
			sigset_t *set;

			set = stackgap_alloc(&sg, sizeof(sigset_t));
			PROC_LOCK(td->td_proc);
			*set = td->td_proc->p_sigmask;
			PROC_UNLOCK(td->td_proc);
			SIGDELSET(*set, signum);
			sa.sigmask = set;
			return sigsuspend(td, &sa);
		}

	default:
		return (ENOSYS);
	}
}


int
svr4_sys_sigprocmask(td, uap)
	struct thread *td;
	struct svr4_sys_sigprocmask_args *uap;
{
	svr4_sigset_t sss;
	sigset_t bss;
	int error = 0, *retval;

	retval = td->td_retval;
	if (uap->oset != NULL) {
		/* Fix the return value first if needed */
		PROC_LOCK(td->td_proc);
		bsd_to_svr4_sigset(&td->td_proc->p_sigmask, &sss);
		PROC_UNLOCK(td->td_proc);
		if ((error = copyout(&sss, uap->oset, sizeof(sss))) != 0)
			return error;
	}

	if (uap->set == NULL)
		/* Just examine */
		return 0;

	if ((error = copyin(uap->set, &sss, sizeof(sss))) != 0)
		return error;

	svr4_to_bsd_sigset(&sss, &bss);

	PROC_LOCK(td->td_proc);
	switch (uap->how) {
	case SVR4_SIG_BLOCK:
		SIGSETOR(td->td_proc->p_sigmask, bss);
		SIG_CANTMASK(td->td_proc->p_sigmask);
		break;

	case SVR4_SIG_UNBLOCK:
		SIGSETNAND(td->td_proc->p_sigmask, bss);
		signotify(td->td_proc);
		break;

	case SVR4_SIG_SETMASK:
		td->td_proc->p_sigmask = bss;
		SIG_CANTMASK(td->td_proc->p_sigmask);
		signotify(td->td_proc);
		break;

	default:
		error = EINVAL;
		break;
	}
	PROC_UNLOCK(td->td_proc);

	return error;
}

int
svr4_sys_sigpending(td, uap)
	struct thread *td;
	struct svr4_sys_sigpending_args *uap;
{
	sigset_t bss;
	int *retval;
	svr4_sigset_t sss;

	DPRINTF(("@@@ svr4_sys_sigpending(%d)\n", td->td_proc->p_pid));
	retval = td->td_retval;
	switch (uap->what) {
	case 1:	/* sigpending */
		if (uap->mask == NULL)
			return 0;
		PROC_LOCK(td->td_proc);
		bss = td->td_proc->p_siglist;
		SIGSETAND(bss, td->td_proc->p_sigmask);
		PROC_UNLOCK(td->td_proc);
		bsd_to_svr4_sigset(&bss, &sss);
		break;

	case 2:	/* sigfillset */
		svr4_sigfillset(&sss);
#if defined(DEBUG_SVR4)
		{
			int i;
			for (i = 0; i < 4; i++)
				DPRINTF(("new sigset[%d] = %lx\n", i, (long)sss.bits[i]));
		}
#endif
		break;

	default:
		return EINVAL;
	}
		
	return copyout(&sss, uap->mask, sizeof(sss));
}

int
svr4_sys_sigsuspend(td, uap)
	register struct thread *td;
	struct svr4_sys_sigsuspend_args *uap;
{
	svr4_sigset_t sss;
	sigset_t *bss;
	struct sigsuspend_args sa;
	int error;
	caddr_t sg = stackgap_init();

	if ((error = copyin(uap->ss, &sss, sizeof(sss))) != 0)
		return error;

	bss = stackgap_alloc(&sg, sizeof(sigset_t));
	svr4_to_bsd_sigset(&sss, bss);

	sa.sigmask = bss;
	return sigsuspend(td, &sa);
}


int
svr4_sys_kill(td, uap)
	register struct thread *td;
	struct svr4_sys_kill_args *uap;
{
	struct kill_args ka;

	ka.pid = uap->pid;
	ka.signum = SVR4_SVR42BSD_SIG(uap->signum);
	return kill(td, &ka);
}


int 
svr4_sys_context(td, uap)
	register struct thread *td;
	struct svr4_sys_context_args *uap;
{
	struct svr4_ucontext uc;
	int error;

	switch (uap->func) {
	case 0:
		PROC_LOCK(td->td_proc);
		DPRINTF(("getcontext(%p)\n", uap->uc));
		svr4_getcontext(td, &uc, &td->td_proc->p_sigmask,
		    sigonstack(cpu_getstack(td)));
		PROC_UNLOCK(td->td_proc);
		return copyout(&uc, uap->uc, sizeof(uc));

	case 1: 
		DPRINTF(("setcontext(%p)\n", uap->uc));
		if ((error = copyin(uap->uc, &uc, sizeof(uc))) != 0)
			return error;
		DPRINTF(("uc_flags = %lx\n", uc.uc_flags));
#if defined(DEBUG_SVR4)
		{
			int i;
			for (i = 0; i < 4; i++)
				DPRINTF(("uc_sigmask[%d] = %lx\n", i,
							uc.uc_sigmask.bits[i]));
		}
#endif
		return svr4_setcontext(td, &uc);

	default:
		DPRINTF(("context(%d, %p)\n", uap->func,
		    uap->uc));
		return ENOSYS;
	}
	return 0;
}

int
svr4_sys_pause(td, uap)
	register struct thread *td;
	struct svr4_sys_pause_args *uap;
{
	struct sigsuspend_args bsa;

	bsa.sigmask = &td->td_proc->p_sigmask;
	return sigsuspend(td, &bsa);
}
