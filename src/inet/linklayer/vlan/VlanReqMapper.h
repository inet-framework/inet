//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_VLANREQMAPPER_H
#define __INET_VLANREQMAPPER_H

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ModuleRefByPar.h"
#include "inet/linklayer/vlan/VlanIdMaps.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/queueing/base/PacketFlowBase.h"

namespace inet {

using namespace inet::queueing;

class INET_API VlanReqMapper : public PacketFlowBase, public TransparentProtocolRegistrationListener, public cListener
{
  protected:
    const Protocol *protocol = nullptr;
    ModuleRefByPar<IInterfaceTable> interfaceTable;
    std::map<std::string, std::map<int, int>> mappedVlanIds; // interface name or "*" -> requested VLAN id -> mapped VLAN id

  protected:
    virtual void initialize(int stage) override;
    virtual void processPacket(Packet *packet) override;

    virtual cGate *getRegistrationForwardingGate(cGate *gate) override;

    virtual void receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details) override;

  public:
    /**
     * Replaces the VLAN id map of the interface, which must be in the
     * interface table of this module. -1 stands for no tag, as key and as
     * mapped VLAN id.
     */
    void setInterfaceVlanIdMap(const NetworkInterface *networkInterface, const std::map<int, int>& vlanIdMap);

    /**
     * Removes the VLAN id map of the interface, so that the entry of "*"
     * applies to it, or, without such an entry, packets pass unchanged.
     */
    void removeInterfaceVlanIdMap(const NetworkInterface *networkInterface);
};

} // namespace inet

#endif

