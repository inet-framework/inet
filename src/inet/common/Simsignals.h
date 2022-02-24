//
// Copyright (C) 2005 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SIMSIGNALS_H
#define __INET_SIMSIGNALS_H

#include "inet/common/Simsignals_m.h"

namespace inet {

/**
 * Signals for publish/subscribe mechanisms.
 */
// TODO document associated signals detail structs
extern INET_API simsignal_t // admin
// - layer 2 (data-link)
// TODO generalize constants (remove "PP"?) - could be used by 80211 and ethernet as well
// they generally carry TxNotifDetails as "details" to identify the interface and the frame
    l2BeaconLostSignal, // missed several consecutive beacons (currently Ieee80211)
    l2AssociatedSignal, // successfully associated with an AP (currently Ieee80211)
    l2AssociatedNewApSignal, // successfully associated with an AP (currently Ieee80211)
    l2AssociatedOldApSignal,
    l2DisassociatedSignal, // same as BEACON_LOST but used in higher layers
    l2ApAssociatedSignal, // emitted by the AP, successfully associated with this AP (currently Ieee80211)
    l2ApDisassociatedSignal, // emitted by the AP, successfully disassociated from this AP (currently Ieee80211)

    linkBrokenSignal, // used for manet link layer feedback

    modesetChangedSignal,

    interpacketGapStartedSignal,
    interpacketGapEndedSignal,

// - layer 3 (network)
    interfaceCreatedSignal,
    interfaceDeletedSignal,
    interfaceStateChangedSignal,
    interfaceConfigChangedSignal,
    interfaceGnpConfigChangedSignal,
    interfaceIpv4ConfigChangedSignal,
    interfaceIpv6ConfigChangedSignal,
    interfaceClnsConfigChangedSignal,
    tedChangedSignal,

// layer 3 - Routing Table
    routeAddedSignal,
    routeDeletedSignal,
    routeChangedSignal,
    mrouteAddedSignal,
    mrouteDeletedSignal,
    mrouteChangedSignal,

// layer 3 - Ipv4
    ipv4MulticastChangeSignal,
    ipv4MulticastGroupJoinedSignal,
    ipv4MulticastGroupLeftSignal,
    ipv4MulticastGroupRegisteredSignal,
    ipv4MulticastGroupUnregisteredSignal,

// for PIM
    ipv4NewMulticastSignal,
    ipv4DataOnNonrpfSignal,
    ipv4DataOnRpfSignal,
    ipv4MdataRegisterSignal,
    pimNeighborAddedSignal,
    pimNeighborDeletedSignal,
    pimNeighborChangedSignal,

// layer 3 - Ipv6
    ipv6HandoverOccurredSignal,
    mipv6RoCompletedSignal,
    ipv6MulticastGroupJoinedSignal,
    ipv6MulticastGroupLeftSignal,
    ipv6MulticastGroupRegisteredSignal,
    ipv6MulticastGroupUnregisteredSignal,

// layer 3 - ISIS
    isisAdjChangedSignal,

// - layer 4 (transport)
// ...

// - layer 7 (application)
// ...

// general
    packetCreatedSignal,
    packetAddedSignal,
    packetRemovedSignal,
    packetDroppedSignal,

    packetSentToUpperSignal,
    packetReceivedFromUpperSignal,

    packetSentToLowerSignal,
    packetReceivedFromLowerSignal,

    packetSentToPeerSignal,
    packetReceivedFromPeerSignal,

    packetSentSignal,
    packetReceivedSignal,

    packetPushedSignal,
    packetPushedInSignal,
    packetPushedOutSignal,
    packetPushStartedSignal,
    packetPushEndedSignal,

    packetPulledSignal,
    packetPulledInSignal,
    packetPulledOutSignal,
    packetPullStartedSignal,
    packetPullEndedSignal,

    packetFilteredSignal,

    packetFlowStartedSignal,
    packetFlowEndedSignal,

    transmissionStartedSignal,
    transmissionEndedSignal,
    receptionStartedSignal,
    receptionEndedSignal,

    tokensChangedSignal,
    tokensAddedSignal,
    tokensRemovedSignal,
    tokensDepletedSignal;

/**
 * Utility function
 */
void printSignalBanner(simsignal_t signalID, const cObject *obj, const cObject *details);
void printSignalBanner(simsignal_t signalID, intval_t value, const cObject *details);

} // namespace inet

#endif

