//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IPV6NODECONFIGURATOR_H
#define __INET_IPV6NODECONFIGURATOR_H

#include "inet/common/SimpleModule.h"
#include "inet/common/ModuleRefByPar.h"
#include "inet/common/lifecycle/ILifecycle.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/networklayer/configurator/ipv6/Ipv6NetworkConfigurator.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/ipv6/Ipv6RoutingTable.h"

namespace inet {

/**
 * This module provides the static configuration for the Ipv6RoutingTable and
 * the Ipv6 network interfaces of a particular node in the network.
 *
 * For more info please see the NED file.
 */
class INET_API Ipv6NodeConfigurator : public SimpleModule, public ILifecycle, protected cListener
{
  protected:
    bool _configureRoutingTable = false;
    opp_component_ptr<NodeStatus> nodeStatus;
    ModuleRefByPar<IInterfaceTable> interfaceTable;
    ModuleRefByPar<Ipv6RoutingTable> routingTable;
    ModuleRefByPar<Ipv6NetworkConfigurator> networkConfigurator;

  public:
    Ipv6NodeConfigurator();

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


