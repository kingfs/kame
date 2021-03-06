/* $NetBSD: varargs.h,v 1.3 2000/02/03 16:16:10 kleink Exp $ */

#ifndef _SH3_VARARGS_H_
#define _SH3_VARARGS_H_

#define _VARARGS_H

#include "sh3/va-sh.h"
#include <sys/featuretest.h>

typedef __gnuc_va_list va_list;

#if !defined(_ANSI_SOURCE) && \
    (!defined(_POSIX_C_SOURCE) && !defined(_XOPEN_SOURCE) || \
     defined(_ISOC99_SOURCE) || (__STDC_VERSION__ - 0) >= 199901L)
#define	va_copy		__va_copy
#endif

#endif /* _SH3_VARARGS_H_ */
