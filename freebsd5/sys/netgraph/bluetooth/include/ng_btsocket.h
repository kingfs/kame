/*
 * ng_btsocket.h
 *
 * Copyright (c) 2001-2002 Maksim Yevmenkin <m_evmenkin@yahoo.com>
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
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $Id: ng_btsocket.h,v 1.7 2002/11/12 22:31:39 max Exp $
 * $FreeBSD: src/sys/netgraph/bluetooth/include/ng_btsocket.h,v 1.1 2002/11/20 23:01:57 julian Exp $
 */

#ifndef _NETGRAPH_BTSOCKET_H_
#define _NETGRAPH_BTSOCKET_H_ 1

/*
 * XXX FIXME: does not belong here, move to sys/socket.h later and fix AF_MAX
 */

#define AF_BLUETOOTH	36		/* Address family */
#define PF_BLUETOOTH	AF_BLUETOOTH	/* Protocol family */

/*
 * XXX FIXME: does not belong here, move to other places later
 */

#define BLUETOOTH_PROTO_HCI	134	/* HCI protocol number */
#define BLUETOOTH_PROTO_L2CAP	135	/* L2CAP protocol number */

/*
 * XXX FIXME: probably does not belong here
 * Bluetooth version of struct sockaddr for raw HCI sockets
 */

struct sockaddr_hci {
	u_char		hci_len;	/* total length */
	u_char		hci_family;	/* address family */
	char		hci_node[16];	/* address (size == NG_NODELEN  + 1) */
};

/* Raw HCI socket options */
#define SOL_HCI_RAW		0x0802	/* socket options level */

#define SO_HCI_RAW_FILTER	1	/* get/set filter on socket */
#define SO_HCI_RAW_DIRECTION	2	/* turn on/off direction info */
#define SCM_HCI_RAW_DIRECTION	SO_HCI_RAW_DIRECTION /* cmsg_type  */

/*
 * Raw HCI socket filter.
 *
 * For packet mask use (1 << (HCI packet indicator - 1))
 * For event mask use (1 << (Event - 1))
 */

struct ng_btsocket_hci_raw_filter {
	bitstr_t	bit_decl(packet_mask, 32);
	bitstr_t	bit_decl(event_mask, (NG_HCI_EVENT_MASK_SIZE * 8));
};

/*
 * Raw HCI sockets ioctl's
 */

/* Get state */
struct ng_btsocket_hci_raw_node_state {
	char			hci_node[16];
	ng_hci_node_state_ep	state;
};
#define SIOC_HCI_RAW_NODE_GET_STATE \
	_IOWR('b', NGM_HCI_NODE_GET_STATE, \
		struct ng_btsocket_hci_raw_node_state)

/* Initialize */
struct ng_btsocket_hci_raw_node_init {
	char	hci_node[16];
};
#define SIOC_HCI_RAW_NODE_INIT \
	_IOWR('b', NGM_HCI_NODE_INIT, \
		struct ng_btsocket_hci_raw_node_init)

/* Get/Set debug level */
struct ng_btsocket_hci_raw_node_debug {
	char			hci_node[16];
	ng_hci_node_debug_ep	debug;
};
#define SIOC_HCI_RAW_NODE_GET_DEBUG \
	_IOWR('b', NGM_HCI_NODE_GET_DEBUG, \
		struct ng_btsocket_hci_raw_node_debug)
#define SIOC_HCI_RAW_NODE_SET_DEBUG \
	_IOWR('b', NGM_HCI_NODE_SET_DEBUG, \
		struct ng_btsocket_hci_raw_node_debug)

/* Get buffer info */
struct ng_btsocket_hci_raw_node_buffer {
	char			hci_node[16];
	ng_hci_node_buffer_ep	buffer;
};
#define SIOC_HCI_RAW_NODE_GET_BUFFER \
	_IOWR('b', NGM_HCI_NODE_GET_BUFFER, \
		struct ng_btsocket_hci_raw_node_buffer)

/* Get BD_ADDR */
struct ng_btsocket_hci_raw_node_bdaddr {
	char		hci_node[16];
	bdaddr_t	bdaddr;
};
#define SIOC_HCI_RAW_NODE_GET_BDADDR \
	_IOWR('b', NGM_HCI_NODE_GET_BDADDR, \
		struct ng_btsocket_hci_raw_node_bdaddr)

