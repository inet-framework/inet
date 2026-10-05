//
// Copyright (C) 2006 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mgmt/Ieee80211MgmtStaSimplified.h"
#include "inet/linklayer/ieee80211/mgmt/Ieee80211MgmtRateSet.h"

#include "inet/networklayer/common/L3AddressResolver.h"

namespace inet {

namespace ieee80211 {

Define_Module(Ieee80211MgmtStaSimplified);

static Ieee80211Mib *findAccessPointMib(const MacAddress& accessPointAddress, bool required = true)
{
    L3AddressResolver addressResolver;
    auto host = addressResolver.findHostWithAddress(accessPointAddress);
    if (host == nullptr) {
        if (required)
            throw cRuntimeError("Access point with address %s not found", accessPointAddress.str().c_str());
        return nullptr;
    }
    auto interfaceTable = addressResolver.findInterfaceTableOf(host);
    if (interfaceTable == nullptr) {
        if (required)
            throw cRuntimeError("Access point interface table with address %s not found", accessPointAddress.str().c_str());
        return nullptr;
    }
    auto networkInterface = interfaceTable->findInterfaceByAddress(accessPointAddress);
    if (networkInterface == nullptr) {
        if (required)
            throw cRuntimeError("Access point interface with address %s not found", accessPointAddress.str().c_str());
        return nullptr;
    }
    auto apMib = dynamic_cast<Ieee80211Mib *>(networkInterface->getSubmodule("mib"));
    if (apMib == nullptr && required)
        throw cRuntimeError("Access point MIB with address %s not found", accessPointAddress.str().c_str());
    return apMib;
}

void Ieee80211MgmtStaSimplified::initialize(int stage)
{
    Ieee80211MgmtBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        mib->mode = Ieee80211Mib::INFRASTRUCTURE;
        mib->bssStationData.stationType = Ieee80211Mib::STATION;
        mib->bssStationData.isAssociated = true;
    }
    else if (stage == INITSTAGE_LINK_LAYER) {
        configureAssociation();
    }
    else if (stage == INITSTAGE_LAST)
        configureAssociation();
}

void Ieee80211MgmtStaSimplified::handleStartOperation(LifecycleOperation *operation)
{
    Ieee80211MgmtBase::handleStartOperation(operation);
    if (operation != nullptr)
        configureAssociation();
}

void Ieee80211MgmtStaSimplified::configureAssociation()
{
    Ieee80211Mib::RateUpdate rateUpdate(*mib);
    L3AddressResolver addressResolver;
    auto accessPointAddress = addressResolver.resolve(par("accessPointAddress"), L3AddressResolver::ADDR_MAC).toMac();
    mib->bssData.bssid = accessPointAddress;
    auto apMib = findAccessPointMib(accessPointAddress);
    Ieee80211Mib::RateUpdate apRateUpdate(*apMib);
    apMib->bssAccessPointData.stations[mib->address] = Ieee80211Mib::ASSOCIATED;
    mib->bssData.ssid = apMib->bssData.ssid;
    mib->bssStationData.isAssociated = true;
    if (mib->getLocalRateSet().supported.known && apMib->getBssRateSet().basic.known) {
        if (!supportsBasicRateSet(mib->getLocalRateSet(), apMib->getBssRateSet()))
            throw cRuntimeError("Simplified station does not support the AP basic rate set");
        mib->installBssAndPeerRateSets(apMib->getBssRateSet(), accessPointAddress, apMib->getLocalRateSet());
        apMib->setPeerRateSet(mib->address, mib->getLocalRateSet());
    }
    // Simplified management is an explicit no-air abstraction: install the state that the
    // Association Request/Response exchange would have committed in detailed management.
    if (mib->isHtOperationSupported() && apMib->isHtOperationSupported()) {
        mib->setPeerHtCapabilities(apMib->address, apMib->localHtCapabilities, apMib->getHtOperation());
        apMib->setPeerHtCapabilities(mib->address, mib->localHtCapabilities, apMib->getHtOperation());
    }
}

void Ieee80211MgmtStaSimplified::stop()
{
    Ieee80211Mib::RateUpdate rateUpdate(*mib);
    mib->bssStationData.isAssociated = false;
    auto apMib = findAccessPointMib(mib->bssData.bssid, false);
    if (apMib != nullptr) {
        Ieee80211Mib::RateUpdate apRateUpdate(*apMib);
        apMib->bssAccessPointData.stations.erase(mib->address);
        apMib->removePeerHtCapabilities(mib->address);
        apMib->removePeerRateSet(mib->address);
    }
    Ieee80211MgmtBase::stop();
}

void Ieee80211MgmtStaSimplified::handleTimer(cMessage *msg)
{
    ASSERT(false);
}

void Ieee80211MgmtStaSimplified::handleCommand(int msgkind, cObject *ctrl)
{
    throw cRuntimeError("handleCommand(): no commands supported");
}

void Ieee80211MgmtStaSimplified::handleAuthenticationFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>& header)
{
    dropManagementFrame(packet);
}

void Ieee80211MgmtStaSimplified::handleDeauthenticationFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>& header)
{
    dropManagementFrame(packet);
}

void Ieee80211MgmtStaSimplified::handleAssociationRequestFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>& header)
{
    dropManagementFrame(packet);
}

void Ieee80211MgmtStaSimplified::handleAssociationResponseFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>& header)
{
    dropManagementFrame(packet);
}

void Ieee80211MgmtStaSimplified::handleReassociationRequestFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>& header)
{
    dropManagementFrame(packet);
}

void Ieee80211MgmtStaSimplified::handleReassociationResponseFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>& header)
{
    dropManagementFrame(packet);
}

void Ieee80211MgmtStaSimplified::handleDisassociationFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>& header)
{
    dropManagementFrame(packet);
}

void Ieee80211MgmtStaSimplified::handleBeaconFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>& header)
{
    dropManagementFrame(packet);
}

void Ieee80211MgmtStaSimplified::handleProbeRequestFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>& header)
{
    dropManagementFrame(packet);
}

void Ieee80211MgmtStaSimplified::handleProbeResponseFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>& header)
{
    dropManagementFrame(packet);
}

} // namespace ieee80211

} // namespace inet
