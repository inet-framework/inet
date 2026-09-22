//
// Copyright (C) 2006 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mgmt/Ieee80211MgmtStaSimplified.h"

#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/linklayer/ieee80211/mgmt/contract/IIeee80211BssProvider.h"

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
        mib->configureBssRole(Ieee80211Mib::STATION);
    }
    else if (stage == INITSTAGE_LINK_LAYER) {
        if (isUp())
            configureAssociation();
    }
}

void Ieee80211MgmtStaSimplified::handleStartOperation(LifecycleOperation *operation)
{
    Ieee80211MgmtBase::handleStartOperation(operation);
    if (operation != nullptr)
        configureAssociation();
}

void Ieee80211MgmtStaSimplified::configureAssociation()
{
    L3AddressResolver addressResolver;
    auto accessPointAddress = addressResolver.resolve(par("accessPointAddress"), L3AddressResolver::ADDR_MAC).toMac();
    auto apMib = findAccessPointMib(accessPointAddress);
    auto *apManagement = check_and_cast<IIeee80211BssProvider *>(apMib->getParentModule()->getSubmodule("mgmt"));
    apManagement->prepareBss();
    mib->commitBss(apMib->getBssData().ssid, accessPointAddress, apMib->getOperationBand(),
            apMib->hasPrimaryChannel() ? apMib->requirePrimaryChannel() : -1,
            mib->isLocalHtCapable() && apMib->hasHtOperation() ? &apMib->getHtOperation() : nullptr);
    mib->setAssociated(true);
    // Simplified management is an explicit no-air abstraction: install the state that the
    // Association Request/Response exchange would have committed in detailed management.
    if (mib->isLocalHtCapable() && apMib->isLocalHtCapable()) {
        mib->setPeerHtCapabilities(apMib->address, apMib->getLocalHtCapabilities());

    }
    apManagement->installSimplifiedPeer(mib->address, mib->isLocalHtCapable() ? &mib->getLocalHtCapabilities() : nullptr);
    mib->publishStateChange();
}

void Ieee80211MgmtStaSimplified::stop()
{
    auto accessPointAddress = mib->getBssData().bssid;
    mib->clearBss();
    auto apMib = findAccessPointMib(accessPointAddress, false);
    if (apMib != nullptr) {
        auto *apManagement = check_and_cast<IIeee80211BssProvider *>(apMib->getParentModule()->getSubmodule("mgmt"));
        apManagement->removeSimplifiedPeer(mib->address);
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

