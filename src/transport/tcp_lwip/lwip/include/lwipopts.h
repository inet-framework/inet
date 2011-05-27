/*
 * Copyright (C) 2010 Zoltan Bojthe
 *
 * LWIP settings for Omnet/Omnest simulator INET framework.
 *
 */

#define NO_SYS 1


#define LWIP_DEBUG

#define TCP_DEBUG LWIP_DBG_ON

/**
 * LWIP_NETCONN==1: Enable Netconn API (require to use api_lib.c)
 */
#define LWIP_NETCONN                    0

/**
 * LWIP_SOCKET==1: Enable Socket API (require to use sockets.c)
 */
#define LWIP_SOCKET                     0

/**
 * MEM_LIBC_MALLOC==1: Use malloc/free/realloc provided by your C-library
 * instead of the lwip internal allocator. Can save code size if you
 * already use it.
 */
#define MEM_LIBC_MALLOC                 1

/**
* MEMP_MEM_MALLOC==1: Use mem_malloc/mem_free instead of the lwip pool allocator.
* Especially useful with MEM_LIBC_MALLOC but handle with care regarding execution
* speed and usage from interrupts!
*/
#define MEMP_MEM_MALLOC                 1

/**
 * LWIP_ARP==1: Enable ARP functionality.
 */
#define LWIP_ARP                        0
#define ARP_QUEUEING                    0

/**
 * LWIP_ICMP==1: Enable ICMP module inside the IP stack.
 * Be careful, disable that make your product non-compliant to RFC1122
 */
#define LWIP_ICMP                       0

/**
 * LWIP_RAW==1: Enable application layer to hook into the IP layer itself.
 */
#define LWIP_RAW                        0

/**
 * LWIP_UDP==1: Turn on UDP.
 */
#define LWIP_UDP                        0

/**
 * LWIP_NETBUF_RECVINFO==1: append destination addr and port to every netbuf.
BZ: check it!!!!!!
 */
#define LWIP_NETBUF_RECVINFO            0

/**
 * LWIP_EVENT_API and LWIP_CALLBACK_API: Only one of these should be set to 1.
 *     LWIP_EVENT_API==1: The user defines lwip_tcp_event() to receive all
 *         events (accept, sent, etc) that happen in the system.
 *     LWIP_CALLBACK_API==1: The PCB callback function is called directly
 *         for the event.
BZ: check it!!!!!!
 */
#define LWIP_CALLBACK_API               0
#define LWIP_EVENT_API                  1

/**
 * CHECKSUM_GEN_IP==1: Generate checksums in software for outgoing IP packets.
 */
#define CHECKSUM_GEN_IP                 0

/**
 * CHECKSUM_GEN_UDP==1: Generate checksums in software for outgoing UDP packets.
 */
#define CHECKSUM_GEN_UDP                0

/**
 * CHECKSUM_GEN_TCP==1: Generate checksums in software for outgoing TCP packets.
 */
#define CHECKSUM_GEN_TCP                0

/**
 * CHECKSUM_CHECK_IP==1: Check checksums in software for incoming IP packets.
 */
#define CHECKSUM_CHECK_IP               0

/**
 * CHECKSUM_CHECK_UDP==1: Check checksums in software for incoming UDP packets.
 */
#define CHECKSUM_CHECK_UDP              0

/**
 * CHECKSUM_CHECK_TCP==1: Check checksums in software for incoming TCP packets.
 */
#define CHECKSUM_CHECK_TCP              0

#define TCP_STATS                       0
#define TCP_SND_BUF                     4096

