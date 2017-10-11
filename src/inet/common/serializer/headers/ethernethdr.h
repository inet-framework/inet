/*
 * Fundamental constants relating to ethernet.
 *
 * $FreeBSD: src/sys/net/ethernet.h,v 1.12.2.8 2002/12/01 14:03:09 sobomax Exp $
 * $DragonFly: src/sys/net/ethernet.h,v 1.19 2008/06/24 11:40:56 sephe Exp $
 *
 */

#ifndef __INET_ETHERNETHDR_H
#define __INET_ETHERNETHDR_H

namespace inet {

namespace serializer {

#ifdef _MSC_VER
#define __PACKED__
#else
#define __PACKED__  __attribute__((packed))
#endif


/*
 * The number of bytes in an ethernet (MAC) address.
 */
#define ETHER_ADDR_LEN    6

/*
 * The number of bytes in the type field.
 */
#define ETHER_TYPE_LEN    2

/*
 * The number of bytes in the trailing CRC field.
 */
#define ETHER_CRC_LEN     4

/*
 * Mbuf adjust factor to force 32-bit alignment of IP header.
 * Drivers should do m_adj(m, ETHER_ALIGN) when setting up a
 * receive so the upper layers get the IP header properly aligned
 * past the 14-byte Ethernet header.
 */
#define ETHER_ALIGN       2

/*
 * The length of the combined header.
 */
#define ETHER_HDR_LEN     (ETHER_ADDR_LEN * 2 + ETHER_TYPE_LEN)

/*
 * The minimum packet length.
 */
#define ETHER_MIN_LEN     64

/*
 * The maximum packet length.
 */
#define ETHER_MAX_LEN     1518

/*
 * A macro to validate a length with
 */
#define ETHER_IS_VALID_LEN(foo) \
    ((foo) >= ETHER_MIN_LEN && (foo) <= ETHER_MAX_LEN)

/*
 * Ethernet CRC32 polynomials (big- and little-endian verions).
 */
#define ETHER_CRC_POLY_LE    0xedb88320
#define ETHER_CRC_POLY_BE    0x04c11db6

extern const uint8_t etherbroadcastaddr[ETHER_ADDR_LEN];

#define ETHER_IS_MULTICAST(addr)    (*(addr) & 0x01) /* is address mcast/bcast? */

/*
 * The ETHERTYPE_NTRAILER packet types starting at ETHERTYPE_TRAIL have
 * (type-ETHERTYPE_TRAIL)*512 bytes of data followed
 * by an ETHER type (as given above) and then the (variable-length) header.
 */
#define ETHERTYPE_TRAIL          0x1000          /* Trailer packet */
#define ETHERTYPE_NTRAILER       16

#define ETHERMTU                 (ETHER_MAX_LEN - ETHER_HDR_LEN - ETHER_CRC_LEN)
#define ETHERMIN                 (ETHER_MIN_LEN - ETHER_HDR_LEN - ETHER_CRC_LEN)

} // namespace serializer

} // namespace inet

#endif /* !_NET_ETHERNET_H_ */

