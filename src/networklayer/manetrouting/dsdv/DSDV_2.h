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

#ifndef __DSDV_2_H__
#define __DSDV_2_H__

#include <stdio.h>
#include <string.h>
#include <vector>
#include <map>
#include <list>

#include "INETDefs.h"

#include "IPv4InterfaceData.h"
#include "IPv4Address.h"
#include "IPv4ControlInfo.h"
#include "IPv4Datagram.h"
#include "IInterfaceTable.h"
#include "InterfaceTableAccess.h"
#include "IRoutingTable.h"
#include "RoutingTable.h"
#include "RoutingTableAccess.h"

using namespace std;

/**
 * DSDV module implementation.
 */
class INET_API DSDV_2 : public cSimpleModule
{
  private:
    cMessage *event;
    struct forwardHello
    {
        cMessage *event;
        DSDV_HelloMessage *hello;
        ~forwardHello();
        forwardHello();
    };
    list<forwardHello*> *forwardList;
    //DSDV_HelloMessage *Hello;
    InterfaceEntry *interface80211ptr;
    int interfaceId;
    unsigned int sequencenumber;
    simtime_t routeLifetime;

  protected:
    simtime_t hellomsgperiod_DSDV;
    IInterfaceTable *ift;
    IRoutingTable *rt;

  public:
    DSDV_2();
    ~DSDV_2();

  protected:
    int numInitStages() const  {return 5;}
    virtual void initialize(int stage);
    virtual void handleMessage(cMessage *msg);

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
        virtual bool isValid() const { return expiryTime == 0 || expiryTime > simTime(); }

        simtime_t getExpiryTime() const {return expiryTime;}
        void setExpiryTime(simtime_t time) {expiryTime = time;}
        void setSequencenumber(int i) {sequencenumber = i;}
        unsigned int getSequencenumber() const {return sequencenumber;}
};

#endif
