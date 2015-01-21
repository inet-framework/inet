/*
 * Copyright (C) 2008
 * DSDV simple example for INET (add-on)
 * Version 2.0
 * Diogo Ant�o & Pedro Menezes
 * Instituto Superior T�cnico
 * Lisboa - Portugal
 * This version and newer version can be found at http://dsdv.8rf.com
 * This code was written while assisting the course "Redes m�veis e sem fios" http://comp.ist.utl.pt/ec-cm
 * Autorization to use and modify this code not needed :P
 * The authors hope it will be useful to help understand how
 * INET and OMNET++ works(more specifically INET 20061020 and omnet++ 3.3).
 * Also we hope it will help in the developing of routing protocols using INET.
*/

#ifndef ___INET_DSDV_2_H
#define ___INET_DSDV_2_H

#include <stdio.h>
#include <string.h>
#include <vector>
#include <map>
#include <list>

#include "inet/common/INETDefs.h"

#include "inet/networklayer/ipv4/IPv4InterfaceData.h"
#include "inet/networklayer/contract/ipv4/IPv4Address.h"
#include "inet/networklayer/contract/ipv4/IPv4ControlInfo.h"
#include "inet/networklayer/ipv4/IPv4Datagram.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/ipv4/IIPv4RoutingTable.h"
#include "inet/networklayer/ipv4/IPv4RoutingTable.h"

namespace inet {

namespace inetmanet {

using namespace std;

/**
 * DSDV module implementation.
 */
class INET_API DSDV_2 : public cSimpleModule    // proactive protocol, don't need to ManetIPv4Hook
{
  private:
    cMessage *event = nullptr;
    cPar *broadcastDelay = nullptr;
    struct forwardHello
    {
        cMessage *event = nullptr;
        DSDV_HelloMessage *hello = nullptr;
        forwardHello() {}
        ~forwardHello();
    };
    list<forwardHello*> *forwardList  = nullptr;
    //DSDV_HelloMessage *Hello;
    InterfaceEntry *interface80211ptr = nullptr;
    int interfaceId = -1;
    unsigned int sequencenumber = 0;
    simtime_t routeLifetime;

  protected:
    simtime_t hellomsgperiod_DSDV;
    IInterfaceTable *ift = nullptr;
    IIPv4RoutingTable *rt = nullptr;

  public:
    DSDV_2();
    ~DSDV_2();

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;

};

/**
 * IPv4 route used by the DSDV protocol (DSDV_2 module).
 */
class INET_API DSDVIPv4Route : public IPv4Route
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

} // namespace inetmanet

} // namespace inet

#endif

