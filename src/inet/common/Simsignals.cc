//
// Copyright (C) 2005 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/Simsignals.h"

#include <stdio.h>

namespace inet {

simsignal_t l2BeaconLostSignal = cComponent::registerSignal("l2BeaconLost");
simsignal_t l2AssociatedSignal = cComponent::registerSignal("l2Associated");
simsignal_t l2AssociatedNewApSignal = cComponent::registerSignal("l2AssociatedNewAp");
simsignal_t l2AssociatedOldApSignal = cComponent::registerSignal("l2AssociatedOldAp");
simsignal_t l2DisassociatedSignal = cComponent::registerSignal("l2Disassociated");
simsignal_t l2ApAssociatedSignal = cComponent::registerSignal("l2ApAssociated");
simsignal_t l2ApDisassociatedSignal = cComponent::registerSignal("l2ApDisassociated");

simsignal_t linkBrokenSignal = cComponent::registerSignal("linkBroken");

simsignal_t modesetChangedSignal = cComponent::registerSignal("modesetChanged");

simsignal_t interpacketGapStartedSignal = cComponent::registerSignal("interpacketGapStarted");
simsignal_t interpacketGapEndedSignal = cComponent::registerSignal("interpacketGapEnded");

// - layer 3 (network)
simsignal_t interfaceCreatedSignal = cComponent::registerSignal("interfaceCreated");
simsignal_t interfaceDeletedSignal = cComponent::registerSignal("interfaceDeleted");
simsignal_t interfaceStateChangedSignal = cComponent::registerSignal("interfaceStateChanged");
simsignal_t interfaceConfigChangedSignal = cComponent::registerSignal("interfaceConfigChanged");
simsignal_t interfaceGnpConfigChangedSignal = cComponent::registerSignal("interfaceGnpConfigChanged");
simsignal_t interfaceIpv4ConfigChangedSignal = cComponent::registerSignal("interfaceIpv4ConfigChanged");
simsignal_t interfaceIpv6ConfigChangedSignal = cComponent::registerSignal("interfaceIpv6ConfigChanged");
simsignal_t interfaceClnsConfigChangedSignal = cComponent::registerSignal("interfaceClnsConfigChanged");
simsignal_t tedChangedSignal = cComponent::registerSignal("tedChanged");

// layer 3 - Routing Table
simsignal_t routeAddedSignal = cComponent::registerSignal("routeAdded");
simsignal_t routeDeletedSignal = cComponent::registerSignal("routeDeleted");
simsignal_t routeChangedSignal = cComponent::registerSignal("routeChanged");
simsignal_t mrouteAddedSignal = cComponent::registerSignal("mrouteAdded");
simsignal_t mrouteDeletedSignal = cComponent::registerSignal("mrouteDeleted");
simsignal_t mrouteChangedSignal = cComponent::registerSignal("mrouteChanged");

// layer 3 - Ipv4
simsignal_t ipv4MulticastGroupJoinedSignal = cComponent::registerSignal("ipv4MulticastGroupJoined");
simsignal_t ipv4MulticastGroupLeftSignal = cComponent::registerSignal("ipv4MulticastGroupLeft");
simsignal_t ipv4MulticastChangeSignal = cComponent::registerSignal("ipv4McastChange");
simsignal_t ipv4MulticastGroupRegisteredSignal = cComponent::registerSignal("ipv4MulticastGroupRegistered");
simsignal_t ipv4MulticastGroupUnregisteredSignal = cComponent::registerSignal("ipv4MulticastGroupUnregistered");

// for PIM
simsignal_t ipv4NewMulticastSignal = cComponent::registerSignal("ipv4NewMulticast");
simsignal_t ipv4DataOnNonrpfSignal = cComponent::registerSignal("ipv4DataOnNonrpf");
simsignal_t ipv4DataOnRpfSignal = cComponent::registerSignal("ipv4DataOnRpf");
simsignal_t ipv4MdataRegisterSignal = cComponent::registerSignal("ipv4MdataRegister");
simsignal_t pimNeighborAddedSignal = cComponent::registerSignal("pimNeighborAdded");
simsignal_t pimNeighborDeletedSignal = cComponent::registerSignal("pimNeighborDeleted");
simsignal_t pimNeighborChangedSignal = cComponent::registerSignal("pimNeighborChanged");

// layer 3 - Ipv6
simsignal_t ipv6HandoverOccurredSignal = cComponent::registerSignal("ipv6HandoverOccurred");
simsignal_t mipv6RoCompletedSignal = cComponent::registerSignal("mipv6RoCompleted");
simsignal_t ipv6MulticastGroupJoinedSignal = cComponent::registerSignal("ipv6MulticastGroupJoined");
simsignal_t ipv6MulticastGroupLeftSignal = cComponent::registerSignal("ipv6MulticastGroupLeft");
simsignal_t ipv6MulticastGroupRegisteredSignal = cComponent::registerSignal("ipv6MulticastGroupRegistered");
simsignal_t ipv6MulticastGroupUnregisteredSignal = cComponent::registerSignal("ipv6MulticastGroupUnregistered");

// layer 3 - ISIS
simsignal_t isisAdjChangedSignal = cComponent::registerSignal("isisAdjChanged");

// - layer 4 (transport)
// ...

// - layer 7 (application)
// ...

// general
simsignal_t packetCreatedSignal = cComponent::registerSignal("packetCreated");
simsignal_t packetAddedSignal = cComponent::registerSignal("packetAdded");
simsignal_t packetRemovedSignal = cComponent::registerSignal("packetRemoved");
simsignal_t packetDroppedSignal = cComponent::registerSignal("packetDropped");

simsignal_t packetSentToUpperSignal = cComponent::registerSignal("packetSentToUpper");
simsignal_t packetReceivedFromUpperSignal = cComponent::registerSignal("packetReceivedFromUpper");

simsignal_t packetSentToLowerSignal = cComponent::registerSignal("packetSentToLower");
simsignal_t packetReceivedFromLowerSignal = cComponent::registerSignal("packetReceivedFromLower");

simsignal_t packetSentToPeerSignal = cComponent::registerSignal("packetSentToPeer");
simsignal_t packetReceivedFromPeerSignal = cComponent::registerSignal("packetReceivedFromPeer");

simsignal_t packetSentSignal = cComponent::registerSignal("packetSent");
simsignal_t packetReceivedSignal = cComponent::registerSignal("packetReceived");

simsignal_t packetPushedSignal = cComponent::registerSignal("packetPushed");
simsignal_t packetPushedInSignal = cComponent::registerSignal("packetPushedIn");
simsignal_t packetPushedOutSignal = cComponent::registerSignal("packetPushedOut");
simsignal_t packetPushStartedSignal = cComponent::registerSignal("packetPushStarted");
simsignal_t packetPushEndedSignal = cComponent::registerSignal("packetPushEnded");

simsignal_t packetPulledSignal = cComponent::registerSignal("packetPulled");
simsignal_t packetPulledInSignal = cComponent::registerSignal("packetPulledIn");
simsignal_t packetPulledOutSignal = cComponent::registerSignal("packetPulledOut");
simsignal_t packetPullStartedSignal = cComponent::registerSignal("packetPullStarted");
simsignal_t packetPullEndedSignal = cComponent::registerSignal("packetPullEnded");

simsignal_t packetFilteredSignal = cComponent::registerSignal("packetFiltered");

simsignal_t packetFlowStartedSignal = cComponent::registerSignal("packetFlowStarted");
simsignal_t packetFlowEndedSignal = cComponent::registerSignal("packetFlowEnded");

simsignal_t transmissionStartedSignal = cComponent::registerSignal("transmissionStarted");
simsignal_t transmissionEndedSignal = cComponent::registerSignal("transmissionEnded");
simsignal_t receptionStartedSignal = cComponent::registerSignal("receptionStarted");
simsignal_t receptionEndedSignal = cComponent::registerSignal("receptionEnded");

simsignal_t tokensChangedSignal = cComponent::registerSignal("tokensChanged");
simsignal_t tokensAddedSignal = cComponent::registerSignal("tokensAdded");
simsignal_t tokensRemovedSignal = cComponent::registerSignal("tokensRemoved");
simsignal_t tokensDepletedSignal = cComponent::registerSignal("tokensDepleted");

void printSignalBanner(simsignal_t signalID, const cObject *obj, const cObject *details)
{
    EV << "** Signal at T=" << simTime()
       << " to " << cSimulation::getActiveSimulation()->getContextModule()->getFullPath() << ": "
       << cComponent::getSignalName(signalID) << " "
       << (obj ? obj->str() : "") << " "
       << (details ? details->str() : "")
       << "\n";
}

void printSignalBanner(simsignal_t signalID, intval_t value, const cObject *details)
{
    EV << "** Signal at T=" << simTime()
       << " to " << cSimulation::getActiveSimulation()->getContextModule()->getFullPath() << ": "
       << cComponent::getSignalName(signalID) << " "
       << value << " "
       << (details ? details->str() : "")
       << "\n";
}

} // namespace inet

