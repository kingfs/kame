/*	$NetBSD: irix_syssgi.h,v 1.5 2002/10/13 22:13:48 manu Exp $ */

/*-
 * Copyright (c) 2001-2002 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Emmanuel Dreyfus.
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
 *	This product includes software developed by the NetBSD
 *	Foundation, Inc. and its contributors.
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

#ifndef _IRIX_SYSSGI_H_
#define _IRIX_SYSSGI_H_

/* From IRIX's <sys/systeminfo.h> */
#define IRIX_MAX_SERIAL_SIZE 16
struct irix_module_info_s {
	__uint64_t serial_num;
	int mod_num;
	char serial_str[IRIX_MAX_SERIAL_SIZE];
};

#define IRIX_MAPELF_RELOCATE		0x4000000

/* From IRIX's <sys/syssgi.h> */

#define	IRIX_SGI_SYSID			1
#define	IRIX_SGI_BUFINFO		2
#define	IRIX_SGI_TUNE_SET		3
#define	IRIX_SGI_TUNE			IRIX_SGI_TUNE_SET
#define	IRIX_SGI_IDBG			4
#define	IRIX_SGI_INVENT			5
#define	IRIX_SGI_RDNAME			6
#define	IRIX_SGI_SETLED			7
#define	IRIX_SGI_SETNVRAM		8
#define	IRIX_SGI_GETNVRAM		9
#define	IRIX_SGI_SETKOPT		10
#define	IRIX_SGI_QUERY_FTIMER		12
#define	IRIX_SGI_QUERY_CYCLECNTR	13
#define	IRIX_SGI_SETSID			20
#define	IRIX_SGI_SETPGID		21
#define	IRIX_SGI_SYSCONF		22
#define	IRIX_SGI_PATHCONF		24
#define	IRIX_SGI_TITIMER		29
#define	IRIX_SGI_READB			30
#define	IRIX_SGI_WRITEB			31
#define	IRIX_SGI_SETGROUPS		40
#define	IRIX_SGI_GETGROUPS		41
#define	IRIX_SGI_SETTIMEOFDAY		52
#define	IRIX_SGI_SETTIMETRIM		53
#define	IRIX_SGI_GETTIMETRIM		54
#define	IRIX_SGI_SPROFIL		55
#define	IRIX_SGI_RUSAGE			56
#define	IRIX_SGI_SIGSTACK		57
#define	IRIX_SGI_NETPROC		59
#define	IRIX_SGI_SIGALTSTACK		60
#define	IRIX_SGI_BDFLUSHCNT		61
#define	IRIX_SGI_SSYNC			62
#define	IRIX_SGI_NFSCNVT		63
#define	IRIX_SGI_GETPGID		64
#define	IRIX_SGI_GETSID			65
#define	IRIX_SGI_IOPROBE		66
#define	IRIX_SGI_CONFIG			67
#define	IRIX_SGI_ELFMAP			68
#define	IRIX_SGI_MCONFIG		69
#define	IRIX_SGI_GETPLABEL		70
#define	IRIX_SGI_SETPLABEL		71
#define	IRIX_SGI_GETLABEL		72
#define	IRIX_SGI_SETLABEL		73
#define	IRIX_SGI_SATREAD		74
#define	IRIX_SGI_SATWRITE		75
#define	IRIX_SGI_SATCTL			76
#define	IRIX_SGI_LOADATTR		77
#define	IRIX_SGI_UNLOADATTR		78
#define	IRIX_SGI_RECVLUMSG		79
#define	IRIX_SGI_PLANGMOUNT		80
#define	IRIX_SGI_GETPSOACL		81
#define	IRIX_SGI_SETPSOACL		82
#define	IRIX_SGI_CAP_GET		83
#define	IRIX_SGI_CAP_SET		84
#define	IRIX_SGI_PROC_ATTR_GET		85
#define	IRIX_SGI_EAG_GETPROCATTR	IRIX_SGI_PROC_ATTR_GET
#define	IRIX_SGI_PROC_ATTR_SET		86
#define	IRIX_SGI_EAG_SETPROCATTR	IRIX_SGI_PROC_ATTR_SET
#define	IRIX_SGI_REVOKE			87
#define	IRIX_SGI_FREVOKE		IRIX_SGI_REVOKE
#define	IRIX_SGI_ACL_GET		88
#define	IRIX_SGI_ACL_SET		89
#define	IRIX_SGI_MAC_GET		90
#define	IRIX_SGI_MAC_SET		91
#define	IRIX_SGI_RXEV_GET		92
#define	IRIX_SGI_SBE_GET_INFO		98
#define	IRIX_SGI_SBE_CLR_INFO		99
#define	IRIX_SGI_GET_EVCONF		102
#define	IRIX_SGI_MPCWAROFF		103
#define	IRIX_SGI_SET_AUTOPWRON		104
#define	IRIX_SGI_SPIPE			105
#define	IRIX_SGI_SYMTAB			106
#define	IRIX_SGI_SET_FP_PRECISE		107
#define	IRIX_SGI_TOSSTSAVE		108
#define	IRIX_SGI_FDHI			109
#define	IRIX_SGI_SET_CONFIG_SMM		110
#define	IRIX_SGI_SET_FP_PRESERVE	111
#define	IRIX_SGI_MINRSS			112
#define	IRIX_SGI_GRIO			113
#define	IRIX_SGI_XLV_SET_TAB		114
#define	IRIX_SGI_XLV_GET_TAB		115
#define	IRIX_SGI_GET_FP_PRECISE		116
#define	IRIX_SGI_GET_CONFIG_SMM		117
#define	IRIX_SGI_FP_IMPRECISE_SUPP	118
#define	IRIX_SGI_CONFIG_NSMM_SUPP	119
#define	IRIX_SGI_RT_TSTAMP_CREATE	122
#define	IRIX_SGI_RT_TSTAMP_DELETE	123
#define	IRIX_SGI_RT_TSTAMP_START	124
#define	IRIX_SGI_RT_TSTAMP_STOP		125
#define	IRIX_SGI_RT_TSTAMP_ADDR		126
#define	IRIX_SGI_RT_TSTAMP_MASK		127
#define	IRIX_SGI_RT_TSTAMP_EOB_MODE	128
#define	IRIX_SGI_USE_FP_BCOPY		129
#define	IRIX_SGI_GET_UST		130
#define	IRIX_SGI_SPECULATIVE_EXEC	131
#define	IRIX_SGI_XLV_NEXT_RQST		132
#define	IRIX_SGI_XLV_ATTR_CURSOR	133
#define	IRIX_SGI_XLV_ATTR_GET		134
#define	IRIX_SGI_XLV_ATTR_SET		135
#define	IRIX_SGI_BTOOLSIZE		136
#define	IRIX_SGI_BTOOLGET		137
#define	IRIX_SGI_BTOOLREINIT		138
#define	IRIX_SGI_CREATE_UUID		139
#define	IRIX_SGI_NOFPE			140
#define	IRIX_SGI_OLD_SOFTFP		141
#define	IRIX_SGI_FS_INUMBERS		142
#define	IRIX_SGI_FS_BULKSTAT		143
#define	IRIX_SGI_RT_TSTAMP_WAIT		144
#define	IRIX_SGI_RT_TSTAMP_UPDATE	145
#define	IRIX_SGI_PATH_TO_HANDLE		146
#define	IRIX_SGI_PATH_TO_FSHANDLE	147
#define	IRIX_SGI_FD_TO_HANDLE		148
#define	IRIX_SGI_OPEN_BY_HANDLE		149
#define	IRIX_SGI_READLINK_BY_HANDLE	150
#define	IRIX_SGI_READ_DANGID		151
#define	IRIX_SGI_CONST			152
#define	IRIX_SGI_XFS_FSOPERATIONS	153
#define	IRIX_SGI_SETASH			154
#define	IRIX_SGI_GETASH			155
#define	IRIX_SGI_SETPRID		156
#define	IRIX_SGI_GETPRID		157
#define	IRIX_SGI_SETSPINFO		158
#define	IRIX_SGI_GETSPINFO		159
#define	IRIX_SGI_SHAREII		160
#define	IRIX_SGI_NEWARRAYSESS		161
#define	IRIX_SGI_GETDFLTPRID		162
#define	IRIX_SGI_SET_DISMISSED_EXC_CNT	163
#define	IRIX_SGI_GET_DISMISSED_EXC_CNT	164
#define	IRIX_SGI_CYCLECNTR_SIZE		165
#define	IRIX_SGI_QUERY_FASTTIMER	166
#define	IRIX_SGI_PIDSINASH		167
#define	IRIX_SGI_ULI			168
#define	IRIX_SGI_AUTOFS_SYS		170
#define	IRIX_SGI_CACHEFS_SYS		171
#define	IRIX_SGI_NFSNOTIFY		172
#define	IRIX_SGI_LOCKDSYS		173
#define	IRIX_SGI_EVENTCTR		174
#define	IRIX_SGI_GETPRUSAGE		175
#define	IRIX_SGI_PROCMASK_LOCATION	176
#define	IRIX_SGI_CKPT_SYS		178
#define	IRIX_SGI_GETGRPPID		179
#define	IRIX_SGI_GETSESPID		180
#define	IRIX_SGI_ENUMASHS		181
#define	IRIX_SGI_SETASMACHID		182
#define	IRIX_SGI_GETASMACHID		183
#define	IRIX_SGI_GETARSESS		184
#define	IRIX_SGI_JOINARRAYSESS		185
#define	IRIX_SGI_DBA_CONFIG		187
#define	IRIX_SGI_RELEASE_NAME		188
#define	IRIX_SGI_SYNCH_CACHE_HANDLER	189
#define	IRIX_SGI_SWASH_INIT		190
#define	IRIX_SGI_NUM_MODULES		191
#define	IRIX_SGI_MODULE_INFO		192
#define	IRIX_SGI_GET_CONTEXT_NAME	193
#define	IRIX_SGI_GET_CONTEXT_INFO	194
#define	IRIX_SGI_PART_OPERATIONS	195
#define	IRIX_SGI_EARLY_ADD_SWAP		197
#define	IRIX_SGI_BRICK_INFO		198
#define	IRIX_SGI_NUMA_MIGR_PAGE		200
#define	IRIX_SGI_NUMA_MIGR_PAGE_ALT	201
#define	IRIX_SGI_KAIO_USERINIT		202
#define	IRIX_SGI_KAIO_READ		203
#define	IRIX_SGI_KAIO_WRITE		204
#define	IRIX_SGI_KAIO_SUSPEND		205
#define	IRIX_SGI_DBA_GETSTATS		206
#define	IRIX_SGI_IO_SHOW_AUX_INFO	207
#define	IRIX_SGI_PMOCTL			208
#define	IRIX_SGI_ALLOCSHARENA		209
#define	IRIX_SGI_SETVPID		210
#define	IRIX_SGI_GETVPID		211
#define	IRIX_SGI_NUMA_TUNE		212
#define	IRIX_SGI_ERROR_FORCE		214
#define	IRIX_SGI_NUMA_STATS_GET		218
#define	IRIX_SGI_DPIPE_FSPE_BIND	219
#define	IRIX_SGI_DYIELD			220
#define	IRIX_SGI_TUNE_GET		221
#define	IRIX_SGI_CHPROJ			222
#define	IRIX_SGI_LCHPROJ		223
#define	IRIX_SGI_FCHPROJ		224
#define	IRIX_SGI_ARSESS_CTL		225
#define	IRIX_SGI_ARSESS_OP		226
#define	IRIX_SGI_FETCHOP_SETUP		227
#define	IRIX_SGI_FS_BULKSTAT_SINGLE	228
#define	IRIX_SGI_FS_SWAPEXT		229
#define	IRIX_SGI_WRITE_IP32_FLASH	230
#define	IRIX_SGI_ROUTERSTATS_ENABLED	231
#define	IRIX_SGI_DBA_CLRSTATS		232
#define	IRIX_SGI_IPC_AUTORMID_SHM	233
#define	IRIX_SGI_FORMAT_MODULE_NUM	234
#define	IRIX_SGI_PARSE_MODULE_NUM	235
#define	IRIX_SGI_IS_DEBUG_KERNEL	300
#define	IRIX_SGI_IS_TRAPLOG_DEBUG_KERNEL	301
#define	IRIX_SGI_POKE			320
#define	IRIX_SGI_PEEK			321
#define	IRIX_SGI_XLV_INDUCE_IO_ERROR	350
#define	IRIX_SGI_XLV_UNINDUCE_IO_ERROR	351
#define	IRIX_SGI_DKSC_INDUCE_IO_ERROR	352
#define	IRIX_SGI_DKSC_UNINDUCE_IO_ERROR	353
#define	IRIX_SGI_XFS_INJECT_ERROR	360
#define	IRIX_SGI_XFS_CLEAR_ERROR	361
#define	IRIX_SGI_XFS_CLEARALL_ERROR	362
#define	IRIX_SGI_XFS_MAKE_SHARED_RO	363
#define	IRIX_SGI_XFS_CLEAR_SHARED_RO	364
#define	IRIX_SGI_FO_DUMP		400
#define	IRIX_SGI_FO_SWITCH		401
#define	IRIX_SGI_NOHANG			402
#define	IRIX_SGI_UNFS			403
#define	IRIX_SGI_ATTR_LIST_BY_HANDLE	404
#define	IRIX_SGI_ATTR_MULTI_BY_HANDLE	405
#define	IRIX_SGI_FSSETDM_BY_HANDLE	406
#define	IRIX_SGI_FO_TRESSPASS		407
#define	IRIX_SGI_SCSI_CTLR_START_NUM	408
#define	IRIX_SGI_ACCTCTL		600
#define	IRIX_SGI_PHYSP			1011
#define	IRIX_SGI_KTHREAD		1012
#define	IRIX_SGI_FLUSH_ICACHE		1015
#define	IRIX_SGI_DEBUGLPAGE		1030
#define	IRIX_SGI_MAPLPAGE		1031
#define	IRIX_SGI_MUTEX_TEST		1040
#define	IRIX_SGI_MUTEX_TEST_INIT	1041
#define	IRIX_SGI_MUTEX_TESTER_INIT	1042
#define	IRIX_SGI_CREATE_MISER_POOL	1043
#define	IRIX_SGI_CREATE_MISER_JOB	1044
#define	IRIX_SGI_MISER_CRITICAL		1045
#define	IRIX_SGI_CONTEXT_SWITCH		1046
#define	IRIX_SGI_MRLOCK_TEST_INIT	1047
#define	IRIX_SGI_MRLOCK_TEST_RLOCK	1048
#define	IRIX_SGI_KMEM_TEST		1051
#define	IRIX_SGI_SHAKE_ZONES		1052
#define	IRIX_SGI_UNICENTER		1053
#define	IRIX_SGI_UNSUPPORTED_MAP_RESERVED_RANGE	1054
#define	IRIX_SGI_CELL			1060
#define	IRIX_SGI_NFS_UNMNT		1061
#define	IRIX_SGI_NUMA_MIGR_INT_VADDR	1100
#define	IRIX_SGI_NUMA_MIGR_INT_PFN	1101
#define	IRIX_SGI_NUMA_PAGEMIGR_TEST	1102
#define	IRIX_SGI_NUMA_TESTS		1103
#define	IRIX_SGI_NUMA_RESERVED		1104
#define	IRIX_SGI_MEMPROF_START		1105
#define	IRIX_SGI_MEMPROF_GET		1106
#define	IRIX_SGI_MEMPROF_CLEARALL	1107
#define	IRIX_SGI_MEMPROF_STOP		1108
#define	IRIX_SGI_HW_CPU_CONFREG		1200
#define	IRIX_SGI_UPANIC_SET		1201
#define	IRIX_SGI_UPANIC			1202
#define	IRIX_SGI_GETJLIMIT		1210
#define	IRIX_SGI_SETJLIMIT		1211
#define	IRIX_SGI_GETJUSAGE		1212
#define	IRIX_SGI_JL_UNUSED1		1213
#define	IRIX_SGI_GETJID			1214
#define	IRIX_SGI_KILLJOB		1215
#define	IRIX_SGI_MAKENEWJOB		1216
#define	IRIX_SGI_GETJOBPID		1217
#define	IRIX_SGI_JOINJOB		1218
#define	IRIX_SGI_SETWAITJOBPID		1219
#define	IRIX_SGI_WAITJOB		1220
#define	IRIX_SGI_IP30MISC		1250

