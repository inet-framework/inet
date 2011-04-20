
#ifndef _AODV_MSG_OMNET_H
#define _AODV_MSG_OMNET_H

#include <stdio.h>
#include <stdlib.h>
#include <omnetpp.h>
//#include <unistd.h>
//#include <sys/time.h>
//#include <sys/types.h>
#include "compatibility.h"


// this structures are a redifinition of def.h struct for omnet

typedef struct
{
    u_int8_t type;
    u_int8_t length;
    char * pointer;
    /* Type specific data follows here */
} AODV_ext;

/* A generic AODV packet header struct... */ /* Omnet++ definition */
struct AODV_msg : public cPacket
{
    u_int8_t type;
    //explicit AODV_msg(const char *name="AodvMgs") : cPacket(name) {extensionsize=0;extension=NULL;}
    explicit AODV_msg(const char *name=NULL) : cPacket(name) {extensionsize=0; extension=NULL;}
    ~AODV_msg ();
    uint8_t ttl;
    AODV_msg (const AODV_msg  &m);
    AODV_msg &  operator= (const AODV_msg &m);
    virtual AODV_msg *dup() const {return new AODV_msg(*this);}
    AODV_ext *  addExtension(int,int,char *);
    void clearExtension();
    AODV_ext * getExtension(int i) {return &extension[i];}
    AODV_ext * getFirstExtension () {return extension;}
    AODV_ext * getNexExtension(AODV_ext*);
    int getNumExtension() {return extensionsize;}
  private:
    AODV_ext *extension;
    int extensionsize;
    /* NS_PORT: Additions for the AODVUU packet type in ns-2 */
};
typedef AODV_msg hdr_aodvuu;    // Name convention for headers

#define HDR_AODVUU(p) ((hdr_aodvuu *) hdr_aodvuu::access(p))
#define AODV_EXT_HDR_SIZE 2
#define AODV_EXT_DATA(ext) ((char *)ext->pointer)
#define AODV_EXT_NEXT(ext) ((AODV_ext *) (ext+1))
#define AODV_EXT_SIZE(ext) (AODV_EXT_HDR_SIZE+ext->length)

/* Extra unreachable destinations... */
typedef struct
{
    // u_int32_t dest_addr;
    Uint128 dest_addr;
    u_int32_t dest_seqno;
} RERR_udest;
//#define RERR_UDEST_SIZE sizeof(RERR_udest)
#define RERR_UDEST_SIZE 8

#ifdef RERR
#undef RERR
#endif

struct RERR : public AODV_msg
{
    unsigned short res1;
    unsigned short n;
    unsigned short res2;
    u_int8_t dest_count;
    RERR_udest *   _udest;
    explicit RERR(const char *name="RERRAodvMsg") : AODV_msg (name) {setBitLength(12*8); dest_count=0; _udest=NULL;}
    ~RERR ();
    RERR (const RERR &m);
    void addUdest(const Uint128 &,unsigned int);
    void clearUdest();
    RERR_udest * getUdest(int);
    RERR &  operator= (const RERR &m);
    virtual RERR *dup() const {return new RERR(*this);}
};

#define RERR_UDEST_FIRST(rerr) (rerr->getUdest(0))
#define RERR_UDEST_NEXT(udest) ((RERR_udest *)((char *)udest + sizeof(RERR_udest)))
#define RERR_SIZE 12
#define RERR_CALC_SIZE(rerr) (rerr->getByteLength())

struct RREP : public AODV_msg
{
    u_int16_t res1;
    u_int16_t a;
    u_int16_t r;
    u_int16_t prefix;
    u_int16_t res2;
    u_int8_t hcnt;
//  u_int32_t dest_addr;
    Uint128 dest_addr;
    u_int32_t dest_seqno;
//  u_int32_t orig_addr;
    Uint128 orig_addr;
    u_int32_t lifetime;
    AODV_ext *extension;
    explicit RREP (const char *name="RREPAodvMsg") : AODV_msg (name) {setBitLength(20*8);}
    RREP (const RREP &m);
    RREP &  operator= (const RREP &m);
    virtual RREP *dup() const {return new RREP(*this);}
    virtual std::string detailedInfo() const;
} ;
#define RREP_SIZE 20

struct RREP_ack : public AODV_msg
{
    u_int8_t reserved;
    explicit RREP_ack (const char *name="RREPAckAodvMsg") : AODV_msg (name) {setBitLength(2*8);}
    RREP_ack (const RREP_ack  &m);
    RREP_ack &  operator= (const RREP_ack &m);
    virtual RREP_ack *dup() const {return new RREP_ack(*this);}
} ;
#define RREP_ACK_SIZE 2

struct RREQ : public AODV_msg
{
    u_int8_t j;     /* Join flag (multicast) */
    u_int8_t r;     /* Repair flag */
    u_int8_t g;     /* Gratuitous RREP flag */
    u_int8_t d;     /* Destination only respond */
    u_int8_t res1;
    u_int8_t res2;
    u_int8_t hcnt;
    u_int32_t rreq_id;
//  u_int32_t dest_addr;
    Uint128 dest_addr;
    u_int32_t dest_seqno;
//  u_int32_t orig_addr;
    Uint128 orig_addr;
    u_int32_t orig_seqno;
    explicit RREQ(const char *name="RREQAodvMsg") : AODV_msg (name) {setBitLength(24*8);}
    RREQ (const RREQ &m);
    RREQ &  operator= (const RREQ &m);
    virtual RREQ *dup() const {return new RREQ(*this);}
    virtual std::string detailedInfo() const;
};

#define RREQ_SIZE 24
#endif

