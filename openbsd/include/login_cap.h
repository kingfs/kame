/*	$OpenBSD: login_cap.h,v 1.1 2000/08/20 18:37:20 millert Exp $	*/

/*-
 * Copyright (c) 1995,1997 Berkeley Software Design, Inc. All rights reserved.
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
 *	This product includes software developed by Berkeley Software Design,
 *	Inc.
 * 4. The name of Berkeley Software Design, Inc.  may not be used to endorse
 *    or promote products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY BERKELEY SOFTWARE DESIGN, INC. ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL BERKELEY SOFTWARE DESIGN, INC. BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	BSDI $From: login_cap.h,v 2.11 1999/09/08 18:11:57 prb Exp $
 */

#define	LOGIN_DEFCLASS		"default"
#define	LOGIN_DEFSTYLE		"passwd"
#define	LOGIN_DEFSERVICE	"login"
#define	LOGIN_DEFUMASK		022
#define	_PATH_LOGIN_CONF	"/etc/login.conf"
#define	_PATH_AUTHPROG		"/usr/libexec/auth/login_"

#define	LOGIN_SETGROUP		0x0001	/* Set group */
#define	LOGIN_SETLOGIN		0x0002	/* Set login */
#define	LOGIN_SETPATH		0x0004	/* Set path */
#define	LOGIN_SETPRIORITY	0x0008	/* Set priority */
#define	LOGIN_SETRESOURCES	0x0010	/* Set resource limits */
#define	LOGIN_SETUMASK		0x0020	/* Set umask */
#define	LOGIN_SETUSER		0x0040	/* Set user */
#define	LOGIN_SETALL 		0x007f	/* Set all. */

#define	BI_AUTH		"authorize"		/* Accepted authentication */
#define	BI_REJECT	"reject"		/* Rejected authentication */
#define	BI_CHALLENGE	"reject challenge"	/* Reject with a challenge */
#define	BI_SILENT	"reject silent"		/* Reject silently */
#define	BI_REMOVE	"remove"		/* remove file on error */
#define	BI_ROOTOKAY	"authorize root"	/* root authenticated */
#define	BI_SECURE	"authorize secure"	/* okay on non-secure line */
#define	BI_SETENV	"setenv"		/* set environment variable */
#define	BI_UNSETENV	"unsetenv"		/* unset environment variable */
#define	BI_VALUE	"value"			/* set local variable */
#define	BI_EXPIRED	"reject expired"	/* account expired */
#define	BI_PWEXPIRED	"reject pwexpired"	/* password expired */

/*
 * bits which can be returned by authenticate()/auth_scan()
 */
#define	AUTH_OKAY	0x01			/* user authentciated */
#define	AUTH_ROOTOKAY	0x02			/* authenticated as root */
#define	AUTH_SECURE	0x04			/* secure login */
#define	AUTH_SILENT	0x08			/* silent rejection */
#define	AUTH_CHALLENGE	0x10			/* a challenge was given */
#define	AUTH_EXPIRED	0x20			/* account expired */
#define	AUTH_PWEXPIRED	0x40			/* password expired */

#define	AUTH_ALLOW	(AUTH_OKAY | AUTH_ROOTOKAY | AUTH_SECURE)

typedef struct {
	char	*lc_class;
	char	*lc_cap;
	char	*lc_style;
} login_cap_t;

#include <sys/cdefs.h>
__BEGIN_DECLS
struct passwd;

login_cap_t *login_getclass __P((char *));
void	 login_close __P((login_cap_t *));
int	 login_getcapbool __P((login_cap_t *, char *, u_int));
quad_t	 login_getcapnum __P((login_cap_t *, char *, quad_t, quad_t));
quad_t	 login_getcapsize __P((login_cap_t *, char *, quad_t, quad_t));
char	*login_getcapstr __P((login_cap_t *, char *, char *, char *));
quad_t	 login_getcaptime __P((login_cap_t *, char *, quad_t, quad_t));
char	*login_getstyle __P((login_cap_t *, char *, char *));

int	secure_path __P((char *));
int	setclasscontext __P((char *, u_int));
int	setusercontext __P((login_cap_t *, struct passwd *, uid_t, u_int));

/*
 * Routines for authentication
 * Most of these will be deprecated in a future release
 */
int	auth_approve __P((login_cap_t *, char *, char *));
int	auth_cat __P((char *));
int	auth_check __P((char *, char *, char *, char *, int *));
void	auth_checknologin __P((login_cap_t *));
void	auth_env __P((void));
char	*auth_mkvalue __P((char *));
int	auth_response __P((char *, char *, char *, char *, int *, char *, char *));
void	auth_rmfiles __P((void));
int	auth_scan __P((int));
int	auth_script __P((char *, ...));
int	auth_script_data __P((char *, int, char *, ...));
char	*auth_value __P((char *));
int	auth_setopt __P((char *, char *));
void	auth_clropts __P((void));
__END_DECLS