/* From IRIX's <sys/unistd.h> */
#define IRIX_SC_ARG_MAX             1
#define IRIX_SC_CHILD_MAX           2
#define IRIX_SC_CLK_TCK             3
#define IRIX_SC_NGROUPS_MAX         4
#define IRIX_SC_OPEN_MAX            5
#define IRIX_SC_JOB_CONTROL         6
#define IRIX_SC_SAVED_IDS           7
#define IRIX_SC_VERSION             8
#define IRIX_SC_PASS_MAX            9
#define IRIX_SC_LOGNAME_MAX         10
#define IRIX_SC_PAGESIZE            11
#define IRIX_SC_PAGE_SIZE           IRIX_SC_PAGESIZE
#define IRIX_SC_XOPEN_VERSION       12
#define IRIX_SC_NACLS_MAX   13
#define IRIX_SC_NPROC_CONF  14
#define IRIX_SC_NPROC_ONLN  15
#define IRIX_SC_STREAM_MAX  16
#define IRIX_SC_TZNAME_MAX  17
#define IRIX_SC_RTSIG_MAX           20
#define IRIX_SC_SIGQUEUE_MAX        21
#define IRIX_SC_REALTIME_SIGNALS    23
#define IRIX_SC_PRIORITIZED_IO      24
#define IRIX_SC_ACL                 25
#define IRIX_SC_AUDIT               26
#define IRIX_SC_INF                 27
#define IRIX_SC_MAC                 28
#define IRIX_SC_CAP                 29
#define IRIX_SC_IP_SECOPTS          30
#define IRIX_SC_KERN_POINTERS       31
#define IRIX_SC_DELAYTIMER_MAX      32
#define IRIX_SC_MQ_OPEN_MAX         33
#define IRIX_SC_MQ_PRIO_MAX         34
#define IRIX_SC_SEM_NSEMS_MAX       35
#define IRIX_SC_SEM_VALUE_MAX       36
#define IRIX_SC_TIMER_MAX           37
#define IRIX_SC_FSYNC               38
#define IRIX_SC_MAPPED_FILES        39
#define IRIX_SC_MEMLOCK             40
#define IRIX_SC_MEMLOCK_RANGE       41
#define IRIX_SC_MEMORY_PROTECTION   42
#define IRIX_SC_MESSAGE_PASSING     43
#define IRIX_SC_PRIORITYIRIX_SCHEDULING 44
#define IRIX_SC_SEMAPHORES          45
#define IRIX_SC_SHARED_MEMORY_OBJECTS 46
#define IRIX_SC_SYNCHRONIZED_IO     47
#define IRIX_SC_TIMERS              48
#define IRIX_SC_ASYNCHRONOUS_IO     64
#define IRIX_SC_ABI_ASYNCHRONOUS_IO 65
#define IRIX_SC_AIO_LISTIO_MAX      66
#define IRIX_SC_AIO_MAX             67
#define IRIX_SC_AIO_PRIO_DELTA_MAX  68
#define IRIX_SC_XOPEN_SHM           75
#define IRIX_SC_XOPEN_CRYPT         76
#define IRIX_SC_BC_BASE_MAX         77
#define IRIX_SC_BC_DIM_MAX          78
#define IRIX_SC_BCIRIX_SCALE_MAX        79
#define IRIX_SC_BC_STRING_MAX       80
#define IRIX_SC_COLL_WEIGHTS_MAX    81
#define IRIX_SC_EXPR_NEST_MAX       82
#define IRIX_SC_LINE_MAX            83
#define IRIX_SC_RE_DUP_MAX          84
#define IRIX_SC_2_C_BIND            85
#define IRIX_SC_2_C_DEV             86
#define IRIX_SC_2_C_VERSION         87
#define IRIX_SC_2_FORT_DEV          88
#define IRIX_SC_2_FORT_RUN          89
#define IRIX_SC_2_LOCALEDEF         90
#define IRIX_SC_2_SW_DEV            91
#define IRIX_SC_2_UPE               92
#define IRIX_SC_2_VERSION           93
#define IRIX_SC_2_CHAR_TERM         94
#define IRIX_SC_XOPEN_ENH_I18N      95
#define IRIX_SC_IOV_MAX             96
#define IRIX_SC_ATEXIT_MAX          97
#define IRIX_SC_XOPEN_UNIX          98
#define IRIX_SC_XOPEN_XCU_VERSION   99
#define IRIX_SC_GETGR_R_SIZE_MAX    100
#define IRIX_SC_GETPW_R_SIZE_MAX    101
#define IRIX_SC_LOGIN_NAME_MAX      102
#define IRIX_SC_THREAD_DESTRUCTOR_ITERATIONS        103
#define IRIX_SC_THREAD_KEYS_MAX     104
#define IRIX_SC_THREAD_STACK_MIN    105
#define IRIX_SC_THREAD_THREADS_MAX  106
#define IRIX_SC_TTY_NAME_MAX        107
#define IRIX_SC_THREADS             108
#define IRIX_SC_THREAD_ATTR_STACKADDR       109
#define IRIX_SC_THREAD_ATTR_STACKSIZE       110
#define IRIX_SC_THREAD_PRIORITYIRIX_SCHEDULING  111
#define IRIX_SC_THREAD_PRIO_INHERIT 112
#define IRIX_SC_THREAD_PRIO_PROTECT 113
#define IRIX_SC_THREAD_PROCESS_SHARED       114
#define IRIX_SC_THREAD_SAFE_FUNCTIONS       115
#define IRIX_SC_KERN_SIM                    116
#define IRIX_SC_MMAP_FIXED_ALIGNMENT        117
#define IRIX_SC_SOFTPOWER                   118
#define IRIX_SC_XBS5_ILP32_OFF32            119
#define IRIX_SC_XBS5_ILP32_OFFBIG           120
#define IRIX_SC_XBS5_LP64_OFF64             121
#define IRIX_SC_XBS5_LPBIG_OFFBIG           122
#define IRIX_SC_XOPEN_LEGACY                123
#define IRIX_SC_XOPEN_REALTIME              124

/* From IRIX's <sys/unistd.h> */
#define IRIX_PC_LINK_MAX		1
#define IRIX_PC_MAX_CANON		2
#define IRIX_PC_MAX_INPUT		3
#define IRIX_PC_NAME_MAX		4
#define IRIX_PC_PATH_MAX		5
#define IRIX_PC_PIPE_BUF		6
#define IRIX_PC_CHOWN_RESTRICTED	7
#define IRIX_PC_NO_TRUNC		8
#define IRIX_PC_VDISABLE		9
#define IRIX_PC_SYNC_IO			10
#define IRIX_PC_PRIO_IO			11
#define IRIX_PC_ASYNC_IO		64
#define IRIX_PC_ABI_ASYNC_IO		65
#define IRIX_PC_ABI_AIO_XFER_MAX	66
#define IRIX_PC_FILESIZEBITS		67

#endif /* _IRIX_TYPES_H_ */