/* Get features */
struct ng_btsocket_hci_raw_node_features {
	char		hci_node [16];
	u_int8_t	features[NG_HCI_FEATURES_SIZE];
};
#define SIOC_HCI_RAW_NODE_GET_FEATURES \
	_IOWR('b', NGM_HCI_NODE_GET_FEATURES, \
		struct ng_btsocket_hci_raw_node_features)

/* Get stat */
struct ng_btsocket_hci_raw_node_stat {
	char			hci_node[16];
	ng_hci_node_stat_ep	stat;
};
#define SIOC_HCI_RAW_NODE_GET_STAT \
	_IOWR('b', NGM_HCI_NODE_GET_STAT, \
		struct ng_btsocket_hci_raw_node_stat)

/* Reset stat */
struct ng_btsocket_hci_raw_node_reset_stat {
	char	hci_node[16];
};
#define SIOC_HCI_RAW_NODE_RESET_STAT \
	_IOWR('b', NGM_HCI_NODE_RESET_STAT, \
		struct ng_btsocket_hci_raw_node_reset_stat)

/* Flush neighbor cache */
struct ng_btsocket_hci_raw_node_flush_neighbor_cache {
	char	hci_node[16];
};
#define SIOC_HCI_RAW_NODE_FLUSH_NEIGHBOR_CACHE \
	_IOWR('b', NGM_HCI_NODE_FLUSH_NEIGHBOR_CACHE, \
		struct ng_btsocket_hci_raw_node_flush_neighbor_cache)

/* Get neighbor cache */
struct ng_btsocket_hci_raw_node_neighbor_cache {
	char					 hci_node[16];
	u_int32_t				 num_entries;
	ng_hci_node_neighbor_cache_entry_ep	*entries;
};
#define SIOC_HCI_RAW_NODE_GET_NEIGHBOR_CACHE \
	_IOWR('b', NGM_HCI_NODE_GET_NEIGHBOR_CACHE, \
		struct ng_btsocket_hci_raw_node_neighbor_cache)

/* Get connection list */
struct ng_btsocket_hci_raw_con_list {
	char			 hci_node[16];
	u_int32_t		 num_connections;
	ng_hci_node_con_ep	*connections;
};
#define SIOC_HCI_RAW_NODE_GET_CON_LIST \
	_IOWR('b', NGM_HCI_NODE_GET_CON_LIST, \
		struct ng_btsocket_hci_raw_con_list)

/* Get/Set link policy settings mask */
struct ng_btsocket_hci_raw_node_link_policy_mask {
	char				hci_node[16];
	ng_hci_node_link_policy_mask_ep	policy_mask;
};
#define SIOC_HCI_RAW_NODE_GET_LINK_POLICY_MASK \
	_IOWR('b', NGM_HCI_NODE_GET_LINK_POLICY_SETTINGS_MASK, \
		struct ng_btsocket_hci_raw_node_link_policy_mask)
#define SIOC_HCI_RAW_NODE_SET_LINK_POLICY_MASK \
	_IOWR('b', NGM_HCI_NODE_SET_LINK_POLICY_SETTINGS_MASK, \
		struct ng_btsocket_hci_raw_node_link_policy_mask)

/* Get/Set packet mask */
struct ng_btsocket_hci_raw_node_packet_mask {
	char				hci_node[16];
	ng_hci_node_packet_mask_ep	packet_mask;
};
#define SIOC_HCI_RAW_NODE_GET_PACKET_MASK \
	_IOWR('b', NGM_HCI_NODE_GET_PACKET_MASK, \
		struct ng_btsocket_hci_raw_node_packet_mask)
#define SIOC_HCI_RAW_NODE_SET_PACKET_MASK \
	_IOWR('b', NGM_HCI_NODE_SET_PACKET_MASK, \
		struct ng_btsocket_hci_raw_node_packet_mask)

/*
 * XXX FIXME: probably does not belong here
 * Bluetooth version of struct sockaddr for L2CAP sockets (RAW and SEQPACKET)
 */

struct sockaddr_l2cap {
	u_char		l2cap_len;	/* total length */
	u_char		l2cap_family;	/* address family */
	u_int16_t	l2cap_psm;	/* PSM (Protocol/Service Multiplexor) */
	bdaddr_t	l2cap_bdaddr;	/* address */
};

