#	$NetBSD: Makefile,v 1.29 1999/04/01 05:12:20 simonb Exp $

LIB=	sa
MKPIC=	no
MKPROFILE=no

SA_USE_CREAD?= no
SA_INCLUDE_NET?= yes

#DEBUGCPPFLAGS= -DBOOTP_DEBUG -DNETIF_DEBUG -DETHER_DEBUG -DNFS_DEBUG -DRPC_DEBUG -DRARP_DEBUG -DDEBUG -DPARANOID
CPPFLAGS=	-I. ${SACPPFLAGS} ${SAMISCCPPFLAGS} \
		-DCOMPAT_UFS ${DEBUGCPPFLAGS}

#COPTS+= -ansi -pedantic -Wall

# stand routines
SRCS+=	alloc.c bcmp.c bcopy.c bzero.c errno.c exit.c exec.c getfile.c gets.c \
	globals.c memcmp.c memcpy.c memset.c panic.c printf.c snprintf.c \
	sprintf.c strerror.c subr_prf.c twiddle.c vsprintf.c

# io routines
SRCS+=	closeall.c dev.c disklabel.c dkcksum.c ioctl.c nullfs.c stat.c fstat.c
SRCS+=	close.c lseek.c open.c read.c write.c
.if (${SA_USE_CREAD} == "yes")
CPPFLAGS+= -D__INTERNAL_LIBSA_CREAD
SRCS+=	cread.c
.endif

.if (${SA_INCLUDE_NET} == "yes")
# network routines
SRCS+=	arp.c ether.c in_cksum.c net.c netif.c rpc.c

# network info services:
SRCS+=	bootp.c rarp.c bootparam.c

# boot filesystems
SRCS+=	nfs.c tftp.c
.endif
SRCS+=	ufs.c cd9660.c ustarfs.c

# only needed during build
libinstall::

.include <bsd.lib.mk>

lib${LIB}.o:: ${OBJS}
	@echo building standard ${LIB} library
	@rm -f lib${LIB}.o
	@${LD} -r -o lib${LIB}.o `lorder ${OBJS} | tsort`
