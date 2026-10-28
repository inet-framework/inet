//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_VLANREQFILTER_H
#define __INET_VLANREQFILTER_H

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ModuleRefByPar.h"
#include "inet/linklayer/common/VlanTagType.h"
#include "inet/linklayer/vlan/VlanIdMaps.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/queueing/base/PacketFilterBase.h"

namespace inet {

using namespace inet::queueing;

class INET_API VlanReqFilter : public PacketFilterBase, public TransparentProtocolRegistrationListener, public cListener
{
  protected:
    VlanTagType vlanTagType = VLAN_TAG_TYPE_C;
    ModuleRefByPar<IInterfaceTable> interfaceTable;
    std::map<std::string, std::set<int>> acceptedVlanIds; // interface name or "*" -> accepted VLAN ids

  protected:
    virtual void initialize(int stage) override;
    virtual void processPacket(Packet *packet) override {}
    virtual void dropPacket(Packet *packet) override;

    virtual cGate *getRegistrationForwardingGate(cGate *gate) override;

  protected:
    virtual void receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details) override;

  public:
    virtual bool matchesPacket(const Packet *packet) const override;

    /**
     * Replaces the accepted VLAN ids of the interface, which must be in the
     * interface table of this module. -1 accepts untagged packets.
     */
    void setInterfaceAcceptedVlanIds(const NetworkInterface *networkInterface, const std::set<int>& vlanIds);

    /**
     * Removes the accepted VLAN ids of the interface, so that the entry of
     * "*" applies to it, or, without such an entry, no packet is accepted.
     */
    void removeInterfaceAcceptedVlanIds(const NetworkInterface *networkInterface);
};

} // namespace inet

#endif

