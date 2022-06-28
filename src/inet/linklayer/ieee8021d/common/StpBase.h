//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_STPBASE_H
#define __INET_STPBASE_H

#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/lifecycle/OperationalBase.h"
#include "inet/linklayer/common/MacAddress.h"
#include "inet/linklayer/configurator/Ieee8021dInterfaceData.h"
#include "inet/linklayer/ethernet/contract/IMacForwardingTable.h"
#include "inet/networklayer/common/InterfaceTable.h"

namespace inet {

/**
 * Base class for Stp and Rstp.
 */
class INET_API StpBase : public OperationalBase, public cListener
{
  protected:
    bool visualize = false; // if true it visualize the spanning tree
    unsigned int numPorts = 0; // number of ports

    unsigned int bridgePriority = 0; // bridge's priority
    MacAddress bridgeAddress; // bridge's MAC address

    simtime_t maxAge;
    simtime_t helloTime;
    simtime_t forwardDelay;

    opp_component_ptr<cModule> switchModule;
    ModuleRefByPar<IMacForwardingTable> macTable;
    ModuleRefByPar<IInterfaceTable> ifTable;
    opp_component_ptr<NetworkInterface> ie;

  public:
    StpBase();
    virtual bool isInitializeStage(int stage) const override { return stage == INITSTAGE_LINK_LAYER; }
    virtual bool isModuleStartStage(int stage) const override { return stage == ModuleStartOperation::STAGE_LINK_LAYER; }
    virtual bool isModuleStopStage(int stage) const override { return stage == ModuleStopOperation::STAGE_LINK_LAYER; }
    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;

    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) override {}

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;

    virtual void start();
    virtual void stop();

    void sendOut(Packet *packet, int interfaceId, const MacAddress& destAddress);

    /**
     * @brief Adds effects to be represented by Tkenv. Colors the link black if forwarding parameter is true
     * and the port to which the link is connected to is also forwarding, otherwise colors the link gray.
     */
    virtual void colorLink(NetworkInterface *ie, bool forwarding) const;

    /**
     * @brief Adds effects to be represented by Tkenv. Inactive links are colored grey.
     * Shows port role, state. Marks root switch.
     */
    virtual void refreshDisplay() const override;

    /**
     * @brief Obtains the root interface ID.
     * @return The root gate interface ID or -1 if there is no root interface.
     */
    virtual int getRootInterfaceId() const;

    /**
     * @brief Gets Ieee8021dInterfaceData for interface ID.
     * @return The port's Ieee8021dInterfaceData, or throws error if it doesn't exist.
     */
    const Ieee8021dInterfaceData *getPortInterfaceData(unsigned int interfaceId) const;
    Ieee8021dInterfaceData *getPortInterfaceDataForUpdate(unsigned int interfaceId);

    /**
     * @brief Gets NetworkInterface for interface ID.
     * @return The port's NetworkInterface, throws error if it doesn't exist.
     */
    NetworkInterface *getPortNetworkInterface(unsigned int interfaceId) const;

    /*
     * Returns the first non-loopback interface.
     */
    virtual NetworkInterface *chooseInterface();
};

} // namespace inet

#endif

