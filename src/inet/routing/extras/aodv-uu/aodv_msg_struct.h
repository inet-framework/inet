
#ifndef _AODV_MSG_OMNET_H
#define _AODV_MSG_OMNET_H

#include <stdio.h>
#include <stdlib.h>
//#include <unistd.h>
//#include <sys/time.h>
//#include <sys/types.h>
#include "inet/routing/extras/base/compatibility.h"
#include "inet/routing/extras/base/ManetRoutingBase.h"
#include "inet/common/packet/chunk/FieldsChunk.h"

#define AddressSizeUsed 4
namespace inet {

namespace inetmanet {

// this structures are a redifinition of def.h struct for omnet

typedef struct
{
    uint8_t type;
    uint8_t length;
    char * pointer;
    uint8_t getType() {return type;}
    uint8_t getLength() {return length;}
    /* Type specific data follows here */
} AODV_ext;

/* A generic AODV packet header struct... */ /* Omnet++ definition */
class AODV_msg : public FieldsChunk
{
  public:
    u_int8_t type;
    uint8_t ttl;
    bool prevFix;
  private:
    AODV_ext *extension;
    int extensionsize;
    /* NS_PORT: Additions for the AODVUU packet type in ns-2 */

  public:
    //explicit AODV_msg(const char *name="AodvMgs") : cPacket(name) {extensionsize=0;extension=nullptr;}
    explicit AODV_msg() : FieldsChunk() {extensionsize=0; extension=nullptr;}
    ~AODV_msg ();
    AODV_msg (const AODV_msg  &m);
    AODV_msg &  operator= (const AODV_msg &m);
    virtual AODV_msg *dup() const override {return new AODV_msg(*this);}
    uint8_t getType() const {return type;}
    uint8_t getTtl() const {return ttl;}
    bool getPrevFix() const {return prevFix;}
    AODV_ext *  addExtension(int,int,char *);
    void clearExtension();
    AODV_ext * getExtension(int i) {return &extension[i];}
    AODV_ext& getExtensionRef(int i) {return extension[i];}
    AODV_ext * getFirstExtension () {return extension;}
    AODV_ext * getNexExtension(AODV_ext*);
    int getNumExtension() {return extensionsize;}
  private:
    void copy(const AODV_msg& other);
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
    L3Address dest_addr;
    u_int32_t dest_seqno;
} RERR_udest;
//#define RERR_UDEST_SIZE sizeof(RERR_udest)
// 8 for IPv4
//#define RERR_UDEST_SIZE 4+AddressSizeUsed

#ifdef RERR
#undef RERR
#endif

class RERR : public AODV_msg
{
public:
    uint16_t res1;
    uint16_t n;
    uint16_t res2;
    uint8_t dest_count;
    RERR_udest *   _udest;
    explicit RERR() : AODV_msg ()
    {
        res1 = 0;
        n = 0;
        res2 = 0;
        dest_count = 0;
        _udest = nullptr;
        ManetRoutingBase * owner = check_and_cast<ManetRoutingBase*>(this->getOwner());
        setChunkLength(b((8+(owner->getAddressSize()*2))*8));
    }
    ~RERR ();
    RERR (const RERR &m);
    unsigned short getRes1() const {return res1;}
    unsigned short getN() const {return n;}
    unsigned short getRes2() const {return res2;}
    int getUdestArraySize() {return dest_count;}
    void addUdest(const L3Address &,unsigned int);
    void clearUdest();
    RERR_udest *getUdest(int);
    RERR_udest& getUdestRef(int i) {return *getUdest(i);}
    RERR &  operator= (const RERR &m);
    virtual RERR *dup() const override {return new RERR(*this);}
  private:
    void copy(const RERR& other);
};

#define RERR_UDEST_FIRST(rerr) (rerr->getUdest(0))
#define RERR_UDEST_NEXT(udest) ((RERR_udest *)((char *)udest + sizeof(RERR_udest)))
// 12 for IPv4
//#define RERR_SIZE 8+AddressSizeUsed
#define RERR_CALC_SIZE(rerr) (rerr->getChunkLength().get())

class RREP : public AODV_msg
{

public:
    uint16_t res1;
    uint16_t a;
    uint16_t r;
    uint16_t prefix;
    uint16_t res2;
    uint8_t hcnt;
//  uint32_t dest_addr;
    L3Address dest_addr;
    uint32_t dest_seqno;
//  uint32_t orig_addr;
    L3Address orig_addr;
    uint32_t lifetime;
    uint32_t cost;
    uint8_t  hopfix;
    uint8_t  totalHops;
    explicit RREP () : AODV_msg ()
    {
        ManetRoutingBase * owner = check_and_cast<ManetRoutingBase*>(this->getOwner());
        setChunkLength(b((12+(owner->getAddressSize()*2))*8));
        res1 = 0;
        a = 0;
        r = 0;
        prefix = 0;
        res2 = 0;
        hcnt = 0;
        dest_addr = L3Address();
        dest_seqno = 0;
        orig_addr = L3Address();
        lifetime = 0;
        cost = 0;
        hopfix = 0;
        totalHops = 0;
    }
    RREP (const RREP &m);
    RREP &  operator= (const RREP &m);
    virtual RREP *dup() const override {return new RREP(*this);}
    virtual std::string str() const override;
    uint16_t getRes1() const {return res1;}
    uint16_t getA() const {return a;}
    uint16_t getR() const {return r;}
    uint16_t getPrefix() const {return prefix;}
    uint16_t getRes2() const {return res2;}
    uint8_t getHcnt() const {return hcnt;}
    //uint32_t getDest_addr() const {return dest_addr;}
    const L3Address& getDest_addr() const {return dest_addr;}
    uint32_t getDest_seqno() const {return dest_seqno;}
    //u_int32_t getOrig_addr() const {return orig_addr;}
    const L3Address& getOrig_addr() const {return orig_addr;}
    uint32_t getLifetime() const {return lifetime;}
    uint32_t getCost() const {return cost;}
    uint8_t  getHopfix() const {return hopfix;}
  private:
    void copy(const RREP& other);
} ;
// 20 for IPv4
//#define RREP_SIZE (AddressSizeUsed*2)+12

class RREP_ack : public AODV_msg
{

