//
// Copyright (C) 2008
// DSDV simple example for INET (add-on)
// Version 2.0
// Diogo Antio & Pedro Menezes
// Instituto Superior Tecnico
// Lisboa - Portugal
// This version and newer version can be found at http://dsdv.8rf.com
// This code was written while assisting the course "Redes moveis e sem fios" http://comp.ist.utl.pt/ec-cm
// Autorization to use and modify this code not needed :P
// The authors hope it will be useful to help understand how
// INET and OMNET++ works(more specifically INET 20061020 and omnet++ 3.3).
// Also we hope it will help in the developing of routing protocols using INET.
//

#ifndef __INET_DSDV_H
#define __INET_DSDV_H

#include <list>
#include <map>
#include <stdio.h>
#include <string.h>
#include <vector>

#include "inet/common/INETDefs.h"
#include "inet/common/packet/Packet.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/contract/ipv4/Ipv4Address.h"
#include "inet/networklayer/ipv4/IIpv4RoutingTable.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"
#include "inet/networklayer/ipv4/Ipv4RoutingTable.h"
#include "inet/routing/base/RoutingProtocolBase.h"
#include "inet/routing/dsdv/DsdvHello_m.h"

namespace inet {

/**
 * DSDV protocol implementation.
 */
class INET_API Dsdv : public RoutingProtocolBase
{
  private:
    struct ForwardEntry
    {
        cMessage *event = nullptr;
        Packet *hello = nullptr;

        ForwardEntry() {}
        ~ForwardEntry();
    };

    bool isForwardHello = false;
    cMessage *event = nullptr;
    cPar *broadcastDelay = nullptr;
    std::list<ForwardEntry *> *forwardList  = nullptr;
    InterfaceEntry *interface80211ptr = nullptr;
    int interfaceId = -1;
    unsigned int sequencenumber = 0;
    simtime_t routeLifetime;
    cModule *host = nullptr;

  protected:
    simtime_t helloInterval;
    IInterfaceTable *ift = nullptr;
    IIpv4RoutingTable *rt = nullptr;

  public:
    Dsdv();
    ~Dsdv();

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage *msg) override;

    void handleSelfMessage(cMessage *msg);

    // lifecycle
    virtual void handleStartOperation(LifecycleOperation *operation) override { start(); }
    virtual void handleStopOperation(LifecycleOperation *operation) override { stop(); }
    virtual void handleCrashOperation(LifecycleOperation *operation) override  { stop(); }
    void start();
    void stop();
};

/**
 * IPv4 route used by the DSDV protocol (DSDV module).
 */
class INET_API DsdvIpv4Route : public Ipv4Route
{
    protected:
        unsigned int sequencenumber; // originated from destination. Ensures loop freeness.
        simtime_t expiryTime;  // time the routing entry is valid until

    public:
        virtual bool isValid() const override { return expiryTime == 0 || expiryTime > simTime(); }

        simtime_t getExpiryTime() const {return expiryTime;}
        void setExpiryTime(simtime_t time) {expiryTime = time;}
        void setSequencenumber(int i) {sequencenumber = i;}
        unsigned int getSequencenumber() const {return sequencenumber;}
};

} // namespace inet

#endif // ifndef __INET_DSDV_H

