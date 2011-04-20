/* Copyright (C) Uppsala University
 *
 * This file is distributed under the terms of the GNU general Public
 * License (GPL), see the file LICENSE
 *
 * Author: Erik Nordström, <erikn@it.uu.se>
 */
#ifndef _DSR_OMNETPP_PKT_H
#define _DSR_OMNETPP_PKT_H

#ifndef OMNETPP
#define OMNETPP
#endif

// #include <endian.h>
#include "compatibility_dsr.h"
#include "dsr-uu/dsr-opt.h"


#ifdef MobilityFramework
#include <NetwPkt_m.h>
#ifndef IPAddress
#define IPAddress int
#endif
#define IPDatagram NetwPkt
#else
#include <IPDatagram.h>
#include <IPProtocolId_m.h>
#endif

#ifdef MobilityFramework

#ifndef IPDatagram
#define IPDatagram NetwPkt
#endif

#ifndef IPAddress
#define IPAddress int

#endif



#endif



class EtxCost
{
  public:
    IPAddress address;
    double cost;
    ~EtxCost() {}
};

class DSRPkt : public IPDatagram
{

  protected:
    struct dsr_opt_hdr *options;
    IPProtocolId encap_protocol;
    IPAddress previous;
    IPAddress next;
    EtxCost  *costVector;
    unsigned int costVectorSize;
    int dsr_ttl;

  public:
    explicit DSRPkt(const char *name=NULL) : IPDatagram(name) {costVector=NULL; options=NULL; costVectorSize=0; dsr_ttl=0;}
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
    const IPAddress& prevAddress() const {return previous;}
    void setPrevAddress(const IPAddress& address_var) {previous = address_var;}
    const IPAddress& nextAddress() const {return next;}
    void setNextAddress(const IPAddress& address_var) {next = address_var;}
#else
    const IPAddress prevAddress() const {return previous;}
    void setPrevAddress(const IPAddress address_var) {previous = address_var;}
    const IPAddress nextAddress() const {return next;}
    void setNextAddress(const IPAddress address_var) {next = address_var;}
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
    IPAddress address;
    double delivery;// the simulation suppose that the code use a u_int32_t
};

class DSRPktExt: public IPDatagram
{
  protected:
    EtxList *extension;
    int size;
  public:
    explicit DSRPktExt(const char *name=NULL) : IPDatagram(name) {size=0; extension=NULL;}
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


#endif              /* _DSR_PKT_H */