  public:
    uint8_t reserved;
    explicit RREP_ack () : AODV_msg () {setChunkLength(b(2*8));}
    RREP_ack (const RREP_ack  &m);
    RREP_ack &  operator= (const RREP_ack &m);
    virtual RREP_ack *dup() const override {return new RREP_ack(*this);}
    uint8_t getReserved() const {return reserved;}
  private:
    void copy(const RREP_ack& other) { reserved = other.reserved; }
} ;
#define RREP_ACK_SIZE 2

class RREQ : public AODV_msg
{
public:
    uint8_t j;     /* Join flag (multicast) */
    uint8_t r;     /* Repair flag */
    uint8_t g;     /* Gratuitous RREP flag */
    uint8_t d;     /* Destination only respond */
    uint8_t res1;
    uint8_t res2;
    uint8_t hcnt;
    uint32_t rreq_id;
//  uint32_t dest_addr;
    L3Address dest_addr;
    uint32_t dest_seqno;
//  uint32_t orig_addr;
    L3Address orig_addr;
    uint32_t orig_seqno;
    uint32_t   cost;
    uint8_t  hopfix;
    explicit RREQ() : AODV_msg ()
    {
        j = 0;
        r = 0;     /* Repair flag */
        g = 0;     /* Gratuitous RREP flag */
        d = 0;     /* Destination only respond */
        res1 = 0;
        res2 = 0;
        hcnt  = 0;
        rreq_id = 0;
        dest_addr = L3Address();
        dest_seqno = 0;
        orig_addr = L3Address();
        orig_seqno = 0;
        cost = 0;
        hopfix = 0;
        ManetRoutingBase * owner = check_and_cast<ManetRoutingBase*>(this->getOwner());
        setChunkLength(b((16+(owner->getAddressSize()*2))*8));
    }

    RREQ (const RREQ &m);
    RREQ &  operator= (const RREQ &m);
    virtual RREQ *dup() const override {return new RREQ(*this);}
    virtual std::string str() const override;
    uint8_t getJ() const {return j;}
    uint8_t getR() const {return r;}
    uint8_t getG() const {return g;}
    uint8_t getD() const {return d;}
    uint8_t getRes1() const {return res1;}
    uint8_t getRes2() const {return res2;}
    uint8_t getHcnt() const {return hcnt;}
    uint32_t getRreq_id() const {return rreq_id;}
    //uint32_t getDest_addr() const {return dest_addr;}
    const L3Address& getDest_addr() const {return dest_addr;}
    uint32_t getDest_seqno() const {return dest_seqno;}
    //u_int32_t getOrig_addr() const {return orig_addr;}
    const L3Address& getOrig_addr() const {return orig_addr;}
    uint32_t getOrig_seqno() const {return orig_seqno;}
    uint32_t getCost() const {return cost;}
    uint8_t getHopfix() const {return hopfix;}
  private:
    void copy(const RREQ& other);
};
// 24 for IPv4
//#define RREQ_SIZE 16+(AddressSizeUsed*2)

} // namespace inetmanet

} // namespace inet
#endif

