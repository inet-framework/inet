//
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IPV4NODECONFIGURATOR_H
#define __INET_IPV4NODECONFIGURATOR_H

#include "inet/common/ModuleRefByPar.h"
#include "inet/common/lifecycle/ILifecycle.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/networklayer/configurator/ipv4/Ipv4NetworkConfigurator.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/ipv4/IIpv4RoutingTable.h"

namespace inet {

/**
 * This module provides the static configuration for the Ipv4RoutingTable and
 * the Ipv4 network interfaces of a particular node in the network.
 *
 * For more info please see the NED file.
 */
class INET_API Ipv4NodeConfigurator : public cSimpleModule, public ILifecycle, protected cListener
{
  protected:
    opp_component_ptr<NodeStatus> nodeStatus;
    ModuleRefByPar<IInterfaceTable> interfaceTable;
    ModuleRefByPar<IIpv4RoutingTable> routingTable;
    ModuleRefByPar<Ipv4NetworkConfigurator> networkConfigurator;

  public:
    Ipv4NodeConfigurator();

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void handleMessage(cMessage *msg) override { throw cRuntimeError("this module doesn't handle messages, it runs only in initialize()"); }
    virtual void initialize(int stage) override;
    virtual bool handleOperationStage(LifecycleOperation *operation, IDoneCallback *doneCallback) override;
    virtual void prepareAllInterfaces();
    virtual void prepareInterface(NetworkInterface *networkInterface);
    virtual void configureAllInterfaces();
    virtual void configureRoutingTable();

    /**
     * Called by the signal handler whenever a change of a category
     * occurs to which this client has subscribed.
     */
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) override;
};

} // namespace inet

#endif

