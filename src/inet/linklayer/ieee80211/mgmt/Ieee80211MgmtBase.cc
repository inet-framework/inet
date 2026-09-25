//
// Copyright (C) 2006 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mgmt/Ieee80211MgmtBase.h"

#include "inet/common/INETUtils.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/lifecycle/LifecycleOperation.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/ieee80211/mgmt/Ieee80211HtMgmtElements.h"
#include "inet/linklayer/ieee80211/mac/contract/IIeee80211ModeSetProvider.h"
#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211Tag_m.h"

#include <cmath>

namespace inet {

namespace ieee80211 {

using namespace inet::physicallayer;

void Ieee80211MgmtBase::initialize(int stage)
{
    // OperationalBase starts management at this stage. Install policy first.
    if (stage == INITSTAGE_NETWORK_CONFIGURATION)
        initializeLocalRateState();
    OperationalBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        mib.reference(this, "mibModule", true);
        interfaceTable.reference(this, "interfaceTableModule", true);
        myIface = getContainingNicModule(this);
        numMgmtFramesReceived = 0;
        numMgmtFramesDropped = 0;
        basicRatesPolicy = par("basicRates").stdstringValue();
        operationalRatesPolicy = par("operationalRates").stdstringValue();
        getContainingNicModule(this)->subscribe(modesetChangedSignal, this);
        WATCH(numMgmtFramesReceived);
        WATCH(numMgmtFramesDropped);
    }
}

Ieee80211RateSet Ieee80211MgmtBase::parseRatePolicy(const std::string& policy, const char *parameterName) const
{
    Ieee80211RateSet result;
    result.known = true;
    if (policy == "auto") {
        result.known = false;
        return result;
    }
    for (const auto& token : cStringTokenizer(policy.c_str()).asVector()) {
        std::string actualUnit;
        try {
            cValue::parseQuantity(token.c_str(), actualUnit);
            if (actualUnit.empty())
                throw cRuntimeError("missing unit");
            double value = cValue::parseQuantity(token.c_str(), "bps");
            bps rate(value);
            if (rate.get() <= 0 || std::fmod(rate.get(), 500000) != 0)
                throw cRuntimeError("rate is not a positive multiple of 500 kbps");
            result.legacyRates.insert(rate);
        }
        catch (const std::exception& e) {
            throw cRuntimeError("Invalid %s token '%s': %s", parameterName, token.c_str(), e.what());
        }
    }
    return result;
}

void Ieee80211MgmtBase::initializeLocalRateState()
{
    auto macModule = getModuleFromPar<IIeee80211ModeSetProvider>(par("macModule"), this);
    modeSet = macModule->getModeSet();
    localRateSet = Ieee80211RateSetState();
    localRateSet.supported.known = true;
    localRateSet.operational.known = true;
    localRateSet.basic.known = true;
    for (const auto *mode : modeSet->getLegacyOperationalModes()) {
        auto rate = mode->getDataMode()->getNetBitrate();
        localRateSet.supported.legacyRates.insert(rate);
        localRateSet.operational.legacyRates.insert(rate);
        if (modeSet->getIsMandatory(mode))
            localRateSet.basic.legacyRates.insert(rate);
    }
    if (mib->isHtOperationSupported()) {
        for (int mcs = 0; mcs < 77; mcs++) {
            if (mib->localHtCapabilities.rxMcsSupported[mcs])
                localRateSet.supported.htMcs.insert(mcs);
            if (mib->localHtCapabilities.rxMcsSupported[mcs])
                localRateSet.operational.htMcs.insert(mcs);
            if (mib->hasPrimaryChannel() && mib->getHtOperation().basicMcsSupported[mcs])
                localRateSet.basic.htMcs.insert(mcs);
        }
    }
    auto explicitOperational = parseRatePolicy(operationalRatesPolicy, "operationalRates");
    auto explicitBasic = parseRatePolicy(basicRatesPolicy, "basicRates");
    if (explicitOperational.known)
        localRateSet.operational.legacyRates = explicitOperational.legacyRates;
    if (explicitBasic.known)
        localRateSet.basic.legacyRates = explicitBasic.legacyRates;
    mib->setLocalRateSet(localRateSet);
    if (mib->mode == Ieee80211Mib::INDEPENDENT || mib->bssStationData.stationType == Ieee80211Mib::ACCESS_POINT)
        mib->setBssRateSet(localRateSet);
    updateSupportedRateElements();
}

void Ieee80211MgmtBase::updateSupportedRateElements()
{
    supportedRates = Ieee80211SupportedRatesElement();
    extendedSupportedRates = Ieee80211ExtendedSupportedRatesElement();
    const auto& rateSet = localRateSet.operational.legacyRates;
    int rateIndex = 0;
    int extendedRateIndex = 0;
    for (const auto *mode : modeSet->getLegacyOperationalModes()) {
        auto rate = mode->getDataMode()->getNetBitrate();
        if (rateSet.count(rate) == 0)
            continue;
        bool isBasicRate = localRateSet.basic.legacyRates.count(rate) != 0;
        if (rateIndex < 8) {
            supportedRates.rate[rateIndex] = rate.get<Mbps>();
            supportedRates.basicRate[rateIndex] = isBasicRate;
            rateIndex++;
        }
        else if (extendedRateIndex < 255) {
            extendedSupportedRates.rate[extendedRateIndex] = rate.get<Mbps>();
            extendedSupportedRates.basicRate[extendedRateIndex] = isBasicRate;
            extendedRateIndex++;
        }
        else
            throw cRuntimeError("Mode set '%s' contains more than 263 selected legacy rates", modeSet->getName());
    }
    supportedRates.numRates = rateIndex;
    extendedSupportedRates.numRates = extendedRateIndex;
}

Ieee80211RateSetState Ieee80211MgmtBase::makeRateSetState(const Ieee80211SupportedRatesElement& primaryRates,
        bool hasExtendedRates, const Ieee80211ExtendedSupportedRatesElement& extraRates,
        const Ieee80211HtCapabilities *htCapabilities, const Ieee80211HtOperation *htOperation) const
{
    Ieee80211RateSetState result;
    result.supported.known = true;
    result.basic.known = true;
    result.operational.known = true;
    auto addRate = [&result](double megabits, bool basic) {
        bps rate = Mbps(megabits);
        result.supported.legacyRates.insert(rate);
        result.operational.legacyRates.insert(rate);
        if (basic)
            result.basic.legacyRates.insert(rate);
    };
    for (int i = 0; i < primaryRates.numRates; i++)
        addRate(primaryRates.rate[i], primaryRates.basicRate[i]);
    if (hasExtendedRates)
        for (int i = 0; i < extraRates.numRates; i++)
            addRate(extraRates.rate[i], extraRates.basicRate[i]);
    if (htCapabilities != nullptr) {
        for (int mcs = 0; mcs < 77; mcs++) {
            if (htCapabilities->rxMcsSupported[mcs]) {
                result.supported.htMcs.insert(mcs);
                result.operational.htMcs.insert(mcs);
            }
            if (htOperation != nullptr && htOperation->basicMcsSupported[mcs])
                result.basic.htMcs.insert(mcs);
        }
    }
    return result;
}

void Ieee80211MgmtBase::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(signalID));

    if (signalID == modesetChangedSignal) {
        modeSet = check_and_cast<physicallayer::Ieee80211ModeSet *>(obj);
        if (localRateSet.operational.known)
            initializeLocalRateState();
    }
}

void Ieee80211MgmtBase::addHtCapabilities(const Ptr<Ieee80211MgmtFrame>& frame) const
{
    if (mib->isHtOperationSupported())
        setHtCapabilities(frame, mib->localHtCapabilities);
}

void Ieee80211MgmtBase::addHtOperation(const Ptr<Ieee80211MgmtFrame>& frame, const physicallayer::IIeee80211Band *band) const
{
    if (mib->isHtOperationSupported())
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
    if (localRateSet.operational.known &&
            (mib->mode == Ieee80211Mib::INDEPENDENT || mib->bssStationData.stationType == Ieee80211Mib::ACCESS_POINT))
        mib->setBssRateSet(localRateSet);
}

void Ieee80211MgmtBase::stop()
{
    mib->clearPeerHtCapabilities();
    mib->clearPeerRateSets();
    mib->clearBssRateSet();
}

} // namespace ieee80211

} // namespace inet
