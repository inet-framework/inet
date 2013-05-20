//
// Copyright (C) 2012 Kyeong Soo (Joseph) Kim
//


#ifndef __INET_UDPAPPCONNECTOR_H
#define __INET_UDPAPPCONNECTOR_H

#include <omnetpp.h>


class UDPAppConnector: public cSimpleModule
{
  protected:
    cMessage *msgServiced;
    cMessage *endServiceMsg;
    cQueue queue;
    simsignal_t qlenSignal;
    simsignal_t busySignal;
    simsignal_t queueingTimeSignal;

  public:
//    UDPAppConnector();
//    virtual ~UDPAppConnector();

  protected:
//    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
};


#endif
