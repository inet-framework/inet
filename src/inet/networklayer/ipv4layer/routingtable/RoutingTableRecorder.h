//
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ROUTINGTABLERECORDER_H
#define __INET_ROUTINGTABLERECORDER_H

#include "inet/networklayer/ipv4/IIpv4RoutingTable.h"

namespace inet {

/**
 * Records routing table changes into a file.
 *
 * @see Ipv4RoutingTable, Ipv4Route
 */
class INET_API RoutingTableRecorder : public cSimpleModule, public cListener
{
  private:
    FILE *routingLogFile;

  public:
    RoutingTableRecorder();
    virtual ~RoutingTableRecorder();

    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) override { receiveChangeNotification(source, signalID, obj); }

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *) override;
    virtual void finish() override;
    virtual void hookListeners();
    virtual void ensureRoutingLogFileOpen();
    virtual void receiveChangeNotification(cComponent *source, simsignal_t signalID, cObject *details);
    virtual void recordInterfaceChange(cModule *host, const NetworkInterface *ie, simsignal_t signalID);
    virtual void recordRouteChange(cModule *host, const IRoute *route, simsignal_t signalID);
};

} // namespace inet

#endif