/* L2CAP socket options */
#define SOL_L2CAP		0x1609	/* socket option level */

#define SO_L2CAP_IMTU		1	/* get/set incoming MTU */
#define SO_L2CAP_OMTU		2	/* get outgoing (peer incoming) MTU */
#define SO_L2CAP_IFLOW		3	/* get incoming flow spec. */
#define SO_L2CAP_OFLOW		4	/* get/set outgoing flow spec. */
#define SO_L2CAP_FLUSH		5	/* get/set flush timeout */

/*
 * Raw L2CAP sockets ioctl's
 */

/* Ping */
struct ng_btsocket_l2cap_raw_ping {
	bdaddr_t		 bdaddr[2];
#define echo_src		 bdaddr[0]
#define echo_dst		 bdaddr[1]
	u_int32_t		 result;
	u_int32_t		 echo_size;
	u_int8_t		*echo_data;
};
#define SIOC_L2CAP_L2CA_PING \
	_IOWR('b', NGM_L2CAP_L2CA_PING, \
		struct ng_btsocket_l2cap_raw_ping)

/* Get info */
struct ng_btsocket_l2cap_raw_get_info {
	bdaddr_t		 bdaddr[2];
#define info_src		 bdaddr[0]
#define info_dst		 bdaddr[1]
	u_int32_t		 result;
	u_int32_t		 info_type;
	u_int32_t		 info_size;
	u_int8_t		*info_data;
};
#define SIOC_L2CAP_L2CA_GET_INFO \
	_IOWR('b', NGM_L2CAP_L2CA_GET_INFO, \
		struct ng_btsocket_l2cap_raw_get_info)

/* Get flags */
struct ng_btsocket_l2cap_raw_node_flags {
	bdaddr_t		src;
	ng_l2cap_node_flags_ep	flags;
};
#define SIOC_L2CAP_NODE_GET_FLAGS \
	_IOWR('b', NGM_L2CAP_NODE_GET_FLAGS, \
		struct ng_btsocket_l2cap_raw_node_flags)

/* Get/Set debug level */
struct ng_btsocket_l2cap_raw_node_debug {
	bdaddr_t		src;
	ng_l2cap_node_debug_ep	debug;
};
#define SIOC_L2CAP_NODE_GET_DEBUG \
	_IOWR('b', NGM_L2CAP_NODE_GET_DEBUG, \
		struct ng_btsocket_l2cap_raw_node_debug)
#define SIOC_L2CAP_NODE_SET_DEBUG \
	_IOWR('b', NGM_L2CAP_NODE_SET_DEBUG, \
		struct ng_btsocket_l2cap_raw_node_debug)

/* Get connection list */
struct ng_btsocket_l2cap_raw_con_list {
	bdaddr_t		 src;
	u_int32_t		 num_connections;
	ng_l2cap_node_con_ep	*connections;
};
#define SIOC_L2CAP_NODE_GET_CON_LIST \
	_IOWR('b', NGM_L2CAP_NODE_GET_CON_LIST, \
		struct ng_btsocket_l2cap_raw_con_list)

/* Get channel list */
struct ng_btsocket_l2cap_raw_chan_list {
	bdaddr_t		 src;
	u_int32_t		 num_channels;
	ng_l2cap_node_chan_ep	*channels;
};
#define SIOC_L2CAP_NODE_GET_CHAN_LIST \
	_IOWR('b', NGM_L2CAP_NODE_GET_CHAN_LIST, \
		struct ng_btsocket_l2cap_raw_chan_list)

/* 
 * Netgraph node type name and cookie 
 */

#define	NG_BTSOCKET_HCI_RAW_NODE_TYPE	"btsock_hci_raw"
#define	NG_BTSOCKET_L2CAP_RAW_NODE_TYPE	"btsock_l2c_raw"
#define	NG_BTSOCKET_L2CAP_NODE_TYPE	"btsock_l2c"

/*
 * Debug levels 
 */

#define NG_BTSOCKET_ALERT_LEVEL	1
#define NG_BTSOCKET_ERR_LEVEL	2
#define NG_BTSOCKET_WARN_LEVEL	3
#define NG_BTSOCKET_INFO_LEVEL	4

#endif /* _NETGRAPH_BTSOCKET_H_ */

