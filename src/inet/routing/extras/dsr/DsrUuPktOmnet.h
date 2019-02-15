/* Copyright (C) Uppsala University
 *
 * This file is distributed under the terms of the GNU general Public
 * License (GPL), see the file LICENSE
 *
 * Author: Erik Nordström, <erikn@it.uu.se>
 */
#ifndef __INET_DSR_OMNETPP_H
#define __INET_DSR_OMNETPP_H


#include <vector>
#include "inet/common/INETDefs.h"
#include "inet/common/packet/chunk/FieldsChunk.h"
#include "inet/networklayer/common/IpProtocolId_m.h"
#include "inet/networklayer/common/L3Address.h"
#include "inet/common/packet/Packet.h"
#include "inet/routing/extras/dsr/dsr-uu-omnetpp.h"

#ifndef WITH_IPv4
#error Ipv4 must be defined for DSR
#endif

namespace inet {

namespace inetmanet {


class EtxCost
{
  public:
    L3Address address;
    double cost;
    ~EtxCost() {}
};

class DSRPkt : public FieldsChunk
{

  protected:
    std::vector<struct dsr_opt_hdr> dsrOptions;
    L3Address previous;
    L3Address next;
    std::vector <EtxCost>  costVector;
    int dsr_ttl;
    int encapProtocol = -1;

  private:
    void copy(const DSRPkt& other);
    void clean();

  public:
    virtual void cleanAll() {clean(); previous = L3Address(); next = L3Address(); dsr_ttl = 0; encapProtocol = -1; };
    explicit DSRPkt() : FieldsChunk() {costVector.clear(); dsrOptions.clear(); dsr_ttl=0;}
    ~DSRPkt ();
    DSRPkt (const DSRPkt  &m);
    DSRPkt &  operator= (const DSRPkt &m);
    virtual DSRPkt *dup() const override {return new DSRPkt(*this);}
    //void addOption();
    //readOption();
    void setTimeToLive (int ttl) {dsr_ttl = ttl;}
    int getTimeToLive() {return dsr_ttl;}
    void modDsrOptions (struct dsr_pkt *p,int);
    void setEncapProtocol(IpProtocolId procotol) {encapProtocol = procotol;}
    int getEncapProtocol() {return encapProtocol;}
    const L3Address getPrevAddress() const {return previous;}
    void setPrevAddress(const L3Address address_var) {previous = address_var;}
    const L3Address getNextAddress() const {return next;}
    void setNextAddress(const L3Address address_var) {next = address_var;}

    std::vector<struct dsr_opt_hdr> &getDsrOptions() {return dsrOptions;}
    void  setDsrOptions(const std::vector<struct dsr_opt_hdr> &op) {dsrOptions.clear(); dsrOptions=op;}
    void clearDsrOptions(){dsrOptions.clear();}
    virtual std::string str() const override;

    void resetCostVector();
    virtual void getCostVector(std::vector<EtxCost> &cost); // Copy
    virtual std::vector<EtxCost> getCostVector() {return costVector;}

    virtual void setCostVector(std::vector<EtxCost> &cost);

    virtual unsigned getCostVectorSize() const {return costVector.size();}

    virtual void setCostVectorSize(EtxCost);
    virtual void setCostVectorSize(L3Address addr, double cost);
};

class EtxList
{
  public:
    L3Address address;
    double delivery;// the simulation suppose that the code use a u_int32_t
};

class DSRPktExt: public DSRPkt
{
  protected:
    EtxList *extension;
    int size;

  private:
    void copy(const DSRPktExt& other);
    void clean() { clearExtension(); }

  public:
    virtual void cleanAll() override {DSRPkt::cleanAll(); clean();}
    explicit DSRPktExt() : DSRPkt() {size=0; extension=nullptr;}
    ~DSRPktExt ();
    DSRPktExt (const DSRPktExt  &m);
    DSRPktExt &     operator= (const DSRPktExt &m);
    void clearExtension();
    EtxList * addExtension(int len);
    EtxList * delExtension(int len);
    EtxList * getExtension() {return extension;}
    int getSizeExtension () {return size;}
    virtual DSRPktExt *dup() const override {return new DSRPktExt(*this);}

};

const Ptr<const DSRPkt> findDsrProtocolHeader(Packet *packet);

const Ptr<const DSRPkt> getDsrProtocolHeader(Packet *packet);

void insertDsrProtocolHeader(Packet *packet, const Ptr<DSRPkt>& header);

const Ptr<DSRPkt> removeDsrProtocolHeader(Packet *packet);


} // namespace inetmanet

} // namespace inet

#endif              /* _DSR_PKT_H */
