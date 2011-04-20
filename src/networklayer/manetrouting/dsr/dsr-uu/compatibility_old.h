#ifndef _MANET_COMPATIBILITY_H
#define _MANET_COMPATIBILITY_H

#include <endian.h>
//#define MobilityFramework

//#define _SYS_WAIT_H
#ifdef  _WIN32
#include <winsock2.h>
typedef      char  int8_t;
typedef      short int16_t;
typedef      int   int32_t;
typedef unsigned char  u_int8_t;
typedef unsigned short u_int16_t;
typedef unsigned int   u_int32_t;
typedef u_int8_t               uint8_t;
typedef u_int16_t              uint16_t;
typedef u_int32_t              uint32_t;

#else
#include <sys/types.h>
#include <stdint.h>
#endif

#define ETH_ALEN    6       /* Octets in one ethernet addr   */

typedef uint32_t in_addr_t;
typedef unsigned short  sa_family_t;
#define MAXTTL      255

#ifndef  _WIN32
struct in_addr
{
    in_addr_t s_addr;
};

struct sockaddr
{
    sa_family_t sa_family;  /* address family, AF_xxx   */
    char        sa_data[14];    /* 14 bytes of protocol address */
};
#endif





struct ethhdr
{
    unsigned char   h_dest[ETH_ALEN];   /* destination eth addr */
    unsigned char   h_source[ETH_ALEN]; /* source ether addr    */
    uint16_t    h_proto;        /* packet type ID field */
#ifdef  _WIN32
};
#else
} __attribute__((packed));
#endif

struct iphdr
{
    unsigned int ihl:4;
    unsigned int version:4;
    u_int8_t tos;
    u_int16_t tot_len;
    u_int16_t id;
    u_int16_t frag_off;
    u_int8_t ttl;
    u_int8_t protocol;
    u_int16_t check;
    u_int32_t saddr;
    u_int32_t daddr;
    /*The options start here. */
};

#endif              /* _DSR_H */
