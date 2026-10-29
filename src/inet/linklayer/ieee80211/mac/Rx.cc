//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/Rx.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/checksum/Checksum.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211InterframeInd_m.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/SignalTag_m.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211Radio.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211Tag_m.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Mac.h"
#include "inet/linklayer/ieee80211/mac/contract/IContention.h"
#include "inet/linklayer/ieee80211/mac/contract/ITx.h"

namespace inet {
namespace ieee80211 {

using namespace inet::physicallayer;

simsignal_t Rx::navChangedSignal = cComponent::registerSignal("navChanged");

Define_Module(Rx);

Rx::Rx()
{
}

Rx::~Rx()
{
    if (networkInterface) {
        for (auto signal : {receptionStartedSignal, receptionEndedSignal, transmissionStartedSignal,
                transmissionEndedSignal, FlatRadioBase::listeningChangedSignal, IRadio::radioModeChangedSignal})
            networkInterface->unsubscribe(signal, this);
    }
    cancelAndDelete(endNavTimer);
    cancelAndDelete(endTxnavTimer);
}

void Rx::initialize(int stage)
{
    ModeSetListener::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        mib.reference(this, "mibModule", true);
        recipientAckPolicy.reference(this, "recipientAckPolicyModule", *par("recipientAckPolicyModule").stringValue() != '\0');
        networkInterface = getContainingNicModule(this);
        for (auto signal : {receptionStartedSignal, receptionEndedSignal, transmissionStartedSignal,
                transmissionEndedSignal, FlatRadioBase::listeningChangedSignal, IRadio::radioModeChangedSignal})
            networkInterface->subscribe(signal, this);
        endNavTimer = new cMessage("NAV");
        endTxnavTimer = new cMessage("endTxnavTimer");
        WATCH(address);
        WATCH(receptionState);
        WATCH(transmissionState);
        WATCH(receivedPart);
        WATCH(mediumFree);
        WATCH_EXPR("rxStatus", getRxStatusTxt());
    }
    // TODO INITSTAGE
    else if (stage == INITSTAGE_NETWORK_INTERFACE_CONFIGURATION) {
        address = check_and_cast<Ieee80211Mac *>(getContainingNicModule(this)->getSubmodule("mac"))->getAddress();
        recomputeMediumFree();
    }
}

std::string Rx::getRxStatusTxt() const
{
    if (contentionFree)
        return "FREE";
    std::string s = "BUSY (";
    bool addSpace = false;
    if (transmissionState != IRadio::TRANSMISSION_STATE_UNDEFINED) {
        switch (transmissionState) {
            case IRadio::TRANSMISSION_STATE_IDLE: s += "Tx-Idle"; break;
            case IRadio::TRANSMISSION_STATE_TRANSMITTING: s += "Tx"; break;
            default: break;
        }
        addSpace = true;
    }
    else {
        switch (receptionState) {
            case IRadio::RECEPTION_STATE_UNDEFINED: s += "Switching"; break;
            case IRadio::RECEPTION_STATE_IDLE: s += "Rx-Idle"; break;
            case IRadio::RECEPTION_STATE_BUSY: s += "Noise"; break;
            case IRadio::RECEPTION_STATE_RECEIVING: s += "Recv"; break;
            default: break;
        }
        addSpace = true;
    }
    if (endNavTimer->isScheduled())
        s += std::string(addSpace ? " " : "") + "NAV";
    if (getTxnavRemaining() > 0)
        s += " TXNAV";
    if (contentionBlocked)
        s += " HCF";
    s += ")";
    return s;
}

void Rx::handleMessage(cMessage *msg)
{
    if (msg == endNavTimer) {
        EV_INFO << "The radio channel has become free according to the NAV" << std::endl;
        emit(navChangedSignal, SimTime::ZERO);
        recomputeMediumFree();
    }
    else if (msg == endTxnavTimer) {
        recomputeMediumFree();
    }
    else
        throw cRuntimeError("Unexpected self message");
}

bool Rx::lowerFrameReceived(Packet *packet)
{
    Enter_Method("lowerFrameReceived(\"%s\")", packet->getName());
    take(packet);

    // Aggregate containers need member acceptance before standalone FCS or header checks.
    // Reject them until MAC aggregate reception exists; PHY range outcomes stay separate.
    auto aggregate = packet->findTag<Ieee80211AmpduInd>();
    bool isFrameOk = !(aggregate && aggregate->getAggregation()) && isFcsOk(packet);
    attachInterframeResult(packet, isFrameOk);
    if (isFrameOk) {
        EV_INFO << "Received frame from PHY: " << packet << endl;
        const auto& header = packet->peekAtFront<Ieee80211MacHeader>();
        if (header->getReceiverAddress() != address)
            setOrExtendNav(header->getDurationField());
        return true;
    }
    else {
        EV_INFO << "Received an erroneous frame from PHY, dropping it." << std::endl;
        PacketDropDetails details;
        details.setReason(INCORRECTLY_RECEIVED);
        emit(packetDroppedSignal, packet, &details);
        delete packet;
        for (auto contention : contentions)
            contention->corruptedFrameReceived();
        return false;
    }
}

void Rx::frameTransmitted(simtime_t durationField)
{
    Enter_Method("frameTransmitted");
    // the txIndex that transmitted the frame should already own the TXOP, so
    // it has no need to (and should not) check the NAV.
    setOrExtendNav(durationField);
}

bool Rx::isReceptionInProgress() const
{
    return receptionState == IRadio::RECEPTION_STATE_RECEIVING &&
           (receivedPart == IRadioSignal::SIGNAL_PART_WHOLE || receivedPart == IRadioSignal::SIGNAL_PART_DATA);
}

bool Rx::isFcsOk(Packet *packet) const
{
    if (packet->hasBitError() || !packet->peekData()->isCorrect())
        return false;
    else {
        const auto& trailer = packet->peekAtBack<Ieee80211MacTrailer>(B(4));
        switch (trailer->getFcsMode()) {
            case FCS_DECLARED_INCORRECT:
                return false;
            case FCS_DECLARED_CORRECT:
                return true;
            case FCS_COMPUTED: {
                const auto& fcsBytes = packet->peekDataAt<BytesChunk>(B(0), packet->getDataLength() - trailer->getChunkLength());
                auto bufferLength = fcsBytes->getChunkLength().get<B>();
                auto buffer = new uint8_t[bufferLength];
                fcsBytes->copyToBuffer(buffer, bufferLength);
                auto computedFcs = ethernetFcs(buffer, bufferLength);
                delete[] buffer;
                return computedFcs == trailer->getFcs();
            }
            default:
                throw cRuntimeError("Unknown FCS mode");
        }
    }
}

void Rx::recomputeMediumFree()
{
    bool oldContentionFree = contentionFree;
    // note: the duration of mode switching (rx-to-tx or tx-to-rx) should also count as busy
    mediumFree = receptionState == IRadio::RECEPTION_STATE_IDLE && transmissionState == IRadio::TRANSMISSION_STATE_UNDEFINED && !endNavTimer->isScheduled();
    // IEEE Std 802.11-2024, 10.23.2.2: active TXNAV excludes local contention.
    contentionFree = mediumFree && getTxnavRemaining() == 0 && !contentionBlocked;
    if (contentionFree != oldContentionFree) {
        for (auto contention : contentions)
            contention->mediumStateChanged(contentionFree);
    }
}

void Rx::receptionStateChanged(IRadio::ReceptionState state)
{
    Enter_Method("receptionStateChanged");
    discardExpiredReception();
    if (state == IRadio::RECEPTION_STATE_BUSY)
        invalidateFutureInterframeHistory();
    else if (state == IRadio::RECEPTION_STATE_IDLE && activeReception) {
        activeReception.reset();
        precedingFrame.reset();
    }
    receptionState = state;
    recomputeMediumFree();
}

void Rx::receivedSignalPartChanged(IRadioSignal::SignalPart part)
{
    Enter_Method("receivedSignalPartChanged");
    receivedPart = part;
    recomputeMediumFree();
}

void Rx::transmissionStateChanged(IRadio::TransmissionState state)
{
    Enter_Method("transmissionStateChanged");
    transmissionState = state;
    recomputeMediumFree();
}

void Rx::setOrExtendNav(simtime_t navInterval)
{
    ASSERT(navInterval >= 0);
    if (navInterval > 0) {
        simtime_t endNav = simTime() + navInterval;
        if (endNavTimer->isScheduled()) {
            simtime_t oldEndNav = endNavTimer->getArrivalTime();
            if (endNav < oldEndNav)
                return; // never decrease NAV
            emit(navChangedSignal, endNavTimer->getArrivalTime() - simTime());
            cancelEvent(endNavTimer);
        }
        else
            emit(navChangedSignal, SimTime::ZERO);
        EV_INFO << "Setting NAV to " << navInterval << std::endl;
        scheduleAt(endNav, endNavTimer);
        emit(navChangedSignal, endNav - simTime());
        recomputeMediumFree();
    }
}

void Rx::registerContention(IContention *contention)
{
    contention->mediumStateChanged(contentionFree);
    contentions.push_back(contention);
}

simtime_t Rx::getTxnavRemaining() const
{
    return endTxnavTimer && endTxnavTimer->isScheduled() ?
        std::max(SIMTIME_ZERO, endTxnavTimer->getArrivalTime() - simTime()) : SIMTIME_ZERO;
}

void Rx::successfulFrameTransmitted(simtime_t ppduEnd, simtime_t durationField)
{
    Enter_Method("successfulFrameTransmitted");
    ASSERT(durationField >= 0);
    // IEEE Std 802.11-2024, 10.23.2.2: the latest successful holder field replaces TXNAV.
    // The countdown starts at PPDU end, not at response reception.
    cancelEvent(endTxnavTimer);
    auto endpoint = ppduEnd + durationField;
    if (durationField > 0 && endpoint > simTime())
        scheduleAt(endpoint, endTxnavTimer);
    recomputeMediumFree();
}

void Rx::setContentionBlocked(bool blocked)
{
    Enter_Method("setContentionBlocked");
    contentionBlocked = blocked;
    recomputeMediumFree();
}

void Rx::discardExpiredReception()
{
    if (completedReception && completedReception->end < simTime())
        completedReception.reset();
}

void Rx::invalidateFutureInterframeHistory()
{
    precedingFrame.reset();
    activeReception.reset();
    if (completedReception)
        completedReception->mayBecomePredecessor = false;
}

void Rx::invalidateInterframeHistory()
{
    Enter_Method("invalidateInterframeHistory");
    invalidateFutureInterframeHistory();
    completedReception.reset();
    ambiguityEnd = -1;
}

void Rx::observeReceptionStart(const IReception *reception)
{
    auto start = reception->getStartTime();
    auto end = reception->getEndTime();
    if (completedReception)
        completedReception->mayBecomePredecessor = false;
    if (start < ambiguityEnd || (activeReception && start < activeReception->end)) {
        ambiguityEnd = std::max(ambiguityEnd, end);
        if (activeReception)
            ambiguityEnd = std::max(ambiguityEnd, activeReception->end);
        invalidateFutureInterframeHistory();
        return;
    }
    ambiguityEnd = -1;
    activeReception = ReceptionEvidence{start, end, precedingFrame,
        modeSet ? modeSet->getSifsTime() : SIMTIME_ZERO,
        modeSet ? modeSet->getSlotTime() / 10 : SIMTIME_ZERO, true};
    // IEEE Std 802.11-2024, 10.27.3.1, 10.27.3.3, and 11.38.1: freeze BSS permission at reception start.
    activeReception->rifsPermitted = recipientAckPolicy && modeSet &&
        modeSet->getPhyType() == Ieee80211ModeSet::PhyType::HT &&
        (mib->mode == Ieee80211Mib::INDEPENDENT || mib->mode == Ieee80211Mib::MESH ||
            (mib->hasPrimaryChannel() && mib->getHtOperation().rifsMode));
    if (activeReception->rifsPermitted)
        activeReception->rifs = modeSet->getReferenceMode()->getRifsTime();
    precedingFrame.reset();
}

void Rx::observeReceptionEnd(const IReception *reception)
{
    if (activeReception && activeReception->start == reception->getStartTime() &&
            activeReception->end == reception->getEndTime()) {
        completedReception = std::move(activeReception);
        activeReception.reset();
    }
}

bool Rx::isCompletedAck(const ITransmission *transmission) const
{
    auto packet = transmission ? transmission->getPacket() : nullptr;
    if (!packet || !packet->findTag<PacketProtocolTag>() || packet->getDataLength() <= b(0))
        return false;
    auto protocol = packet->getTag<PacketProtocolTag>()->getProtocol();
    if (protocol != &Protocol::ieee80211FhssPhy && protocol != &Protocol::ieee80211IrPhy &&
            protocol != &Protocol::ieee80211DsssPhy && protocol != &Protocol::ieee80211HrDsssPhy &&
            protocol != &Protocol::ieee80211OfdmPhy && protocol != &Protocol::ieee80211ErpOfdmPhy &&
            protocol != &Protocol::ieee80211HtPhy && protocol != &Protocol::ieee80211VhtPhy)
        return false;
    // Optional evidence must not turn an unsupported representation into a MAC error.
    try {
        int flags = Chunk::PF_ALLOW_INCORRECT | Chunk::PF_ALLOW_INCOMPLETE | Chunk::PF_ALLOW_IMPROPERLY_REPRESENTED;
        auto phy = Ieee80211Radio::peekIeee80211PhyHeaderAtFront(packet, b(-1), flags);
        if (!phy->isCorrect() || phy->isIncomplete() || phy->isImproperlyRepresented() ||
                phy->getChunkLength() <= b(0) || phy->getChunkLength() + LENGTH_ACK > packet->getDataLength())
            return false;
        auto mac = packet->peekAt<Ieee80211MacHeader>(phy->getChunkLength(), b(-1), flags);
        return mac->isCorrect() && !mac->isIncomplete() && !mac->isImproperlyRepresented() && mac->getType() == ST_ACK;
    }
    catch (const cRuntimeError&) {
        return false;
    }
}

void Rx::attachInterframeResult(Packet *packet, bool accepted)
{
    discardExpiredReception();
    auto time = packet->findTag<SignalTimeInd>();
    if (!time || time->getStartTime() < SIMTIME_ZERO || time->getEndTime() <= time->getStartTime()) {
        invalidateFutureInterframeHistory();
        if (accepted)
            packet->addTagIfAbsent<Ieee80211InterframeInd>()->setRelated(false);
        return;
    }
    bool related = false;
    if (completedReception && completedReception->start == time->getStartTime() &&
            completedReception->end == time->getEndTime()) {
        auto evidence = std::move(*completedReception);
        completedReception.reset();
        if (accepted) {
            auto header = packet->peekAtFront<Ieee80211MacHeader>();
            auto twoAddress = dynamicPtrCast<const Ieee80211TwoAddressHeader>(header);
            auto indication = packet->findTag<Ieee80211ModeInd>();
            bool ht = indication && indication->getMode() &&
                indication->getMode()->getModulationClass() == IIeee80211Mode::ModulationClass::HT;
            // Model rule for coverage class zero, based on IEEE Std 802.11-2024, 10.3.2.3.3.
            if (evidence.preceding && evidence.sifs > SIMTIME_ZERO) {
                auto gap = evidence.start - evidence.preceding->end;
                related = gap >= evidence.sifs - evidence.tolerance && gap <= evidence.sifs + evidence.tolerance;
                auto& preceding = *evidence.preceding;
                // IEEE Std 802.11-2024, 10.3.2.3.2: RIFS requires HT, the same TA and RA, and no SIFS response.
                if (evidence.rifsPermitted && preceding.ht && ht && preceding.noSifsResponse &&
                        preceding.extension >= SIMTIME_ZERO && twoAddress && !preceding.transmitter.isUnspecified() &&
                        preceding.transmitter == twoAddress->getTransmitterAddress() &&
                        preceding.receiver == header->getReceiverAddress()) {
                    auto correctedGap = gap + preceding.extension;
                    related |= correctedGap >= evidence.rifs - evidence.rifs / 10 &&
                        correctedGap <= evidence.rifs + evidence.rifs / 10;
                }
            }
            if (evidence.mayBecomePredecessor) {
                PrecedingFrame preceding{evidence.end, false};
                preceding.ht = ht;
                preceding.transmitter = twoAddress ? twoAddress->getTransmitterAddress() : MacAddress();
                preceding.receiver = header->getReceiverAddress();
                preceding.extension = indication ? indication->getSignalExtensionDuration() : SimTime(-1);
                auto dataOrMgmt = dynamicPtrCast<const Ieee80211DataOrMgmtHeader>(header);
                preceding.noSifsResponse = recipientAckPolicy && (dataOrMgmt ? !recipientAckPolicy->isAckNeeded(dataOrMgmt) :
                    !dynamicPtrCast<const Ieee80211RtsFrame>(header) && !dynamicPtrCast<const Ieee80211BlockAckReq>(header));
                precedingFrame = preceding;
            }
        }
    }
    if (accepted)
        packet->addTagIfAbsent<Ieee80211InterframeInd>()->setRelated(related);
}

void Rx::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details)
{
    Enter_Method_Silent();
    discardExpiredReception();
    if (source->getParentModule() == networkInterface && dynamic_cast<IRadio *>(source)) {
        if (signalID == receptionStartedSignal)
            observeReceptionStart(check_and_cast<const IReception *>(obj));
        else if (signalID == receptionEndedSignal)
            observeReceptionEnd(check_and_cast<const IReception *>(obj));
        else if (signalID == transmissionStartedSignal)
            invalidateFutureInterframeHistory();
        else if (signalID == transmissionEndedSignal) {
            invalidateFutureInterframeHistory();
            if (isCompletedAck(dynamic_cast<const ITransmission *>(obj)))
                precedingFrame = PrecedingFrame{simTime(), true};
        }
    }
    if (signalID == modesetChangedSignal) {
        invalidateFutureInterframeHistory();
        ModeSetListener::receiveSignal(source, signalID, obj, details);
    }
}

void Rx::receiveSignal(cComponent *source, simsignal_t signalID, intval_t value, cObject *details)
{
    Enter_Method_Silent();
    discardExpiredReception();
    if (source->getParentModule() != networkInterface || !dynamic_cast<IRadio *>(source))
        return;
    if (signalID == FlatRadioBase::listeningChangedSignal)
        invalidateFutureInterframeHistory();
    else if (signalID == IRadio::radioModeChangedSignal) {
        if (value == IRadio::RADIO_MODE_OFF)
            invalidateInterframeHistory();
        else if (value == IRadio::RADIO_MODE_SLEEP || value == IRadio::RADIO_MODE_TRANSMITTER)
            invalidateFutureInterframeHistory();
    }
}

} // namespace ieee80211
} // namespace inet
