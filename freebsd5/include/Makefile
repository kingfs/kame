#	@(#)Makefile	8.2 (Berkeley) 1/4/94
# $FreeBSD: src/include/Makefile,v 1.191.2.1 2003/01/05 08:52:42 simokawa Exp $
#
# Doing a make install builds /usr/include
#
# The ``rm -rf''s used below are safe because rm doesn't follow symbolic
# links.

CLEANFILES= osreldate.h version vers.c
SUBDIR= arpa protocols rpcsvc rpc
INCS=	a.out.h ar.h assert.h bitstring.h complex.h cpio.h ctype.h db.h \
	dirent.h dlfcn.h elf.h elf-hints.h err.h fmtmsg.h fnmatch.h fstab.h \
	fts.h getopt.h glob.h grp.h hesiod.h histedit.h ieeefp.h ifaddrs.h \
	inttypes.h iso646.h kenv.h langinfo.h libgen.h limits.h link.h \
	locale.h malloc.h memory.h monetary.h mpool.h ndbm.h netconfig.h \
	netdb.h nl_types.h nlist.h nsswitch.h objformat.h paths.h pthread.h \
	pthread_np.h pwd.h ranlib.h readpassphrase.h regex.h regexp.h \
	resolv.h rune.h runetype.h search.h setjmp.h sgtty.h signal.h stab.h \
	stdbool.h stddef.h stdio.h stdlib.h strhash.h string.h stringlist.h \
	strings.h sysexits.h tar.h time.h timeconv.h timers.h ttyent.h \
	ulimit.h unistd.h utime.h utmp.h uuid.h vis.h wchar.h wctype.h

MHDRS=	float.h floatingpoint.h stdarg.h varargs.h

# posix4/mqueue.h is useless without an implementation and isn't installed:
PHDRS=	sched.h semaphore.h _semaphore.h # mqueue.h

LHDRS=	aio.h errno.h fcntl.h linker_set.h poll.h stdint.h syslog.h \
	termios.h ucontext.h

LDIRS=	cam net netatalk netatm netgraph netinet netinet6 netipsec \
	netipx netkey netnatm netncp netns netsmb nfs nfsclient nfsserver \
	pccard posix4 sys vm
LDIRS+=	altq

LSUBDIRS=	cam/scsi dev/an dev/ic dev/iicbus dev/firewire dev/ofw \
	dev/ppbus dev/smbus dev/usb dev/wi fs/devfs \
	fs/fdescfs fs/fifofs fs/msdosfs fs/ntfs fs/nullfs fs/nwfs fs/portalfs \
	fs/procfs fs/smbfs fs/umapfs fs/unionfs isofs/cd9660 \
	netatm/ipatm netatm/sigpvc netatm/spans netatm/uni \
	netgraph/bluetooth/include \
	security/mac_biba security/mac_bsdextended security/mac_lomac\
	security/mac_mls security/mac_partition ufs/ffs ufs/ufs

# For SHARED=symlinks, cam, netatm, and netgraph are symlinks, so cam/scsi,
# netatm/*, and netgraph/* are taken care of
LSYMSUBDIRS=	${LSUBDIRS:Ncam/scsi:Nnetatm/*:Nnetgraph/*}

# Define SHARED to indicate whether you want symbolic links to the system
# source (``symlinks''), or a separate copy (``copies'').  ``symlinks'' is
# probably only useful for developers and should be avoided if you do not
# wish to tie your /usr/include and /usr/src together.
#SHARED=	symlinks
SHARED?=	copies

INCS+=	osreldate.h

osreldate.h:	${.CURDIR}/../sys/conf/newvers.sh \
		${.CURDIR}/../sys/sys/param.h \
		${.CURDIR}/Makefile
	@${ECHO} creating osreldate.h from newvers.sh
	setvar PARAMFILE ${.CURDIR}/../sys/sys/param.h; \
	. ${.CURDIR}/../sys/conf/newvers.sh;			\
	echo "$$COPYRIGHT" > osreldate.h;			\
	echo "#ifdef _KERNEL" >> osreldate.h;			\
	echo '#error "/usr/include/osreldate.h cannot be used in the kernel, use sys/param.h"' >> osreldate.h; \
	echo "#else" >> osreldate.h;				\
	echo \#'undef __FreeBSD_version' >> osreldate.h;	\
	echo \#'define __FreeBSD_version' $$RELDATE >> osreldate.h; \
	echo "#endif" >> osreldate.h

.for i in ${LHDRS}
INCSLINKS+=	sys/$i ${INCLUDEDIR}/$i
.endfor
.for i in ${MHDRS}
INCSLINKS+=	machine/$i ${INCLUDEDIR}/$i
.endfor
.for i in ${PHDRS}
INCSLINKS+=	posix4/$i ${INCLUDEDIR}/$i
.endfor

copies:
.for i in ${LDIRS} ${LSYMSUBDIRS} machine crypto
	if [ -L ${DESTDIR}/usr/include/$i ]; then \
		rm -f ${DESTDIR}/usr/include/$i; \
	fi
.endfor
	mtree -deU ${MTREE_FOLLOWS_SYMLINKS} -f ${.CURDIR}/../etc/mtree/BSD.include.dist \
		-p ${DESTDIR}/usr/include
.for i in ${LDIRS} ${LSUBDIRS}
	cd ${.CURDIR}/../sys; \
		${INSTALL} -C -o ${BINOWN} -g ${BINGRP} -m 444 $i/*.h \
		${DESTDIR}/usr/include/$i
.endfor
	cd ${.CURDIR}/../sys; \
		${INSTALL} -C -o ${BINOWN} -g ${BINGRP} -m 444 opencrypto/*.h \
		${DESTDIR}/usr/include/crypto
.if exists(${.CURDIR}/../sys/${MACHINE_ARCH}/include)
	cd ${.CURDIR}/../sys/${MACHINE_ARCH}/include; \
		${INSTALL} -C -o ${BINOWN} -g ${BINGRP} -m 444 *.h \
		${DESTDIR}/usr/include/machine
.if exists(${.CURDIR}/../sys/${MACHINE_ARCH}/include/pc)
	cd ${.CURDIR}/../sys/${MACHINE_ARCH}/include/pc; \
		${INSTALL} -C -o ${BINOWN} -g ${BINGRP} -m 444 *.h \
		${DESTDIR}/usr/include/machine/pc
.endif
.endif

symlinks:
	@${ECHO} "Setting up symlinks to kernel source tree..."
.for i in ${LDIRS}
	rm -rf ${DESTDIR}/usr/include/$i
	ln -s ../../sys/$i ${DESTDIR}/usr/include/$i
.endfor
	rm -rf ${DESTDIR}/usr/include/crypto
	ln -s ../../sys/opencrypto ${DESTDIR}/usr/include/crypto
.for i in ${LSYMSUBDIRS}
	rm -rf ${DESTDIR}/usr/include/$i
	ln -s ../../../sys/$i ${DESTDIR}/usr/include/$i
.endfor
	rm -rf ${DESTDIR}/usr/include/machine
	ln -s ../../sys/${MACHINE_ARCH}/include ${DESTDIR}/usr/include/machine

.include <bsd.prog.mk>

installincludes: ${SHARED}