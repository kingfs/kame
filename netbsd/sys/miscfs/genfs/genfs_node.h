/* $NetBSD: genfs_node.h,v 1.3 2001/12/18 07:49:36 chs Exp $ */

/*
 * Copyright (c) 2001 Chuck Silvers.
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
 *      This product includes software developed by Chuck Silvers.
 * 4. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
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

#ifndef	_MISCFS_GENFS_GENFS_NODE_H_
#define	_MISCFS_GENFS_GENFS_NODE_H_

struct vm_page;

struct genfs_ops {
	void	(*gop_size)(struct vnode *, off_t, off_t *);
	int	(*gop_alloc)(struct vnode *, off_t, off_t, int, struct ucred *);
	int	(*gop_write)(struct vnode *, struct vm_page **, int, int);
};

#define GOP_SIZE(vp, size, eobp) \
	(*VTOG(vp)->g_op->gop_size)((vp), (size), (eobp))
#define GOP_ALLOC(vp, off, len, flags, cred) \
	(*VTOG(vp)->g_op->gop_alloc)((vp), (off), (len), (flags), (cred))
#define GOP_WRITE(vp, pgs, npages, flags) \
	(*VTOG(vp)->g_op->gop_write)((vp), (pgs), (npages), (flags))

struct genfs_node {
	struct genfs_ops	*g_op;		/* ops vector */
	struct lock		g_glock;	/* getpages lock */
};

#define VTOG(vp) ((struct genfs_node *)(vp)->v_data)

void	genfs_size(struct vnode *, off_t, off_t *);
void	genfs_node_init(struct vnode *, struct genfs_ops *);
int	genfs_gop_write(struct vnode *, struct vm_page **, int, int);
int	genfs_compat_gop_write(struct vnode *, struct vm_page **, int, int);

#endif	/* _MISCFS_GENFS_GENFS_NODE_H_ */
