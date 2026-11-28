//
// Copyright (C) 2006 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// Contributor: Miguel González-López (shared HT capability validation and management rate publication)
//


#include "inet/linklayer/ieee80211/mgmt/Ieee80211MgmtBase.h"

#include <cmath>

#include "inet/common/INETUtils.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/lifecycle/LifecycleOperation.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/ieee80211/mgmt/Ieee80211HtMgmtElements.h"
#include "inet/linklayer/ieee80211/IIeee80211Mac.h"
#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211Tag_m.h"

namespace inet {

namespace ieee80211 {

using namespace inet::physicallayer;

void Ieee80211MgmtBase::resetPeerAssociationContext(const MacAddress& peer)
{
    auto mac = getModuleFromPar<IIeee80211Mac>(par("macModule"), this);
    mac->resetPeerAssociationContext(peer);
}

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
    mib->setLocalRateSet(makeRateSet(supportedRates, extendedRateIndex > 0, extendedSupportedRates));
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
    bool vhtSupported = false;
    for (int i = 0; i < modeSet->getNumModes(); i++)
        vhtSupported |= modeSet->getMode(i)->getModulationClass() == IIeee80211Mode::ModulationClass::VHT;
    // IEEE Std 802.11-2024, 10.27.3.3 and 11.38.1: preserve the configured RIFS policy.
    operation.rifsMode = !vhtSupported && (operation.protectionMode == Ieee80211HtProtectionMode::NO_PROTECTION ||
            operation.protectionMode == Ieee80211HtProtectionMode::TWENTY_MHZ_PROTECTION);
    const auto& mandatory = modeSet->getHtMcsMandatory();
    for (int mcs = 0; mcs < 77; mcs++)
        operation.basicMcsSupported[mcs] = mandatory[mcs] && mib->getLocalHtCapabilities().rxMcsSupported[mcs];
    return operation;
}

void Ieee80211MgmtBase::prepareLocalOperation()
{

}

Ieee80211RateSetState Ieee80211MgmtBase::makeRateSet(const Ieee80211SupportedRatesElement& rates,
        bool extendedPresent, const Ieee80211ExtendedSupportedRatesElement& extendedRates, bool basicKnown)
{
    Ieee80211RateSetState state;
    if (rates.numRates < 0 || rates.numRates > 8 ||
            (extendedPresent && (extendedRates.numRates < 1 || extendedRates.numRates > 255)))
        throw cRuntimeError("Invalid management rate element count");
    // Zero rates preserve absent information in abstract management inputs.
    // The serializer requires a nonempty Supported Rates element on the wire.
    if (rates.numRates == 0)
        return state;
    state.supported.known = state.operational.known = true;
    state.basic.known = basicKnown;
    auto addRates = [&state, basicKnown](const auto& element) {
        for (int i = 0; i < element.numRates; i++) {
            // Reject nonfinite values before ordered-set comparison can discard them.
            if (!std::isfinite(element.rate[i]))
                throw cRuntimeError("Invalid nonfinite management rate");
            auto rate = Mbps(element.rate[i]);
            state.supported.legacyRates.insert(rate);
            state.operational.legacyRates.insert(rate);
            if (basicKnown && element.basicRate[i])
                state.basic.legacyRates.insert(rate);
        }
    };
    // IEEE Std 802.11-2024, 9.4.2.3 and 9.4.2.11: both elements
    // describe operational rates. Request frames do not supply BSS basic rates.
    addRates(rates);
    if (extendedPresent)
        addRates(extendedRates);
    validateIeee80211RateSetState(state, "management rate state");
    // IEEE Std 802.11-2024, Table 9-131: values 122..127 are BSS membership
    // selectors, not legacy rates. Selector-based join checks remain unmodeled.
    for (int selector = 122; selector <= 127; selector++) {
        auto rate = Mbps(selector * 0.5);
        state.supported.legacyRates.erase(rate);
        state.operational.legacyRates.erase(rate);
        state.basic.legacyRates.erase(rate);
    }
    return state;
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
