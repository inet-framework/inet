//
// Copyright (C) 2006 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mgmt/Ieee80211MgmtBase.h"

#include "inet/common/INETUtils.h"
#include "inet/common/ModuleAccess.h"
#include "inet/physicallayer/wireless/ieee80211/contract/packetlevel/IIeee80211ModeSetCoordinator.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/lifecycle/LifecycleOperation.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/ieee80211/mgmt/Ieee80211HtMgmtElements.h"
#include "inet/linklayer/ieee80211/mgmt/Ieee80211VhtMgmtElements.h"
#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211Tag_m.h"

namespace inet {

namespace ieee80211 {

using namespace inet::physicallayer;

void Ieee80211MgmtBase::initialize(int stage)
{
    OperationalBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        mib.reference(this, "mibModule", true);
        interfaceTable.reference(this, "interfaceTableModule", true);
        myIface = getContainingNicModule(this);
        numMgmtFramesReceived = 0;
        numMgmtFramesDropped = 0;
        configurationProvider.reference(this, "macModule", true);
        if (auto coordinator = dynamic_cast<IIeee80211ModeSetCoordinator *>(configurationProvider.get()))
            coordinator->registerModeSetConsumer(this, IIeee80211ModeSetCoordinator::MANAGEMENT_STATE);
        WATCH(numMgmtFramesReceived);
        WATCH(numMgmtFramesDropped);
    }
    else if (stage == INITSTAGE_LINK_LAYER) {
        prepareConfiguration();
        if (isUp())
            prepareLocalOperation();
        mib->publishStateChange();
    }
}

void Ieee80211MgmtBase::prepareConfiguration()
{
    if (configurationPrepared)
        return;
    configurationProvider->prepareLocalCapabilities();
    modeSet = configurationProvider->getConfiguredModeSet();
    if (modeSet == nullptr)
        throw cRuntimeError("Configured IEEE 802.11 mode catalog is unavailable");
    configurationPrepared = true;
    updateSupportedRates();
}

void Ieee80211MgmtBase::applyModeSet(const Ieee80211ModeSet *newModeSet)
{
    Enter_Method_Silent();
    modeSet = newModeSet;
    updateSupportedRates();
    if (isUp() && mib->hasActiveBss())
        prepareLocalOperation();
}

void Ieee80211MgmtBase::updateSupportedRates()
{
    supportedRates = Ieee80211SupportedRatesElement();
    extendedSupportedRates = Ieee80211ExtendedSupportedRatesElement();
    int rateIndex = 0;
    int extendedRateIndex = 0;
    // Supported Rates carries the legacy OperationalRateSet only. HT/VHT
    // MCS support is advertised through the corresponding capabilities
    // elements (IEEE Std 802.11-2024, 9.4.2.3, 9.4.2.54.4, 11.1.4.6).
    for (const auto *mode : modeSet->getLegacyOperationalModes()) {
        bool isBasicRate = modeSet->getIsMandatory(mode);
        double rate = mode->getDataMode()->getNetBitrate().get<Mbps>();
        if (rateIndex < 8) {
            supportedRates.rate[rateIndex] = rate;
            supportedRates.basicRate[rateIndex] = isBasicRate;
            rateIndex++;
        }
        else if (extendedRateIndex < 255) {
            extendedSupportedRates.rate[extendedRateIndex] = rate;
            extendedSupportedRates.basicRate[extendedRateIndex] = isBasicRate;
            extendedRateIndex++;
        }
        else
            throw cRuntimeError("Mode set '%s' contains more than 263 legacy operational rates", modeSet->getName());
    }
    supportedRates.numRates = rateIndex;
    extendedSupportedRates.numRates = extendedRateIndex;
}

Ieee80211HtOperation Ieee80211MgmtBase::computeLocalHtOperation(int primaryChannel, const IIeee80211Band *band) const
{
    Ieee80211HtOperation operation;
    operation.primaryChannel = primaryChannel;
    int offset = mib->par("htSecondaryChannelOffset");
    if (offset != 0 && offset != 1 && offset != 3)
        throw cRuntimeError("htSecondaryChannelOffset must be 0, 1, or 3");
    if (offset != 0 && mib->getLocalHtCapabilities().supportedChannelWidths.count(MHz(40)) == 0)
        throw cRuntimeError("40 MHz HT operation requires a configured PHY that can operate a 40 MHz channel width");
    if (offset != 0 && band != nullptr && !band->isHt40OperationSupported(primaryChannel, offset)) {
        EV_WARN << "Configured 40 MHz HT operation is unsupported on primary channel " << primaryChannel
                << " in band '" << band->getName() << "'; falling back to 20 MHz BSS operation.\n";
        offset = 0;
    }
    operation.secondaryChannelOffset = offset;
    operation.operatingChannelWidth = offset != 0 ? MHz(40) : MHz(20);
    int protection = mib->par("htProtectionMode");
    if (protection < 0 || protection > 3)
        throw cRuntimeError("htProtectionMode must be between 0 and 3");
    operation.protectionMode = static_cast<Ieee80211HtProtectionMode>(protection);
    const auto& mandatory = modeSet->getHtMcsMandatory();
    for (int mcs = 0; mcs < 77; mcs++)
        operation.basicMcsSupported[mcs] = mandatory[mcs] && mib->getLocalHtCapabilities().rxMcsSupported[mcs];
    return operation;
}

void Ieee80211MgmtBase::prepareLocalOperation()
{

}

void Ieee80211MgmtBase::addVhtCapabilities(const Ptr<Ieee80211MgmtFrame>& frame) const
{
    if (mib->isVhtOperationSupported())
        setVhtCapabilities(frame, mib->getLocalVhtCapabilities());
}

void Ieee80211MgmtBase::addVhtOperation(const Ptr<Ieee80211MgmtFrame>& frame) const
{
    if (mib->isVhtOperationSupported())
        setVhtOperation(frame, mib->getLocalVhtOperation());
}

void Ieee80211MgmtBase::addHtCapabilities(const Ptr<Ieee80211MgmtFrame>& frame) const
{
    if (mib->isLocalHtCapable())
        setHtCapabilities(frame, mib->getLocalHtCapabilities());
}

void Ieee80211MgmtBase::addHtOperation(const Ptr<Ieee80211MgmtFrame>& frame, const physicallayer::IIeee80211Band *band) const
{
    if (mib->isLocalHtCapable())
        setHtOperation(frame, band, mib->getHtOperation());
}

void Ieee80211MgmtBase::handleMessageWhenUp(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        // process timers
        EV << "Timer expired: " << msg << "\n";
        handleTimer(msg);
    }
    else if (msg->arrivedOn("macIn")) {
        // process incoming frame
        EV << "Frame arrived from MAC: " << msg << "\n";
        auto packet = check_and_cast<Packet *>(msg);
        const Ptr<const Ieee80211DataOrMgmtHeader>& header = packet->peekAt<Ieee80211DataOrMgmtHeader>(packet->getFrontOffset() - B(24));
        processFrame(packet, header);
    }
    else if (msg->arrivedOn("agentIn")) {
        // process command from agent
        EV << "Command arrived from agent: " << msg << "\n";
        int msgkind = msg->getKind();
        cObject *ctrl = msg->removeControlInfo();
        delete msg;

        handleCommand(msgkind, ctrl);
    }
    else
        throw cRuntimeError("Unknown message");
    mib->publishStateChange();
}

