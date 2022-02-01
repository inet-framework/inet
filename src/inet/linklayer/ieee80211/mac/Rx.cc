//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/Rx.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/checksum/EthernetCRC.h"
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
    cancelAndDelete(endNavTimer);
}

void Rx::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        endNavTimer = new cMessage("NAV");
        WATCH(address);
        WATCH(receptionState);
        WATCH(transmissionState);
        WATCH(receivedPart);
        WATCH(mediumFree);
    }
    // TODO INITSTAGE
    else if (stage == INITSTAGE_NETWORK_INTERFACE_CONFIGURATION) {
        address = check_and_cast<Ieee80211Mac *>(getContainingNicModule(this)->getSubmodule("mac"))->getAddress();
        recomputeMediumFree();
    }
}

void Rx::handleMessage(cMessage *msg)
{
    if (msg == endNavTimer) {
        EV_INFO << "The radio channel has become free according to the NAV" << std::endl;
        emit(navChangedSignal, SimTime::ZERO);
        recomputeMediumFree();
    }
    else
        throw cRuntimeError("Unexpected self message");
}

bool Rx::lowerFrameReceived(Packet *packet)
{
    Enter_Method("lowerFrameReceived(\"%s\")", packet->getName());
    take(packet);

    bool isFrameOk = isFcsOk(packet);
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
                auto bufferLength = B(fcsBytes->getChunkLength()).get();
                auto buffer = new uint8_t[bufferLength];
                fcsBytes->copyToBuffer(buffer, bufferLength);
                auto computedFcs = ethernetCRC(buffer, bufferLength);
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
    bool oldMediumFree = mediumFree;
    // note: the duration of mode switching (rx-to-tx or tx-to-rx) should also count as busy
    mediumFree = receptionState == IRadio::RECEPTION_STATE_IDLE && transmissionState == IRadio::TRANSMISSION_STATE_UNDEFINED && !endNavTimer->isScheduled();
    if (mediumFree != oldMediumFree) {
        for (auto contention : contentions)
            contention->mediumStateChanged(mediumFree);
    }
}

void Rx::receptionStateChanged(IRadio::ReceptionState state)
{
    Enter_Method("receptionStateChanged");
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

void Rx::refreshDisplay() const
{
    if (mediumFree)
        getDisplayString().setTagArg("t", 0, "FREE");
    else {
        std::stringstream os;
        os << "BUSY (";
        bool addSpace = false;
        if (transmissionState != IRadio::TRANSMISSION_STATE_UNDEFINED) {
            switch (transmissionState) {
                case IRadio::IRadio::TRANSMISSION_STATE_UNDEFINED: break; // cannot happen
                case IRadio::IRadio::TRANSMISSION_STATE_IDLE: os << "Tx-Idle"; break;
                case IRadio::IRadio::TRANSMISSION_STATE_TRANSMITTING: os << "Tx"; break;
            }
            addSpace = true;
        }
        else {
            switch (receptionState) {
                case IRadio::RECEPTION_STATE_UNDEFINED: os << "Switching"; break;
                case IRadio::RECEPTION_STATE_IDLE: os << "Rx-Idle"; break; // cannot happen
                case IRadio::RECEPTION_STATE_BUSY: os << "Noise"; break;
                case IRadio::RECEPTION_STATE_RECEIVING: os << "Recv"; break;
            }
            addSpace = true;
        }
        if (endNavTimer->isScheduled()) {
            os << (addSpace ? " " : "") << "NAV";
        }
        os << ")";
        getDisplayString().setTagArg("t", 0, os.str().c_str());
    }
}

void Rx::registerContention(IContention *contention)
{
    contention->mediumStateChanged(mediumFree);
    contentions.push_back(contention);
}

} // namespace ieee80211
} // namespace inet

