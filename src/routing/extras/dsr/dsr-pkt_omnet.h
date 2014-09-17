/* Copyright (C) Uppsala University
 *
 * This file is distributed under the terms of the GNU general Public
 * License (GPL), see the file LICENSE
 *
 * Author: Erik Nordström, <erikn@it.uu.se>
 */
#ifndef __INET_DSR_OMNETPP_H
#define __INET_DSR_OMNETPP_H

#ifndef OMNETPP
#define OMNETPP
#endif

// #include <endian.h>
#include "inet/routing/extras/base/compatibility_dsr.h"
#include "dsr-uu/dsr-opt.h"



#ifdef MobilityFramework
#include "NetwPkt_m.h"
#ifndef IPv4Address
#define IPv4Address int
#endif
#define IPv4Datagram NetwPkt
#else
#include "inet/networklayer/ipv4/IPv4Datagram.h"
#include "inet/networklayer/common/IPProtocolId_m.h"
#endif

#ifdef MobilityFramework

#ifndef IPv4Datagram
#define IPv4Datagram NetwPkt
#endif

#ifndef IPv4Address
#define IPv4Address int

#endif



#endif

namespace inet {

namespace inetmanet {

class EtxCost
{
  public:
    IPv4Address address;
    double cost;
    ~EtxCost() {}
};

class DSRPkt : public IPv4Datagram
{

  protected:
    struct dsr_opt_hdr *options;
    IPProtocolId encap_protocol;
    IPv4Address previous;
    IPv4Address next;
    EtxCost  *costVector;
    unsigned int costVectorSize;
    int dsr_ttl;

  private:
    void copy(const DSRPkt& other);
    void clean();

  public:
    explicit DSRPkt(const char *name=NULL) : IPv4Datagram(name) {costVector=NULL; options=NULL; costVectorSize=0; dsr_ttl=0;}
    ~DSRPkt ();
    DSRPkt (const DSRPkt  &m);
    DSRPkt (struct dsr_pkt *dp,int interface_id);
    DSRPkt &    operator= (const DSRPkt &m);
    virtual DSRPkt *dup() const {return new DSRPkt(*this);}
    //void addOption();
    //readOption();
#ifdef MobilityFramework
    void setTimeToLive (int ttl) {setTtl(ttl);}
    int getTimeToLive() {return getTtl();}
#endif
    void    ModOptions (struct dsr_pkt *p,int);
    void setEncapProtocol(IPProtocolId procotol) {encap_protocol = procotol;}
    IPProtocolId getEncapProtocol() {return encap_protocol;}
#ifdef MobilityFramework
    const IPv4Address& prevAddress() const {return previous;}
    void setPrevAddress(const IPv4Address& address_var) {previous = address_var;}
    const IPv4Address& nextAddress() const {return next;}
    void setNextAddress(const IPv4Address& address_var) {next = address_var;}
#else
    const IPv4Address prevAddress() const {return previous;}
    void setPrevAddress(const IPv4Address address_var) {previous = address_var;}
    const IPv4Address nextAddress() const {return next;}
    void setNextAddress(const IPv4Address address_var) {next = address_var;}
#endif
    struct dsr_opt_hdr * getOptions() const {return options;}
    void  setOptions(dsr_opt_hdr * op) {if (options !=NULL) free(options);  options=op;}
    virtual std::string detailedInfo() const;

    void resetCostVector();
    virtual void getCostVector(EtxCost &cost,int &size); // Copy
    virtual EtxCost* getCostVector() {return ((costVectorSize>0)?costVector:NULL);}

    virtual void setCostVector(EtxCost &cost, int size);
    virtual void setCostVector(EtxCost *cost,int size);

    virtual unsigned getCostVectorSize() const {return costVectorSize;}

    virtual void setCostVectorSize(unsigned n);
    virtual void setCostVectorSize(EtxCost);
    virtual void setCostVectorSize(u_int32_t addr, double cost);
};

class EtxList
{
  public:
    IPv4Address address;
    double delivery;// the simulation suppose that the code use a u_int32_t
};

class DSRPktExt: public IPv4Datagram
{
  protected:
    EtxList *extension;
    int size;

  private:
    void copy(const DSRPktExt& other);
    void clean() { clearExtension(); }

  public:
    explicit DSRPktExt(const char *name=NULL) : IPv4Datagram(name) {size=0; extension=NULL;}
    ~DSRPktExt ();
    DSRPktExt (const DSRPktExt  &m);
    DSRPktExt &     operator= (const DSRPktExt &m);
    void clearExtension();
    EtxList * addExtension(int len);
    EtxList * delExtension(int len);
    EtxList * getExtension() {return extension;}
    int getSizeExtension () {return size;}
    virtual DSRPktExt *dup() const {return new DSRPktExt(*this);}

};

} // namespace inetmanet

} // namespace inet

#endif              /* _DSR_PKT_H */