void Ieee80211MgmtBase::sendDown(Packet *frame)
{
    ASSERT(isUp());
    frame->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::ieee80211Mgmt);
    send(frame, "macOut");
}

void Ieee80211MgmtBase::dropManagementFrame(Packet *frame)
{
    EV << "ignoring management frame: " << (cMessage *)frame << "\n";
    delete frame;
    numMgmtFramesDropped++;
}

void Ieee80211MgmtBase::processFrame(Packet *packet, const Ptr<const Ieee80211DataOrMgmtHeader>& header)
{
    switch (header->getType()) {
        case ST_AUTHENTICATION:
            numMgmtFramesReceived++;
            handleAuthenticationFrame(packet, dynamicPtrCast<const Ieee80211MgmtHeader>(header));
            break;

        case ST_DEAUTHENTICATION:
            numMgmtFramesReceived++;
            handleDeauthenticationFrame(packet, dynamicPtrCast<const Ieee80211MgmtHeader>(header));
            break;

        case ST_ASSOCIATIONREQUEST:
            numMgmtFramesReceived++;
            handleAssociationRequestFrame(packet, dynamicPtrCast<const Ieee80211MgmtHeader>(header));
            break;

        case ST_ASSOCIATIONRESPONSE:
            numMgmtFramesReceived++;
            handleAssociationResponseFrame(packet, dynamicPtrCast<const Ieee80211MgmtHeader>(header));
            break;

        case ST_REASSOCIATIONREQUEST:
            numMgmtFramesReceived++;
            handleReassociationRequestFrame(packet, dynamicPtrCast<const Ieee80211MgmtHeader>(header));
            break;

        case ST_REASSOCIATIONRESPONSE:
            numMgmtFramesReceived++;
            handleReassociationResponseFrame(packet, dynamicPtrCast<const Ieee80211MgmtHeader>(header));
            break;

        case ST_DISASSOCIATION:
            numMgmtFramesReceived++;
            handleDisassociationFrame(packet, dynamicPtrCast<const Ieee80211MgmtHeader>(header));
            break;

        case ST_BEACON:
            numMgmtFramesReceived++;
            handleBeaconFrame(packet, dynamicPtrCast<const Ieee80211MgmtHeader>(header));
            break;

        case ST_PROBEREQUEST:
            numMgmtFramesReceived++;
            handleProbeRequestFrame(packet, dynamicPtrCast<const Ieee80211MgmtHeader>(header));
            break;

        case ST_PROBERESPONSE:
            numMgmtFramesReceived++;
            handleProbeResponseFrame(packet, dynamicPtrCast<const Ieee80211MgmtHeader>(header));
            break;

        default:
            throw cRuntimeError("Unexpected frame type (%s)%s", packet->getClassName(), packet->getName());
    }
}

void Ieee80211MgmtBase::start()
{
    if (configurationPrepared)
        prepareLocalOperation();
}

void Ieee80211MgmtBase::stop()
{
    mib->clearBss();
    mib->publishStateChange();
}

} // namespace ieee80211

} // namespace inet
