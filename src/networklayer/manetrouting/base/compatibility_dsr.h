#ifndef _MANET_COMPATIBILITY_DSR_H
#define _MANET_COMPATIBILITY_DSR_H

//#ifdef _WIN32
//#include <unistd.h>
//#include <sys/time.h>
//#include <sys/types.h>
//#endif
#include <omnetpp.h>

#ifndef u_int8_t
typedef uint8_t u_int8_t;
#endif

#ifndef u_int16_t
typedef uint16_t u_int16_t;
#endif

#ifndef u_int32_t
typedef uint32_t u_int32_t;
#endif


#define ETH_ALEN    6       /* Octets in one ethernet addr   */

#ifndef in_addr_t
typedef uint32_t in_addr_t;
#endif

typedef unsigned short  sa_family_t;
#define MAXTTL      255

//struct in_addr
//  {
//    in_addr_t s_addr;
//  };

#ifndef in_addr_t
typedef uint32_t in_addr_t;
#endif

#ifdef _WIN32
struct In_addr
{
    uint32_t S_addr;
#undef s_addr
#define s_addr S_addr
};

struct Sockaddr
{
    sa_family_t sa_family;  /* address family, AF_xxx   */
    char        sa_data[14];    /* 14 bytes of protocol address */
};

#undef in_addr
#undef sockaddr

#define in_addr In_addr
#define sockaddr Sockaddr
#else
struct in_addr
{
    uint32_t s_addr;
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

};

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
